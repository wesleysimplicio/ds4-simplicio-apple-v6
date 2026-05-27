"""US4 V6 Apple Edition - local OpenAI-compatible server.

Runs two MLX backends behind a single HTTP endpoint so any OpenAI client
(simplicio-cli, langchain, raw curl, etc.) can hit one base URL:

    /v1/chat/completions    -> Qwen2.5-Coder via mlx-lm (subprocess)
    /v1/completions         -> Qwen2.5-Coder via mlx-lm (subprocess, proxied)
    /v1/models              -> aggregated list (chat + embedding)
    /v1/embeddings          -> EmbeddingGemma via mlx-embeddings (in-process)
    /health                 -> liveness probe

Design intent: stdlib only (no FastAPI, no uvicorn). One file. The chat path
is delegated to the official `mlx_lm.server` shipped with mlx-lm >= 0.31.0,
which already speaks OpenAI shape; we boot it as a child process on
PORT + 1 and reverse-proxy. The embeddings path is implemented locally
because mlx-embeddings does not ship a server.

Environment knobs (all optional):

    US4_SERVE_HOST          default 127.0.0.1
    US4_SERVE_PORT          default 8080
    US4_SERVE_CHAT_MODEL    default mlx-community/Qwen2.5-Coder-7B-Instruct-4bit
    US4_SERVE_EMBED_MODEL   default mlx-community/embeddinggemma-300m-bf16
    US4_SERVE_DISABLE_CHAT  truthy -> skip mlx-lm subprocess
    US4_SERVE_DISABLE_EMBED truthy -> skip embeddings handler
    US4_SERVE_LOG_LEVEL     default INFO

Exit codes:

    0  graceful shutdown
    1  fatal startup error
    2  mlx-lm not installed and chat not disabled
    3  mlx-embeddings not installed and embeddings not disabled
"""

from __future__ import annotations

import atexit
import ipaddress
import json
import logging
import os
import signal
import socket
import subprocess
import sys
import threading
import time
import urllib.error
import urllib.request
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from typing import Any, Optional

LOG = logging.getLogger("us4.serve")

DEFAULT_CHAT_MODEL = "mlx-community/Qwen2.5-Coder-7B-Instruct-4bit"
DEFAULT_EMBED_MODEL = "mlx-community/embeddinggemma-300m-bf16"
MAX_BODY_BYTES = 8 * 1024 * 1024
UPSTREAM_TIMEOUT_S = 120


def _sanitize_path(path: str) -> str:
    return path.replace("\n", "\\n").replace("\r", "\\r")[:120]


def _truthy(value: Optional[str]) -> bool:
    if not value:
        return False
    return value.lower() not in {"0", "false", "no", "off", ""}


class Settings:
    host: str = os.environ.get("US4_SERVE_HOST", "127.0.0.1")
    port: int = int(os.environ.get("US4_SERVE_PORT", "8080"))
    chat_model: str = os.environ.get("US4_SERVE_CHAT_MODEL", DEFAULT_CHAT_MODEL)
    embed_model: str = os.environ.get("US4_SERVE_EMBED_MODEL", DEFAULT_EMBED_MODEL)
    disable_chat: bool = _truthy(os.environ.get("US4_SERVE_DISABLE_CHAT"))
    disable_embed: bool = _truthy(os.environ.get("US4_SERVE_DISABLE_EMBED"))
    log_level: str = os.environ.get("US4_SERVE_LOG_LEVEL", "INFO").upper()

    @property
    def upstream_port(self) -> int:
        return self.port + 1

    @property
    def upstream_url(self) -> str:
        return f"http://127.0.0.1:{self.upstream_port}"


SETTINGS = Settings()


class EmbeddingsBackend:
    """In-process EmbeddingGemma via mlx-embeddings. Lazy-loaded."""

    def __init__(self, model_id: str) -> None:
        self._model_id = model_id
        self._lock = threading.Lock()
        self._model = None
        self._tokenizer = None

    def ensure_loaded(self) -> None:
        # No fast path outside the lock: with ThreadingHTTPServer two requests
        # can race past the None check and trigger duplicate model loads
        # (catastrophic on unified memory). The lock cost is negligible after
        # the model is loaded once.
        with self._lock:
            if self._model is not None:
                return
            try:
                from mlx_embeddings import load as _load
            except ImportError as exc:
                raise RuntimeError(
                    "mlx-embeddings is not installed. "
                    "Run: pip install -r scripts/requirements-serve.txt"
                ) from exc
            LOG.info("loading embedding model: %s", self._model_id)
            t0 = time.monotonic()
            self._model, self._tokenizer = _load(self._model_id)
            LOG.info("embedding model loaded in %.2fs", time.monotonic() - t0)

    def embed(self, texts: list[str]) -> list[list[float]]:
        self.ensure_loaded()
        # mlx-embeddings 0.1.0 ships a `generate()` helper that calls
        # `model(**inputs)` with keys `input_ids` / `attention_mask`. Some model
        # classes (gemma3_text, others) expect `inputs=` instead, raising
        # TypeError. Tokenize here and dispatch with the signature the loaded
        # model accepts so the backend stays model-agnostic.
        batch = self._tokenizer(
            texts,
            return_tensors="mlx",
            padding=True,
            truncation=True,
            max_length=512,
        )
        ids = batch["input_ids"]
        attention_mask = batch.get("attention_mask")
        try:
            output = self._model(inputs=ids, attention_mask=attention_mask)
        except TypeError:
            output = self._model(input_ids=ids, attention_mask=attention_mask)
        return self._extract_vectors(output)

    @staticmethod
    def _extract_vectors(output: Any) -> list[list[float]]:
        candidate = output
        for attr in ("text_embeds", "sentence_embeddings", "embeddings", "pooler_output"):
            if hasattr(candidate, attr):
                candidate = getattr(candidate, attr)
                break
        if hasattr(candidate, "tolist"):
            data = candidate.tolist()
        else:
            data = list(candidate)
        if data and not isinstance(data[0], list):
            return [list(map(float, data))]
        return [[float(x) for x in row] for row in data]


EMBED_BACKEND: Optional[EmbeddingsBackend] = None


def _send_json(handler: BaseHTTPRequestHandler, status: int, payload: dict) -> None:
    body = json.dumps(payload).encode("utf-8")
    handler.send_response(status)
    handler.send_header("Content-Type", "application/json")
    handler.send_header("Content-Length", str(len(body)))
    handler.end_headers()
    handler.wfile.write(body)


def _send_error(handler: BaseHTTPRequestHandler, status: int, message: str, code: str = "invalid_request_error") -> None:
    _send_json(handler, status, {"error": {"message": message, "type": code}})


def _proxy_upstream(handler: BaseHTTPRequestHandler, path: str, raw_body: bytes) -> None:
    if SETTINGS.disable_chat:
        _send_error(handler, 503, "chat backend disabled (US4_SERVE_DISABLE_CHAT)", "service_unavailable")
        return
    url = f"{SETTINGS.upstream_url}{path}"
    method = handler.command
    headers = {"Content-Type": handler.headers.get("Content-Type", "application/json")}
    accept = handler.headers.get("Accept")
    if accept:
        headers["Accept"] = accept
    req = urllib.request.Request(url, data=raw_body if raw_body else None, method=method, headers=headers)
    try:
        with urllib.request.urlopen(req, timeout=UPSTREAM_TIMEOUT_S) as resp:
            handler.send_response(resp.status)
            content_type = resp.headers.get("Content-Type", "application/json")
            handler.send_header("Content-Type", content_type)
            transfer = resp.headers.get("Transfer-Encoding")
            content_length = resp.headers.get("Content-Length")
            if transfer:
                handler.send_header("Transfer-Encoding", transfer)
            elif content_length:
                handler.send_header("Content-Length", content_length)
            handler.end_headers()
            try:
                while True:
                    chunk = resp.read(4096)
                    if not chunk:
                        break
                    handler.wfile.write(chunk)
                    handler.wfile.flush()
            except (BrokenPipeError, ConnectionResetError):
                LOG.info("client disconnected mid-stream")
                return
    except urllib.error.HTTPError as exc:
        body = exc.read()
        handler.send_response(exc.code)
        handler.send_header("Content-Type", exc.headers.get("Content-Type", "application/json"))
        handler.send_header("Content-Length", str(len(body)))
        handler.end_headers()
        try:
            handler.wfile.write(body)
        except (BrokenPipeError, ConnectionResetError):
            return
    except urllib.error.URLError as exc:
        _send_error(handler, 502, f"chat upstream unreachable: {exc.reason}", "bad_gateway")


def _build_models_payload() -> dict:
    now = int(time.time())
    data: list[dict] = []
    if not SETTINGS.disable_chat:
        data.append({"id": SETTINGS.chat_model, "object": "model", "created": now, "owned_by": "us4-local"})
    if not SETTINGS.disable_embed:
        data.append({"id": SETTINGS.embed_model, "object": "model", "created": now, "owned_by": "us4-local"})
    return {"object": "list", "data": data}


def _handle_embeddings(handler: BaseHTTPRequestHandler, body: Optional[dict]) -> None:
    if SETTINGS.disable_embed:
        _send_error(handler, 503, "embedding backend disabled (US4_SERVE_DISABLE_EMBED)", "service_unavailable")
        return
    if not body:
        _send_error(handler, 400, "request body must be JSON with 'input' field")
        return
    raw_input = body.get("input")
    if raw_input is None:
        _send_error(handler, 400, "missing 'input' field")
        return
    if isinstance(raw_input, str):
        texts = [raw_input]
    elif isinstance(raw_input, list) and all(isinstance(item, str) for item in raw_input):
        texts = list(raw_input)
    else:
        _send_error(handler, 400, "'input' must be string or list of strings")
        return
    requested_model = body.get("model") or SETTINGS.embed_model
    if EMBED_BACKEND is None:
        _send_error(handler, 503, "embedding backend not initialized", "service_unavailable")
        return
    try:
        vectors = EMBED_BACKEND.embed(texts)
    except Exception:
        LOG.exception("embedding failure")
        _send_error(handler, 500, "embedding failed", "internal_error")
        return
    data = [
        {"object": "embedding", "index": idx, "embedding": vec}
        for idx, vec in enumerate(vectors)
    ]
    total_tokens = sum(len(t.split()) for t in texts)
    _send_json(
        handler,
        200,
        {
            "object": "list",
            "data": data,
            "model": requested_model,
            "usage": {"prompt_tokens": total_tokens, "total_tokens": total_tokens},
        },
    )


class Us4Handler(BaseHTTPRequestHandler):
    server_version = "us4-serve/0.1"

    def log_message(self, fmt: str, *args: Any) -> None:
        LOG.info("%s %s", self.address_string(), fmt % args)

    def do_GET(self) -> None:
        path = self.path.split("?", 1)[0]
        if path in ("/health", "/v1/health"):
            _send_json(self, 200, {"status": "ok", "chat": not SETTINGS.disable_chat, "embed": not SETTINGS.disable_embed})
            return
        if path in ("/v1/models", "/models"):
            _send_json(self, 200, _build_models_payload())
            return
        _send_error(self, 404, f"not found: {_sanitize_path(path)}", "not_found")

    def do_POST(self) -> None:
        path = self.path.split("?", 1)[0]
        length_header = self.headers.get("Content-Length")
        try:
            length = int(length_header) if length_header else 0
        except ValueError:
            length = 0
        if length < 0:
            _send_error(self, 400, "invalid Content-Length")
            return
        if length > MAX_BODY_BYTES:
            _send_error(self, 413, "request body too large", "payload_too_large")
            return
        raw_body = self.rfile.read(length) if length > 0 else b""
        body: Optional[dict] = None
        if raw_body:
            try:
                body = json.loads(raw_body.decode("utf-8"))
            except (json.JSONDecodeError, UnicodeDecodeError):
                body = None
        if path in ("/v1/embeddings", "/embeddings"):
            _handle_embeddings(self, body)
            return
        if path in ("/v1/chat/completions", "/chat/completions", "/v1/completions", "/completions"):
            _proxy_upstream(self, path, raw_body)
            return
        _send_error(self, 404, f"not found: {_sanitize_path(path)}", "not_found")


def _spawn_upstream() -> Optional[subprocess.Popen]:
    if SETTINGS.disable_chat:
        LOG.info("chat backend disabled; skipping mlx-lm subprocess")
        return None
    try:
        import mlx_lm  # noqa: F401
    except ImportError:
        LOG.error("mlx-lm is not installed. Run: pip install -r scripts/requirements-serve.txt")
        sys.exit(2)
    cmd = [
        sys.executable,
        "-m",
        "mlx_lm",
        "server",
        "--model",
        SETTINGS.chat_model,
        "--host",
        "127.0.0.1",
        "--port",
        str(SETTINGS.upstream_port),
        "--log-level",
        SETTINGS.log_level,
    ]
    LOG.info("spawning mlx-lm chat backend on 127.0.0.1:%d", SETTINGS.upstream_port)
    proc = subprocess.Popen(cmd, stdout=sys.stdout, stderr=sys.stderr, close_fds=True)
    atexit.register(_terminate_proc, proc)
    return proc


def _terminate_proc(proc: Optional[subprocess.Popen]) -> None:
    if not proc or proc.poll() is not None:
        return
    LOG.info("terminating mlx-lm subprocess (pid=%s)", proc.pid)
    proc.terminate()
    try:
        proc.wait(timeout=10)
    except subprocess.TimeoutExpired:
        proc.kill()


def _wait_upstream_ready(timeout_s: float = 120.0) -> bool:
    if SETTINGS.disable_chat:
        return True
    deadline = time.monotonic() + timeout_s
    health_url = f"{SETTINGS.upstream_url}/health"
    while time.monotonic() < deadline:
        try:
            with urllib.request.urlopen(health_url, timeout=2) as resp:
                if resp.status == 200:
                    LOG.info("mlx-lm backend ready")
                    return True
        except (urllib.error.URLError, ConnectionError, socket.timeout, TimeoutError):
            time.sleep(1.0)
    LOG.warning("mlx-lm backend did not become ready in %.0fs", timeout_s)
    return False


def _init_embeddings() -> None:
    global EMBED_BACKEND
    if SETTINGS.disable_embed:
        LOG.info("embeddings backend disabled; skipping mlx-embeddings init")
        return
    try:
        import mlx_embeddings  # noqa: F401
    except ImportError:
        LOG.error("mlx-embeddings is not installed. Run: pip install -r scripts/requirements-serve.txt")
        sys.exit(3)
    EMBED_BACKEND = EmbeddingsBackend(SETTINGS.embed_model)


def _install_signal_handlers(server: ThreadingHTTPServer, upstream: Optional[subprocess.Popen]) -> None:
    def _shutdown(_signum: int, _frame: Any) -> None:
        LOG.info("signal received; shutting down")
        threading.Thread(target=server.shutdown, daemon=True).start()
        _terminate_proc(upstream)

    signal.signal(signal.SIGINT, _shutdown)
    signal.signal(signal.SIGTERM, _shutdown)


def _warn_if_non_loopback(host: str) -> None:
    try:
        ip = ipaddress.ip_address(host)
    except ValueError:
        if host not in ("localhost", ""):
            LOG.warning("binding to non-numeric host %s; ensure access is restricted", host)
        return
    if not ip.is_loopback:
        LOG.warning(
            "binding to non-loopback address %s: this exposes the API to the network "
            "without auth. Use only on trusted networks.",
            host,
        )


def main() -> int:
    logging.basicConfig(level=SETTINGS.log_level, format="%(asctime)s %(name)s %(levelname)s %(message)s")
    LOG.info(
        "us4 serve starting (host=%s port=%d chat=%s embed=%s)",
        SETTINGS.host,
        SETTINGS.port,
        "off" if SETTINGS.disable_chat else SETTINGS.chat_model,
        "off" if SETTINGS.disable_embed else SETTINGS.embed_model,
    )
    _warn_if_non_loopback(SETTINGS.host)
    _init_embeddings()
    upstream = _spawn_upstream()
    _wait_upstream_ready()
    server = ThreadingHTTPServer((SETTINGS.host, SETTINGS.port), Us4Handler)
    _install_signal_handlers(server, upstream)
    LOG.info("listening on http://%s:%d", SETTINGS.host, SETTINGS.port)
    try:
        server.serve_forever()
    finally:
        _terminate_proc(upstream)
    return 0


if __name__ == "__main__":
    sys.exit(main())
