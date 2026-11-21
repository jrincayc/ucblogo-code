# Validation Commands

Run from the repository root unless a verified isolated build supports otherwise. Check dependencies and active build state first; do not run every command on every task.

| Goal | Existing Command | Caveat |
| --- | --- | --- |
| Full web build / Node suite | `npm run build:web` | Requires Emscripten, Autotools, Node/npm dependencies; cleans/reconfigures root build. Incremental: `emmake make` then `make check`. |
| Browser relink | `emmake make ucblogo.html` | Uses configured `WEB_LDFLAGS`; restages data via `install-data`. |
| Node Logo suite, no rebuild | `npm run test:logo:node` | Runs `node tests/acceptance/run-logo-node.js` against the existing Node build; checks individual results, OOP skipped; suite ends with `bye`. |
| Source and runner checks, no rebuild | `npm run test:source` | Runs `node --test tests/acceptance/source.test.js tests/acceptance/logo-results.test.js tests/acceptance/runner.test.js`; no browser application rebuild. |
| Build plus source and all functional browser tests | `npm test` | Runs `npm run build:web` then `npm run test:built`; no Lighthouse/audits. |
| Source and all functional browser tests, no rebuild | `npm run test:built` | Source checks plus the shared TestCafe suite, including the Logo suite with its explicit OOP skip. Requires current application artifacts. |
| Build plus all functional browser tests | `npm run test:browser` | Runs `npm run build:web` then `npm run test:browser:built`; no audits. |
| All functional browser tests, no rebuild | `npm run test:browser:built` | Runs `node tests/acceptance/run-testcafe.js`; owns an available-port `emrun` server with HTTP readiness checks, or reuses `TESTCAFE_APP_URL`. Fixtures wait for runtime/input readiness. |
| Focused browser case | `node tests/acceptance/run-testcafe.js --test 'exact test name'` | Exact name, not a substring; use `--test-grep 'regex'` for a regex. Shared runner provides a portable Node whole-run watchdog; GNU `timeout` is not required. |
| Browser Logo language suite | `node tests/acceptance/run-testcafe.js --fixture 'Bundled Logo regression suite'` | Uses `tests/test.lg`; checks completion and individual failures/errors. OOP is explicitly skipped; skipped tests are not passing coverage. |
| Settings fixture | `npm run test:settings`, `npm run test:settings:built` | Build/built-only wrappers select the settings fixture through the shared runner; Chrome/Firefox wrappers use `TESTCAFE_BROWSER`. |
| Autocomplete (TestCafe) | `npm run test:autocomplete`, `npm run test:autocomplete:desktop`, `npm run test:autocomplete:phone` | Desktop/phone wrappers select the autocomplete fixture through the shared runner; use current application artifacts, not raw `--app`/fixed startup-delay commands. |
| Lighthouse | `npm run test:lighthouse` | Separate browser performance/audit checks, not language correctness coverage. |
| Remote iPad browser suite | `npm run test:ipad:remote` | Runs `node tests/acceptance/run-testcafe.js --browser remote`; requires LAN-reachable `TESTCAFE_HOSTNAME` and `TESTCAFE_APP_URL`, proxy ports `38079,38080`, and human device connection. Separate connection and whole-run watchdogs. |
| Serve built browser app | `emrun --no-browser --port PORT ucblogo.html` | Reuse suitable server/browser; use available alternative if emrun fails. Wait for input readiness. |

The shared runner uses `TESTCAFE_BROWSER` (default headless Chrome), `TESTCAFE_HEADLESS=0` for headed runs, and `TEST_DELAY_MS` for slow observation. `TESTCAFE_APP_URL` is preferred for reusing an existing server; legacy `UCBOLOGO_TEST_URL` warns, as do `BROWSERS` and `HEADLESS` (migrate to `TESTCAFE_BROWSER` and `TESTCAFE_HEADLESS`). `TESTCAFE_WATCHDOG_MS` bounds the whole run using portable Node supervision and cleanup of owned processes; `TESTCAFE_BROWSER_INIT_TIMEOUT` separately bounds remote-device connection waiting. Do not replace readiness checks with a fixed startup delay. Built-only commands do not rebuild: new browser fixtures alone need no application rebuild, but embedded Logo/application-source changes do. Keep Lighthouse separate and retain its existing audit options.

For a single Logo regression, read `tests/_how-to.txt`, `tests/UnitTests.lg`, `tests/test.lg`, and the nearest `UnitTests-*.lg` for setup/invocation. Start collector tests at `tests/UnitTests-MemMgr.lg`; `tests/runjs-multiline.lg` is a bridge regression example.

The programming skill records this browser full-suite entry point:

```logo
ignore runjs "|FS.chdir('/')| load "tests/test.lg
```

It changes the VFS working directory and ends with `bye`, terminating Logo. Prefer a dedicated test session, distinguish post-exit loader errors from test results, and inspect current failure aggregation/OOP exclusions before claiming success.
