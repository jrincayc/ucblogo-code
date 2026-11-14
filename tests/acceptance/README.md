# Portable Acceptance Tests

Functional browser coverage is consolidated in TestCafe. The shared runner discovers all `*.testcafe.js` fixtures plus [ipad-keyboard.test.js](ipad-keyboard.test.js); there is no second Selenium assertion pass. Source checks and runner/grader unit tests use Node's test runner. Lighthouse remains a separate Mocha audit, not part of functional acceptance.

The implemented inventory is **57 desktop functional cases**, or **49 mobile/remote cases** after capability filtering excludes eight desktop resize tests. These are case counts, not a claim that every supported device has passed. Real Safari on the maintainer's iPad is **pending human signoff**; desktop Safari is **not tested locally**. See [COVERAGE.md](COVERAGE.md) for exact destinations, baseline history, and exclusions.

## Prerequisites And Freshness

Run commands from the repository root. Development tooling requires **Node >=20**, npm dependencies (`npm install`), and the chosen installed browser. The dependency/toolchain release may impose a newer minimum. This development-tool requirement is separate from the compiled Logo application's Node runtime support; see [webedition.md](../../webedition.md). A fresh build also needs the existing Emscripten/Autotools environment for [emscript.sh](../../emscript.sh). Automatic local serving needs `emrun` on `PATH` (or `EMRUN` pointing to it).

Build commands below invoke `./emscript.sh` once and run tests only after success. Built-artifact commands never rebuild. Rebuild after changing embedded sources, including the shell, runtime JS, C, Logo startup files, or packaged Logo tests. Verify the served HTML, loader, Wasm, and packaged files belong to the same successful build and intended configuration. Rebuilding disk artifacts does not replace an already-running runtime.

Use a dedicated test runtime, never a page containing unsaved Logo work. Fixtures navigate, replace output, reload, run terminating Logo tests, and temporarily change settings or mock browser APIs. Preserve unsaved definitions, editor contents, and files from any current page before starting, and leave that page out of automation. Helpers restore captured `ucblogo.settings` and `ucblogo.fontSize` strings exactly, including after reload normalization. This is not a general backup of in-memory work or browser storage. Never clear all site data to make a run pass.

## Command Contract

These are the consolidated package-script contracts. All paths below are relative to the repository root; the shared runner is [run-testcafe.js](run-testcafe.js).

| Command | Contract |
| --- | --- |
| `npm test` | `./emscript.sh && npm run test:built`: build once, then source/unit and functional browser checks. No Lighthouse. |
| `npm run test:built` | `npm run test:source && npm run test:browser:built`: existing artifacts only. |
| `npm run test:browser` | `./emscript.sh && npm run test:browser:built`: build once, then functional browser checks only. |
| `npm run test:browser:built` | `node tests/acceptance/run-testcafe.js`: all discovered functional fixtures. |
| `npm run test:source` | `node --test tests/acceptance/source.test.js tests/acceptance/logo-results.test.js tests/acceptance/runner.test.js`: five source cases, 15 grader cases, 27 runner cases (four real-Chrome cases opt-in). No Wasm build. |
| `npm run test:runner:browser` | Portable Node wrapper enables `RUNNER_BROWSER_TESTS=1` and runs all 27 runner self-tests, including real-Chrome failure, hang, capability, and reporter-flush checks. |
| `npm run test:logo:node` | `node tests/acceptance/run-logo-node.js`: grade the existing Node/Wasm Logo build. |
| `npm run test:settings` | `./emscript.sh && npm run test:settings:built`: build once, then the settings fixture. |
| `npm run test:settings:built` | `node tests/acceptance/run-testcafe.js --fixture 'Settings persistence'`: settings and UI against existing artifacts. |
| `npm run test:autocomplete` | Shared autocomplete fixture in desktop Chrome, then phone emulation; existing artifacts only. |
| `npm run test:autocomplete:desktop` | Desktop alias of the shared autocomplete fixture. |
| `npm run test:autocomplete:phone` | Shared autocomplete fixture with `--browser 'chrome:headless:emulation:width=390;height=844;mobile=true;touch=true'`. Emulation, not an actual phone. |
| `npm run test:browser:smoke` | Shared runner with `--test 'Logo starts and displays its welcome message'`, existing artifacts only. |
| `npm run test:ipad:remote` | Shared runner with `--browser remote`, all capability-eligible fixtures, not just the keyboard simulation. Requires LAN hostname and app URL. |
| `npm run test:lighthouse` | Unchanged separate mobile/desktop Lighthouse audit against existing artifacts. |

`test:browser:build` remains an alias of `test:browser`. The temporary `test:selenium:legacy` command, Selenium test files, and `selenium-webdriver` dependency are retired. Mocha/Chai remain for Lighthouse.

Direct commands are useful during focused iteration after a successful build:

```sh
node tests/acceptance/run-testcafe.js --test 'Logo starts and displays its welcome message'
node tests/acceptance/run-testcafe.js --fixture 'Logo word autocompletion'
node tests/acceptance/run-testcafe.js --test-grep 'editor:|Ctrl\+\['
TESTCAFE_BROWSER=firefox npm run test:browser:built
TESTCAFE_HEADLESS=0 TESTCAFE_SPEED=0.5 TEST_DELAY_MS=1000 npm run test:autocomplete
node tests/acceptance/run-testcafe.js --browser 'chrome:headless:emulation:width=390;height=844;mobile=true;touch=true'
node tests/acceptance/run-testcafe.js --screenshots path=reports/testcafe/screenshots
```

`--fixture` and `--test` select **exact names**, not substrings; `--fixture-grep` and `--test-grep` accept regular expressions. Filters compose with capability requirements, so a filter cannot enable an unavailable resize case. Additional TestCafe CLI flags are forwarded (an optional `--` separator is accepted). For example, pass `--no-sandbox --disable-dev-shm-usage` where the local Chrome environment needs them.

## Runner Configuration

Configuration lives in [run-testcafe.js](run-testcafe.js) and [application-server.js](application-server.js). Proxy automation (`--disable-native-automation`) is the portable default across Chrome, Firefox, and remote Safari. Desktop-only native automation is an explicit opt-in via `TESTCAFE_NATIVE_AUTOMATION=1` or `--native-automation`, and is rejected with mobile/remote aliases.

| Setting | Meaning And Default |
| --- | --- |
| `TESTCAFE_BROWSER` / `--browser` | Browser alias or comma-separated aliases; default `chrome`. The environment variable wins over `--browser`. |
| `TESTCAFE_HEADLESS` | `1` by default; appends `:headless` to bare Chrome/Chromium/Firefox/Edge aliases. `0` leaves aliases unchanged; it does not strip an explicit `:headless`. |
| `TESTCAFE_APP_URL` | Reuse an HTTP(S) app URL and its existing server. Otherwise start owned local `emrun` on an available port. |
| `TESTCAFE_CAPABILITIES` | Comma-separated capability allowlist. Desktop default `resize,screenshots`; mobile/emulation/remote default empty. An explicit empty string removes both capabilities. |
| `TESTCAFE_TEST` | Exact test-name filter, unless overridden by CLI `--test`. |
| `TESTCAFE_HOSTNAME` | Host address advertised by TestCafe; required for remote and must resolve to a LAN-reachable address, not loopback, unspecified, or link-local. |
| `TESTCAFE_PORTS` | Two distinct proxy ports, default `38079,38080` for remote. |
| `TESTCAFE_BROWSER_INIT_TIMEOUT` | Connection/startup timeout in milliseconds: local `30000`, remote `300000`. Also bounds the remote connection wizard. |
| `TESTCAFE_WATCHDOG_MS` | Whole-run timeout, including server readiness, device connection, and tests: local `300000`, remote `900000`. |
| `TESTCAFE_ASSERTION_TIMEOUT` / `TESTCAFE_SELECTOR_TIMEOUT` | Default `10000` ms each; individual readiness and long Logo-suite assertions can specify longer waits. |
| `TESTCAFE_SPEED` | Optional TestCafe action speed, e.g. `0.5`. |
| `TEST_DELAY_MS` | Optional viewing delay after Logo commands; also slows autocomplete typing. Not a startup-readiness mechanism. |
| `TEST_INTERACTION_DELAY_MS` | Autocomplete interaction pause, default `100` ms. |
| `POPUP_LATENCY_BUDGET_MS` | Optional autocomplete latency threshold. Default **0 disables the threshold assertion**; the case still measures and logs samples. Use a positive measured budget for gating. |

`BROWSERS`, `HEADLESS`, and `UCBOLOGO_TEST_URL` are deprecated compatibility inputs and emit warnings; migrate to `TESTCAFE_BROWSER`, `TESTCAFE_HEADLESS`, and `TESTCAFE_APP_URL`. Conflicting old/new app URLs are rejected. Do not mix obsolete Selenium environment examples into new runs.

There is **no fixed startup delay**: the server helper waits for a successful HTTP response, then fixtures wait for welcome output and initialized, idle Logo input. [testcafe-helpers.js](testcafe-helpers.js) also checks Asyncify, the evaluator lock, and continuation-input state before submitting Logo commands. This is readiness checking, not permission to reenter an active interpreter.

The default isolated local origin uses an available loopback port, where the application skips service-worker registration. For URL reuse, verify freshness yourself: bypass/disable an existing service worker, disable HTTP caching for diagnosis, and reload the dedicated test runtime. A LAN origin may have a worker; an already-loaded page is not refreshed merely by rebuilding. Preserve user storage and never restart a shared browser without permission.

The runner propagates TestCafe failures and stops only its owned server/browser/process trees, including after failure, connection timeout, SIGINT, SIGTERM, or the whole-run watchdog. A reused server is not stopped. The watchdog exits `124`; SIGINT/SIGTERM use `130`/`143`. Startup/assertion/connection failures return nonzero. Browser-init timeout and whole-run watchdog are separate limits; increase the latter deliberately for a long human-coordinated session rather than disabling bounds.

The default reporters are terminal `spec` and JSON at `reports/testcafe.json` (overwritten by later runs). Preserve or rename reports for each platform. Screenshots require an explicit path such as `--screenshots path=reports/testcafe/screenshots`; autocomplete checkpoints additionally require the `screenshots` capability. Remote defaults do not assume screenshot support. Record unavailable captures rather than enabling unsupported capabilities.

## Human iPad Session

**Pending:** no real-device pass is claimed by this consolidation. Use the maintainer's iPad on the same LAN; no paid hosted service is required. Complete desktop Chrome, mobile Chrome/emulation, and Firefox automated checks before this broader human pass, unless diagnosing an iPad-specific issue.

1. Preserve all unsaved work on the iPad's current Logo page. Keep it separate and open a fresh test tab/session for the connection URL. Do not point automation at an unsaved runtime page. Tests intentionally reload and terminate runtimes, including accepting test-page unload prompts.
2. Reuse a known-fresh LAN app server or start a separate server for this session. For example, `emrun --no-browser --hostname 0.0.0.0 --port 8085 ucblogo.html` from the repository root, provided that port is free. The human starting this server owns its cleanup. Verify its served build identity.
3. Use the host's actual LAN IP in both settings below. Allow inbound TCP **38079 and 38080** through the host firewall; allow the app port (here **8085**) where direct LAN app access/manual checks need it. Replace ports consistently if occupied, and ensure the Wi-Fi network does not isolate clients.
4. Start the remote suite, then open the **connection URL printed by TestCafe** in the fresh Safari test tab. Do not substitute the app URL for that connection URL.

```sh
TESTCAFE_HOSTNAME=192.168.1.20 \
TESTCAFE_APP_URL=http://192.168.1.20:8085/ucblogo.html \
TESTCAFE_PORTS=38079,38080 \
TESTCAFE_BROWSER_INIT_TIMEOUT=300000 \
TESTCAFE_WATCHDOG_MS=900000 \
npm run test:ipad:remote
```

TestCafe's proxy fetches the app URL **from the host** and serves it to the device through the two proxy ports. The examples intentionally use a LAN app URL so direct manual access and proxy access refer to the same build. The device must reach the proxy hostname and both ports. Default remote capabilities exclude only the eight resize-dependent cases: **49 functional cases remain**, including the Logo suite, settings, bridge, autocomplete, errors, startup, simulated viewport checks, and screen-mode checks at the device's actual viewport. This is the complete eligible suite, not a two-test iPad proof of concept.

Automated key simulation and in-page suggestion `.click()` calls are not native iOS keyboard or touch evidence. Synthetic Visual Viewport property changes and dispatched orientation events are not physical rotation. After automation, use another dedicated, fresh app session for these **human checkpoints**, in Safari and (where installed) the standalone PWA:

- Show and dismiss the native on-screen keyboard in command input and editor. Check caret/selection, punctuation keys, autocomplete, focus return, and modal dismissal; use a hardware keyboard for Tab/arrows/Escape/Ctrl+[ where available, recording unavailable hardware.
- Rotate portrait -> landscape -> portrait with keyboard shown and hidden. Check input, popup, and dialogs remain visible; no document jump, unwanted horizontal scroll, or lost output scroll/one-line anchor. Inspect visual viewport offsets with `#viewport-debug` if needed.
- Tap actual suggestions and controls; scroll long output and dialog bodies with fingers. Confirm usable touch targets and stable dialog titles without treating DOM clicks as proof of hit-testing.
- Background and resume Safari/PWA, hide/show browser chrome, reload, and confirm persisted settings and saved files remain available. Record interruption, storage, startup, or worker/cache errors rather than clearing user data.
- In the installed PWA, repeat keyboard/rotation, touch, background/resume, relaunch, and offline/reconnect checkpoints. If installation, a secure origin, offline provisioning, or any step is unavailable, record it as pending/excluded, not passed. Proxy automation does not establish installed-PWA behavior.

Record actual results, not just an overall green mark:

```text
Date / tester:
Device model / actual iPadOS version:
Safari version / hardware keyboard (if any):
Mode: Safari tab | installed standalone PWA
Commit + dirty state / build configuration + artifact hashes or timestamps:
App URL + server root / TestCafe connection host + ports:
Runner alias / capabilities / exact command + filters:
Automated result: passed / failed / filtered (attach JSON, logs, screenshots):
Resize exclusions: eight cases (ACC-VIEW-004A-D and ACC-VIEW-005A-D)
Native keyboard / rotation shown + hidden / touch:
Background-resume / reload + storage / PWA relaunch + offline-reconnect:
Other exclusions + reasons / failures + reproduction steps:
Pending checkpoints / desktop Safari: not tested unless separately recorded:
Owned server/process cleanup or intentionally retained preview URL + owner:
```

An iPad pass does not establish iPhone or desktop Safari coverage. No Mac is available locally for desktop Safari; actual Safari validation would need available macOS CI or separately approved infrastructure. Automated WebKit is not a substitute for claiming desktop Safari tested.

## Logo Grading And Audits

[logo-results.js](logo-results.js) grades individual `Ok`, `Failed`, and `Error` lines and requires `All tests completed.`. [run-logo-node.js](run-logo-node.js) also fails on process errors, nonzero exits, signals, timeouts, and runtime crashes rather than trusting completion text alone. The recorded Node Logo baseline is **65 passing checks; OOP skipped**, not counted as passing. Browser Logo coverage is one functional harness case containing those language checks, not 65 additional TestCafe cases. Only the known final-`bye` Asyncify shutdown diagnostic after successful completion is allowed in the browser fixture; unrelated exceptions must still fail. Grader tests cover that distinction.

[lighthouse.test.js](lighthouse.test.js) remains separate: `npm run test:lighthouse` writes HTML and JSON to `reports/lighthouse/mobile/` and `reports/lighthouse/desktop/` (override root with `LIGHTHOUSE_REPORT_DIR`). It uses headless Chrome (`CHROME_BINARY` can select it) and the shared app-server helper, so `TESTCAFE_APP_URL` can reuse a known-fresh server. Thresholds remain **performance >=0.567 mobile / >=0.882 desktop**, **CLS <0.1**, and **accessibility, best-practices, SEO >=0.8** for both. Do not mix these audited thresholds with the optional, disabled-by-default autocomplete latency budget or treat audits as functional/device coverage.
