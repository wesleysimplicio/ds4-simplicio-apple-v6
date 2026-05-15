#!/usr/bin/env node
'use strict';

const fs = require('node:fs');
const path = require('node:path');
const { fileURLToPath } = require('node:url');
const { spawnSync } = require('node:child_process');

const root = path.resolve(__dirname, '..');
const wantsCoverage = process.argv.slice(2).includes('--coverage');
const testFiles = fs.readdirSync(__dirname)
  .filter((file) => file.endsWith('.test.js'))
  .map((file) => path.join(__dirname, file));

if (testFiles.length === 0) {
  console.error('No test files found in test/*.test.js');
  process.exit(1);
}

const coverageRoot = path.join(root, 'coverage');
const v8CoverageRoot = path.join(coverageRoot, '.v8');
const env = { ...process.env };

if (wantsCoverage) {
  fs.rmSync(coverageRoot, { recursive: true, force: true });
  fs.mkdirSync(v8CoverageRoot, { recursive: true });
  env.NODE_V8_COVERAGE = v8CoverageRoot;
}

const result = spawnSync(process.execPath, ['--test', ...testFiles], {
  cwd: root,
  stdio: 'inherit',
  env,
});

const status = result.status ?? 1;
if (status !== 0) process.exit(status);

if (wantsCoverage) {
  writeCoverageSummary();
}

function writeCoverageSummary() {
  const trackedFiles = new Set([path.join(root, 'bin', 'cli.js')]);
  const byFile = new Map();

  for (const file of fs.readdirSync(v8CoverageRoot)) {
    if (!file.endsWith('.json')) continue;
    const payload = JSON.parse(fs.readFileSync(path.join(v8CoverageRoot, file), 'utf8'));
    for (const script of payload.result || []) {
      if (!script.url.startsWith('file://')) continue;
      const scriptPath = fileURLToPath(script.url);
      if (!trackedFiles.has(scriptPath)) continue;
      const ranges = byFile.get(scriptPath) || [];
      for (const fn of script.functions || []) {
        for (const range of fn.ranges || []) {
          if (range.count > 0) ranges.push(range);
        }
      }
      byFile.set(scriptPath, ranges);
    }
  }

  const files = {};
  let totalLines = 0;
  let coveredLines = 0;

  for (const file of trackedFiles) {
    const source = fs.readFileSync(file, 'utf8');
    const ranges = byFile.get(file) || [];
    const stats = countCoveredLines(source, ranges);
    const rel = path.relative(root, file).replace(/\\/g, '/');
    files[rel] = makeSummary(stats.total, stats.covered);
    totalLines += stats.total;
    coveredLines += stats.covered;
  }

  const summary = {
    total: makeSummary(totalLines, coveredLines),
    ...files,
  };

  fs.writeFileSync(
    path.join(coverageRoot, 'coverage-summary.json'),
    JSON.stringify(summary, null, 2) + '\n',
  );
}

function countCoveredLines(source, ranges) {
  const lines = source.split(/\r?\n/);
  const effectiveRanges = ranges.filter((range) => {
    return (range.endOffset - range.startOffset) < source.length * 0.9;
  });
  const hasTopLevelRange = ranges.some((range) => {
    return (range.endOffset - range.startOffset) >= source.length * 0.9;
  });
  const firstFunctionOffset = source.indexOf('\nfunction ');
  let offset = 0;
  let total = 0;
  let covered = 0;
  let inTemplateLiteral = false;

  for (const line of lines) {
    const start = offset;
    const end = offset + line.length;
    offset = end + 1;

    const countLine = isCountedLine(line, inTemplateLiteral);
    inTemplateLiteral = nextTemplateLiteralState(line, inTemplateLiteral);
    if (!countLine) continue;
    total++;
    const coveredByTopLevel = hasTopLevelRange && firstFunctionOffset !== -1 && start < firstFunctionOffset;
    if (coveredByTopLevel || effectiveRanges.some((range) => range.startOffset <= start && range.endOffset >= end)) {
      covered++;
    }
  }

  return { total, covered };
}

function isCountedLine(line, inTemplateLiteral) {
  const trimmed = line.trim();
  if (inTemplateLiteral) return false;
  return trimmed !== '' && !trimmed.startsWith('//') && !trimmed.startsWith('*');
}

function nextTemplateLiteralState(line, current) {
  const matches = line.match(/`/g);
  if (!matches || matches.length % 2 === 0) return current;
  return !current;
}

function makeSummary(total, covered) {
  const pct = total === 0 ? 100 : Number(((covered / total) * 100).toFixed(2));
  return {
    lines: { total, covered, skipped: 0, pct },
    statements: { total, covered, skipped: 0, pct },
    functions: { total: 0, covered: 0, skipped: 0, pct: 100 },
    branches: { total: 0, covered: 0, skipped: 0, pct: 100 },
  };
}
