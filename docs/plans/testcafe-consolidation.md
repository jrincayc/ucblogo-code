# Project Plan: Consolidate Browser Tests In TestCafe

Status: Implemented; automated validation complete; human iPad Safari signoff pending.
Date: 2026-11-13

## Execution Record

- Functional acceptance uses one TestCafe suite: Chrome 152 desktop passed
  57/57, Chrome 390x844 touch emulation passed 49/49, and Firefox 155 passed
  57/57 on Linux Mint 22.3. Eight window-resize cases are capability-filtered
  from mobile/remote runs; simulation is not physical-device coverage.
- Browser and direct Node Logo grading each report 65 passing checks and OOP
  skipped. Portable runner self-tests passed 27/27, including deliberate
  assertion/startup/hung-action failures, signals, reporter flushing, and cleanup.
- Selenium is retired. Source checks use Node; Lighthouse remains separate.
- Tests used the matching existing 2026-11-13 17:55 HTML/JS/Wasm artifacts, newer
  than their application inputs. No application source changed or was rebuilt.
- [Coverage ledger](../../tests/acceptance/COVERAGE.md) maps all original checks;
  [usage guide](../../tests/acceptance/README.md) provides the shared commands,
  capability exclusions, and pending real-iPad human checkpoints.

The baseline below describes pre-migration sources and scripts.

## Objective

Use TestCafe as the single browser acceptance-test harness, with the same test
definitions runnable on local browsers, GitHub Actions, and the maintainer's
physical iPad over LAN. Preserve existing assertions and make focused runs easy.

Companion project: [Browser and Node.js CI](browser-node-ci.md). Its browser
capability probes can proceed in parallel; the complete CI suite depends on this
migration's shared runner and coverage inventory.

## Scope And Boundaries

- Move all browser functional acceptance tests from Selenium/Mocha/Chai to
  TestCafe, including cases not currently selected by npm scripts.
- Keep Logo regression cases in Logo. Run them through TestCafe in browsers and
  directly through the Node.js or native interpreter outside browsers. TestCafe
  is not a Node.js or native-interpreter test runner.
- Preserve source-only JavaScript checks as focused Node tests where they test
  syntax or isolated functions; add equivalent browser-observable assertions
  where applicable. Do not lose checks merely because they are not browser tests.
- Keep Lighthouse as a specialized Chrome audit tool, not a TestCafe replacement
  for performance and accessibility scoring. Its Mocha wrapper can remain, or
  move separately to Node's test runner without changing audit coverage.
- Do not rewrite application behavior, the Logo suite, native wx tests, or the
  build system wholesale as part of the harness migration.
- Do not add a parallel Playwright suite. Retain TestCafe's remote-browser path
  for the real-iPad requirement.

## Current Baseline

| Source | Current Role | Planned Disposition |
| --- | --- | --- |
| [package.json](../../package.json) | Separate Selenium settings/error scripts, TestCafe autocomplete and iPad scripts, and Lighthouse | Provide consistent browser-suite entry points while preserving useful wrappers |
| `settings-persistence.test.js` (retired) | Browser Logo suite, accessibility/focus, viewport/scroll, JS bridge, speech, text/theme settings and reload persistence; also source-only soft-key checks | Split into focused TestCafe fixtures; source checks retained separately |
| `error-reporting.test.js` (retired) | Source-only error-reporting checks | Retained in Node; browser diagnostic assertions added |
| `autocomplete.test.js` (retired) | Selenium autocomplete coverage, including editor behavior | Reconciled with TestCafe before retirement |
| [autocomplete.testcafe.js](../../tests/acceptance/autocomplete.testcafe.js) | Existing TestCafe autocomplete coverage | Reuse and fill parity gaps, rather than porting duplicates |
| [ipad-keyboard.test.js](../../tests/acceptance/ipad-keyboard.test.js) | TestCafe remote-browser startup and simulated-keyboard proof of concept | Integrate into shared discovery; distinguish simulation from actual keyboard checks |
| [application-server.js](../../tests/acceptance/application-server.js) | Shared server lifecycle for existing Mocha tests | Reuse or adapt once for TestCafe and audits |
| [lighthouse.test.js](../../tests/acceptance/lighthouse.test.js) | Mobile/desktop Lighthouse scores and report artifacts | Retain separate audit coverage |
| [test.lg](../../tests/test.lg) and [UnitTests.lg](../../tests/UnitTests.lg) | Language regression entry point and framework | Retain; make browser and Node result grading reliable |

The former iPad npm script selected only its keyboard fixture, not the complete
acceptance suite. Existing suites use different default URLs/ports. A common
remote-browser command must select all applicable fixtures and preserve storage
across reload within a persistence test.

## Implementation Milestones

### 1. Inventory And Establish Parity

- [x] Enumerate actual cases, parameterized variants, assertions, current skips,
  and script-discovery gaps in all acceptance files. Assign stable case IDs.
- [x] Record each case's destination: existing TestCafe equivalent, new fixture,
  source-only Node test, Lighthouse audit, or explicitly justified exclusion.
- [x] Record a baseline on fresh artifacts in focused desktop Chrome runs.
  Identify pre-existing failures without weakening expectations to hide them.
- [x] Check the project's Node/tool dependency requirements and lockfile before
  changing dependencies. Keep application runtime support separate from the
  Node version required to execute browser-testing tools.

Exit criterion: every existing check has a disposition and a reproducible
invocation; test counts alone are not sufficient evidence of parity.

### 2. Establish One Portable Runner Contract

- [x] Use a consistent fixture naming/discovery rule, including the existing
  iPad file whose name currently differs from the TestCafe suffix.
- [x] Define shared configuration for application URL, browser target, exact
  case/fixture filtering, reports, timeouts, headed mode, and optional viewer
  delays. Reconcile existing environment variables instead of adding aliases
  without a migration path.
- [x] Separate build-and-test commands from built-artifact-only commands. Build
  once before a suite; a failed build must never fall through to stale outputs.
- [x] Support starting an owned server or reusing an explicitly supplied URL.
  Wait for real server and interpreter readiness, not fixed startup delays.
- [x] Default to proxy automation for the common portability baseline
  (`--disable-native-automation`). Treat Chromium native automation as a
  separately validated optimization, not a requirement for test correctness.
- [x] Share small helpers for input submission, expected output, runtime
  readiness, DOM state, and diagnostics. Do not enter active/suspended Wasm C
  code to inspect state; use DOM/JS snapshots and existing queued input paths.
- [x] Add effective action/run watchdogs and cleanup. Use a portable process
  watchdog or platform-specific wrappers; GNU `timeout` is not present by
  default on every target OS. Remote-device connection time is a separate budget.
- [x] Isolate test-owned settings/files and restore temporary mocks. Preserve
  real-iPad user data; do not use blanket site-data or localStorage clearing.

Exit criterion: one representative startup/input case and one reload-persistence
case use the shared contract in Chrome and Firefox, including a deliberate
failure that returns nonzero and leaves no owned processes behind.

### 3. Migrate Incrementally By Behavior

All seven groups are migrated; the coverage ledger records exact destinations.

1. Port startup, JavaScript error reporting, and diagnostic assertions.
2. Port soft-key accessibility, selection/caret behavior, modal focus, and source
   checks without substituting implementation-string checks for rendered behavior.
3. Port Logo/JS bridge cases, including multiline source, hostile/fuzz strings,
   long payloads, variadic ordering, sprite and speech bindings, and fallbacks.
4. Port settings, independent theme/text-color tests, their precedence interaction,
   SVG label-size isolation, and persistence after reload.
5. Port viewport/scroll/screen-mode cases. Label synthetic visual-viewport and
   keyboard manipulations as simulation, not real-device input coverage.
6. Reconcile terminal and editor autocomplete coverage, including suspended
   interpreter editing, newline behavior, Escape/Ctrl+[, layout, and touch targets.
7. Port the browser execution of the bundled Logo suite into a dedicated fixture
   because its final `bye` terminates the runtime.

For each group, run the smallest migrated case first, compare it with the old
assertions, then run that group in Chrome and Firefox. Fix harness differences
without changing expected application behavior. Keep the old implementation until
its replacement is demonstrated; do not permanently run duplicate suites.

### 4. Make The Shared Suite Real-iPad Ready

- [x] Provide one remote command that selects the complete applicable functional
  suite, plus filters for a short smoke pass and device-specific cases.
- [x] Retain a LAN-reachable TestCafe hostname, both proxy ports, connection
  URL/QR output, explicit app URL, and clear firewall instructions. Loopback on
  the iPad is not the development machine.
- [x] Audit desktop-only operations such as window resizing, screenshots,
  downloads, multi-window APIs, and OS-level keyboard actions. Adapt tests by
  declared capability; do not silently skip shared assertions on mobile.
- [x] Use explicit capability metadata rather than treating touch support as
  synonymous with a phone. An iPad can present a desktop user agent.
- [x] Separate automated assertions from human checkpoints for native keyboard,
  portrait/landscape rotation, touch, background/resume, and installed-PWA behavior.
  Synthetic events or DOM clicks do not prove native input behavior.
- [ ] Coordinate a real iPad session after automated checks pass. Record device,
  iPadOS/Safari version, commit/build identity, passed cases, exclusions, and
  human observations. Do not claim every test is remotely automatable.

Exit criterion: the common suite runs remotely on the maintainer's iPad, with
unsupported operations reported explicitly and native-device checks recorded.

### 5. Retire Redundant Harnesses And Document Usage

- [x] Remove Selenium files/helpers and `selenium-webdriver` only after the parity
  inventory is complete. Retain Mocha/Chai if Lighthouse or other retained checks
  still use them; remove dependencies only when no consumers remain.
- [x] Make the main test entry point include all functional fixtures rather than
  just settings. Keep Logo/Node, source checks, and audits separately invocable.
- [x] Document full, focused, headed, emulated-mobile, Safari, and remote-iPad
  commands. Clearly mark commands that rebuild versus reuse artifacts.
- [x] Update affected testing guidance and skill references when the migration
  lands, replacing the current Selenium-specific advice with verified commands.

## Validation And Completion

- [x] Every baseline case is represented, with no unexplained exclusions or
  weakened assertions; runtime and source-only failures both fail their commands.
- [x] Chrome and Firefox run the common suite; mobile emulation is identified
  as emulation. Safari/Edge runs use the companion CI capability work.
- [x] An intentionally failed assertion, browser startup failure, and hung action
  each produce a bounded nonzero result and useful diagnostics.
- [x] The Logo-suite adapter detects `Failed`/`Error`, missing completion, crashes,
  and known skips rather than trusting `bye` or exit code alone. Account for
  multiline result formatting and distinguish expected error-test output.
- [x] Persistence tests preserve state across reload, but do not leak it between
  unrelated cases. Mocked speech/audio paths remain separate from native checks.
- [ ] The real-iPad workflow is demonstrated with human participation; periodic
  iPad use does not require maintaining a second implementation of each test.

## Risks And Delivery Order

TestCafe's proxy can affect JS rewriting, service workers, secure contexts, and
Wasm loading. Prove a representative interop/persistence case early. Browser
capability differences require narrow, documented exclusions, not broad skips.
Keep focused runs serial until persistence and shared runtime isolation are proven.

Deliver as reviewable changes: inventory/runner, individual behavior groups,
iPad compatibility, then harness removal. No dates are promised before baseline
failures and case counts are known. Implement only the minimum build changes
needed for a trustworthy test contract; coordinate broader build profiles with
the companion project rather than making them a hidden migration prerequisite.
