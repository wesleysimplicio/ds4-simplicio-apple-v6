import { test, expect, type TestInfo } from '@playwright/test';
import { execFile } from 'node:child_process';
import path from 'node:path';
import { promisify } from 'node:util';

const execFileAsync = promisify(execFile);
const repoRoot = path.resolve(__dirname, '..', '..');
const cliPath = path.join(repoRoot, 'bin', 'cli.js');
const packageJson = require(path.join(repoRoot, 'package.json')) as { version: string };

type CliRun = {
  stdout: string;
  stderr: string;
};

async function runCli(args: string[]): Promise<CliRun> {
  const { stdout, stderr } = await execFileAsync(process.execPath, [cliPath, ...args], {
    cwd: repoRoot,
    env: {
      ...process.env,
      NO_COLOR: '1',
    },
  });

  return {
    stdout: stdout.trim(),
    stderr: stderr.trim(),
  };
}

async function attachCommand(testInfo: TestInfo, label: string, result: CliRun): Promise<void> {
  await testInfo.attach(`stdout-${label}`, {
    body: result.stdout || '(empty)',
    contentType: 'text/plain',
  });
  await testInfo.attach(`stderr-${label}`, {
    body: result.stderr || '(empty)',
    contentType: 'text/plain',
  });
}

test.describe('Starter CLI smoke', () => {
  test('exposes version contract in text and JSON', async ({}, testInfo) => {
    const textResult = await runCli(['--version']);
    await attachCommand(testInfo, 'version-text', textResult);

    expect(textResult.stderr).toBe('');
    expect(textResult.stdout).toBe(packageJson.version);

    const jsonResult = await runCli(['--version', '--json']);
    await attachCommand(testInfo, 'version-json', jsonResult);

    expect(jsonResult.stderr).toBe('');
    expect(JSON.parse(jsonResult.stdout)).toMatchObject({
      cli: 'us4-cli',
      version: packageJson.version,
    });
  });

  test('exposes probe contract in text and JSON', async ({}, testInfo) => {
    const textResult = await runCli(['--probe']);
    await attachCommand(testInfo, 'probe-text', textResult);

    expect(textResult.stderr).toBe('');
    expect(textResult.stdout).toContain(`us4-cli ${packageJson.version}`);
    expect(textResult.stdout).toContain('mode: ');
    expect(textResult.stdout).toContain('platform: ');
    expect(textResult.stdout).toContain('memory: ');

    const jsonResult = await runCli(['--probe', '--json']);
    await attachCommand(testInfo, 'probe-json', jsonResult);

    expect(jsonResult.stderr).toBe('');
    expect(JSON.parse(jsonResult.stdout)).toMatchObject({
      cli: 'us4-cli',
      version: packageJson.version,
      probe: {
        platform: expect.any(String),
        arch: expect.any(String),
        cpuModel: expect.any(String),
        logicalCores: expect.any(Number),
        totalMemoryBytes: expect.any(Number),
        totalMemoryGiB: expect.any(Number),
        appleSilicon: expect.any(Boolean),
        aneEligible: expect.any(Boolean),
      },
      mode: {
        requested: 'auto',
        selected: expect.any(String),
        taxonomy: expect.arrayContaining(['FULL', 'MICRO_PLUS', 'NANO']),
        source: 'memory-tier',
      },
    });
  });

  test('keeps mode auto JSON aligned with the probe shape', async ({}, testInfo) => {
    const modeResult = await runCli(['--mode', 'auto', '--json']);
    await attachCommand(testInfo, 'mode-auto-json', modeResult);

    expect(modeResult.stderr).toBe('');
    expect(JSON.parse(modeResult.stdout)).toMatchObject({
      cli: 'us4-cli',
      version: packageJson.version,
      probe: {
        platform: expect.any(String),
        arch: expect.any(String),
      },
      mode: {
        requested: 'auto',
        selected: expect.any(String),
        taxonomy: [
          'FULL',
          'BALANCED_PLUS',
          'DEGRADED',
          'ULTRA_LOW',
          'MICRO',
          'MICRO_PLUS',
          'NANO',
        ],
        source: 'memory-tier',
      },
    });
  });
});
