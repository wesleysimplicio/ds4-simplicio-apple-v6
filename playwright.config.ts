import { defineConfig } from '@playwright/test';

/**
 * Playwright config for CLI smoke coverage.
 *
 * The repository still uses the Node starter CLI as the runnable surface.
 * Playwright is the orchestrator for smoke checks and evidence capture until
 * a native `us4-cli` binary becomes the primary executable under test.
 */
export default defineConfig({
  testDir: './tests/e2e',
  timeout: 30_000,
  expect: {
    timeout: 5_000,
  },
  forbidOnly: !!process.env.CI,
  retries: process.env.CI ? 2 : 0,
  workers: 1,
  outputDir: 'test-results/',
  reporter: [
    ['html', { outputFolder: 'playwright-report', open: 'never' }],
    ['json', { outputFile: 'test-results/results.json' }],
    ['junit', { outputFile: 'test-results/results.xml' }],
    ['list'],
  ],
  use: {
    trace: 'on',
    screenshot: 'off',
    video: 'off',
  },
  projects: [
    {
      name: 'cli-smoke',
    },
  ],
});
