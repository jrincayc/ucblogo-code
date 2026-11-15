# Portable Acceptance Coverage Ledger

Status: functional TestCafe consolidation implemented; human iPad Safari signoff pending.
Inventory date: 2026-11-15.

All former browser assertions have destinations in the shared TestCafe suite; five source/unit assertions moved to Node's test runner. Stable IDs identify behaviors, not executable case counts. Multiple IDs can intentionally share a combined case, and one ID can cover multiple parameter variants. Every original ID is retained below; new rendered soft-key and device-viewport checks have additional IDs. Exact destination titles are in code spans, with the owning file linked at each section. Historical retired filenames below are not links.

## Inventory And Evidence

| Layer / Fixture | Desktop Cases | Mobile / Remote Cases | Implementation |
| --- | --- | --- | --- |
| `Logo word autocompletion` | 20 | 20 | [autocomplete.testcafe.js](autocomplete.testcafe.js) |
| `Logo JavaScript bridge` | 8 | 8 | [bridge.testcafe.js](bridge.testcafe.js) |
| `JavaScript error reporting` | 2 | 2 | [error-reporting.testcafe.js](error-reporting.testcafe.js) |
| `iPad startup and keyboard viewport simulation` | 2 | 2 | [ipad-keyboard.test.js](ipad-keyboard.test.js) |
| `Bundled Logo regression suite` | 1 | 1 | [logo-suite.testcafe.js](logo-suite.testcafe.js) |
| `Help dialog` | 20 | 17 | [help-dialog.testcafe.js](help-dialog.testcafe.js) |
| `Help dialog follows the Logo text size` | 3 | 3 | [help-font-size.testcafe.js](help-font-size.testcafe.js) |
| `Settings persistence` (includes UI) | 12 | 12 | [settings-persistence.testcafe.js](settings-persistence.testcafe.js) |
| `Viewport and output scrolling` | 14 | 6 | [viewport.testcafe.js](viewport.testcafe.js) |
| **Functional total** | **82** | **71** | Shared discovery and capability filtering in [run-testcafe.js](run-testcafe.js) |

The eleven excluded cases require `resize`: HELP-010 through HELP-012 and ACC-VIEW-004A through ACC-VIEW-004D and ACC-VIEW-005A through ACC-VIEW-005D. Desktop defaults to `resize,screenshots`; mobile/emulation/remote defaults to no capabilities. Those eleven cases are filtered out, not passed. Synthetic viewport cases and two screen-mode cases at the current device viewport remain eligible. Responsive autocomplete layout assertions adapt to available width; they do not add separate mobile cases.

Other recorded layers are **73 Node source/unit tests** across seven test files, and the direct Node Logo baseline of **65 passing checks with OOP skipped**. The runner contributes 27 of those tests; four real-Chrome checks are opt-in (`RUNNER_BROWSER_TESTS=1`). The bundled browser Logo suite is one TestCafe case containing the language checks, not 75 more browser cases.

Verified on 2026-11-15, Linux Mint 22.3, Node 25.7.0: Chrome 152 desktop **82/82**, Chrome 390x844 touch emulation **71/71**, Firefox 155 desktop **82/82**, and source/unit tests **69 passed / 4 optional browser checks skipped / 0 failed**. Browser tests used the fresh build produced by `emmake make ucblogo.html`; the pre-existing server on port 8080 served those artifacts. The bundled Logo browser suite reported 75 passed, 0 failed, 0 errors, with OOP skipped and its known `Tests.File.LiteralDribble` XFAIL.

Counts describe the current suite and the execution above, not proof of every browser. **A real phone and actual iPad Safari, including native keyboard/rotation/touch/lifecycle/PWA checkpoints, remain pending maintainer testing. Desktop Safari is not tested locally because no Mac is available.** Phone emulation is not an actual phone; iPad Safari does not establish desktop Safari, iPhone, or installed-PWA coverage. Use the [README.md](README.md) human record template to attach versions, build identity, commands, results, and exclusions. No paid hosted service is required.

## Source And Error Reporting

The source and unit layer runs with `npm run test:source` across seven files, including [source.test.js](source.test.js), help index/docs/loading, deployment, grader, and runner checks. Browser error cases are in [error-reporting.testcafe.js](error-reporting.testcafe.js), fixture `JavaScript error reporting`.

| ID | Exact Destination Title(s) | Assertions / Variants |
| --- | --- | --- |
| ACC-SRC-001 | `provides named click buttons in a named group without changing their size` | Source suite `soft-key accessibility`: shell application script parses; named click buttons/group and fixed 1.75em dimensions. Rendered counterpart ACC-UI-003. |
| ACC-SRC-002 | `replaces selected command text and restores the caret` | Source suite `soft-key accessibility`: selection replacement, caret, command focus; rendered counterpart ACC-UI-002. |
| ACC-SRC-003 | `replaces selected editor text and restores the caret` | Source suite `soft-key accessibility`: selection replacement, caret, editor focus; rendered counterpart ACC-UI-004. |
| ACC-ERR-001 | `skips local service workers and handles registration failures` | Source suite `startup error reporting`: localhost, IPv4/IPv6 loopback; nonlocal rejected registration warns; non-production builds (`Module.serviceWorker = false`) unregister instead of registering. |
| ACC-ERR-002 | `traces uncaught errors and unhandled rejections without suppressing them`; `uncaught browser errors appear in the visible diagnostic panel`; `unhandled rejections appear in the diagnostic panel without being suppressed` | First title is in source suite `startup error reporting`; other two are browser cases. Error/stack/source/line metadata, panel/status, and nonsuppression. Browser handlers are deliberately triggered; expected rejection handling is narrow, not global JS-error suppression. |

## Logo Suite

Destination: [logo-suite.testcafe.js](logo-suite.testcafe.js), fixture `Bundled Logo regression suite`. Grading is shared with [run-logo-node.js](run-logo-node.js) through [logo-results.js](logo-results.js), with [logo-results.test.js](logo-results.test.js) unit coverage.

| ID | Exact Destination Title | Assertions / Limits |
| --- | --- | --- |
| ACC-LOGO-001 | `ACC-LOGO-001 detects suite failures and requires its completion marker` | Runs bundled Logo suite; grades individual results and requires `All tests completed.`. `Failed`, `Error`, missing results/completion, and runtime crashes fail. Allows only the exact known final-`bye` Asyncify abort with the expected `doRewind` stack after successful grading; unrelated or premature exceptions fail. OOP is explicitly skipped, not passing. |

## Settings And Accessibility

Destination: [settings-persistence.testcafe.js](settings-persistence.testcafe.js), fixture `Settings persistence`. [testcafe-helpers.js](testcafe-helpers.js) waits for runtime/input readiness and captures/restores the owned settings key around reloads. This does not preserve unsaved Logo runtime state; use a separate test page.

| ID | Exact Destination Title | Assertions / Variants |
| --- | --- | --- |
| ACC-SET-001 | `ACC-SET-001 sets, reports, and steps the Logo text size` | Setter, reporter, increase/decrease, UI CSS/control values, runtime setting, and stored value. |
| ACC-SET-001c | `ACC-SET-001c typing a size in the text size box does not apply partial values` | Blank and out-of-range partial values do not change the setting while typing; valid values apply and clamp on commit. |
| ACC-SET-002 | `ACC-SET-002 keeps SVG label size independent of Logo text size` | Computed SVG LABEL size matches drawing font rather than UI text size. |
| ACC-SET-003 | `ACC-SET-003 applies dark mode and follows the system preference` | Dark colors/borders; system selection and computed system background. |
| ACC-SET-004 | `ACC-SET-004 applies text colors and full flat-control styling` | Native appearance before `settc`; two RGB pairs after it, editor/input/button/history styling, disabled treatment, device-pixel border tolerances. |
| ACC-SET-005 | `ACC-SET-005 preserves text colors across theme changes` | Logo-selected text foreground/background/border retain precedence across dark/light themes. |
| ACC-SET-006 | `ACC-SET-006 persists Logo and dialog settings across reload` | Font, theme, rendering, speak-output and visual-bell settings, restored controls/effects; speech and tone probes restored afterward. Probes do not prove audible output. |
| ACC-UI-001 | `ACC-UI-001 labels controls and manages settings dialog focus` | Native labels, named dialogs, modal focus entry/restoration, direct-child titles and scroll ownership; output replacement retains log semantics and focusability. |
| ACC-UI-002 | `ACC-UI-002 clicking a soft key replaces selected text and restores focus` | Command selection/caret/focus. Rendered key uses in-page click; desktop fallback uses a temporary click button. Not native touch evidence. |
| ACC-UI-003 | `ACC-UI-003 names light-DOM soft keys and preserves their size` | Named group and ten buttons, click handlers, no label/mousedown abuse, computed 1.75em sizing. Desktop creates/removes a temporary touch-capable component when needed. |
| ACC-UI-004 | `ACC-UI-004 clicking a soft key replaces selected editor text and restores focus` | Editor selection/caret/focus; rendered-key or temporary-button path as above. |

## Help Dialog

Destination: [help-dialog.testcafe.js](help-dialog.testcafe.js), fixture `Help dialog`.
The 20 desktop cases include word browsing/search, document loading/search/retry,
tabs, insertion into both command input and editor, stacked-dialog focus,
responsive layout, and 4.5:1 text contrast in light and dark themes. Three
resize-dependent cases are filtered from phone emulation; the remaining 17 pass
there. The three text-size cases are in [help-font-size.testcafe.js](help-font-size.testcafe.js).

## JavaScript Bridge

Destination: [bridge.testcafe.js](bridge.testcafe.js), fixture `Logo JavaScript bridge`. Browser speech/audio APIs are mocked and restored; these are transport assertions, not native audio or voice-availability signoff. Missing required speech support fails rather than silently counting a pass.

| ID | Exact Destination Title | Assertions / Variants |
| --- | --- | --- |
| ACC-BRIDGE-001 | `ACC-BRIDGE-001 binds variadic runjs parameters as values` | Direct JS `Module.runjs`, string and number. |
| ACC-BRIDGE-002 | `ACC-BRIDGE-002 binds variadic runjs parameters from Logo and preserves the writer` | Logo-origin string/numeric arguments; writer restored. |
| ACC-BRIDGE-003 | `ACC-BRIDGE-003 binds a large ordered parameter list from Logo` | 128 ordered parameters checked by weighted result. |
| ACC-BRIDGE-004 | `ACC-BRIDGE-004 passes sprite names to make and set turtle commands as bound values` | Sprite symbol and turtle reference. |
| ACC-BRIDGE-005 | `ACC-BRIDGE-005 passes speech configuration and speak text as bound values` | Language, pitch, rate, voice name, spoken text through mocks. |
| ACC-BRIDGE-006 | `ACC-BRIDGE-006 parses getvoices results through the Logo-to-JS bridge` | Mock voice result parsed in Logo. |
| ACC-BRIDGE-007 | `ACC-BRIDGE-007 keeps hostile bound strings as data across a fuzz corpus and 9000 characters` | Injection-like text, multiline marker, quotes/slashes, NUL, tab/CR; no execution; 9000-character word intact. |
| ACC-BRIDGE-008 | `ACC-BRIDGE-008 runs multiline JavaScript from a v-barred Logo word` | Packaged [runjs-multiline.lg](../runjs-multiline.lg) produces 42. |

## Viewport And Output

Destination: [viewport.testcafe.js](viewport.testcafe.js), fixture `Viewport and output scrolling`. Portrait resize variants use 500x900 -> 520x940; landscape uses 900x500 -> 940x520. Text/split-mode variants use the initial orientation dimensions. The eight rows marked `resize` are the **only default mobile/remote functional exclusions**. Simulation rows do not exercise a native keyboard or physical rotation.

| ID | Exact Destination Title | Variants / Capability |
| --- | --- | --- |
| ACC-VIEW-001 | `aligns the app to a keyboard-offset visual viewport and resets it afterward (simulation)` | Synthetic height/offsetTop/scale changes, viewport scroll, restoration; output scroll preserved. No resize requirement. |
| ACC-VIEW-002 | `shows viewport diagnostics only when requested and updates on viewport scroll (simulation)` | Opt-in hash, synthetic offsetTop update, pointer-events/transform. No resize requirement. |
| ACC-VIEW-003A | `resets document scroll on visual viewport resize without moving output (simulation)` | Parameter `eventName = visual viewport resize`; synthetic event. |
| ACC-VIEW-003B | `resets document scroll on orientationchange without moving output (simulation)` | Parameter `eventName = orientationchange`; synthetic event including delayed document scroll. |
| ACC-VIEW-004A | `preserves Logo output scroll position when resizing in portrait` | `orientation = portrait`, `singleLine = false`; requires `resize`. |
| ACC-VIEW-004B | `keeps one-line Logo output anchored when resizing in portrait` | `orientation = portrait`, `singleLine = true`; requires `resize`. |
| ACC-VIEW-004C | `preserves Logo output scroll position when resizing in landscape` | `orientation = landscape`, `singleLine = false`; requires `resize`. |
| ACC-VIEW-004D | `keeps one-line Logo output anchored when resizing in landscape` | `orientation = landscape`, `singleLine = true`; requires `resize`. |
| ACC-VIEW-005A | `preserves Logo output scroll position when switching screen modes in portrait` | `orientation = portrait`, `singleLine = false`; requires `resize`. |
| ACC-VIEW-005B | `keeps one-line Logo output anchored when switching screen modes in portrait` | `orientation = portrait`, `singleLine = true`; requires `resize`. |
| ACC-VIEW-005C | `preserves Logo output scroll position when switching screen modes in landscape` | `orientation = landscape`, `singleLine = false`; requires `resize`. |
| ACC-VIEW-005D | `keeps one-line Logo output anchored when switching screen modes in landscape` | `orientation = landscape`, `singleLine = true`; requires `resize`. |
| ACC-VIEW-006A | `preserves Logo output scroll position when switching screen modes at the device viewport` | `singleLine = false`; current device viewport, no resize requirement. |
| ACC-VIEW-006B | `keeps one-line Logo output anchored when switching screen modes at the device viewport` | `singleLine = true`; current device viewport, no resize requirement. |

## Autocomplete

Destination: [autocomplete.testcafe.js](autocomplete.testcafe.js), fixture `Logo word autocompletion`. All historical Selenium autocomplete assertions and the pre-existing TestCafe assertions share these 20 cases; there is no duplicate Selenium execution. ACC-AUTO-005/006 share a case, ACC-AUTO-008/009 share a case, and ACC-AUTO-015 through ACC-AUTO-019 share one width-adaptive layout case. ACC-AUTO-022 still has two explicit cases. All stable IDs remain traceable rather than being dropped to make case counts match.

Suggestions use in-page DOM `.click()` (including editor suggestions), not native pointer/touch actions; keyboard tests use TestCafe key simulation. Providers, owned settings, and captured legacy font-size values are restored. Desktop can resize to 1280x800; phone alias is 390x844 Chrome emulation. Layout assertions use actual available width rather than equating a device name with a layout guarantee.

| ID | Exact Destination Title(s) | Assertions / Variants |
| --- | --- | --- |
| ACC-AUTO-001 | `symbol source lists primitives, user procedures, variables, and library procedures` | Primitive, session procedure, variable, and autoloadable library entries. |
| ACC-AUTO-002 | `editor: completes words while Logo is suspended` | Editor selection completion; discard and wait for Logo idle. |
| ACC-AUTO-003 | `editor: Tab completion preserves newline and Escape behavior` | Tab completion, Enter newline, Escape and Ctrl+[ hide suggestions without closing editor or losing focus. |
| ACC-AUTO-004 | `stub: closed combobox until a matching word is typed` | Closed ARIA combobox/list semantics. |
| ACC-AUTO-005 | `stub: shows matching suggestions while typing and closes when nothing matches` | Incremental matching, option order, count/status; combined with next ID. |
| ACC-AUTO-006 | `stub: shows matching suggestions while typing and closes when nothing matches` | No-match closure in the same case, not a second run. |
| ACC-AUTO-007 | `stub: tapping a suggestion completes only the word at the caret` | Value/caret/focus/popup state after DOM click; not a native tap. |
| ACC-AUTO-008 | `stub: help summary and argument syntax, none for undocumented words` | Documented summary vs absent summary; combined with next ID. |
| ACC-AUTO-009 | `stub: help summary and argument syntax, none for undocumented words` | Primitive aliases/varargs syntax vs absent syntax in same case. |
| ACC-AUTO-010 | `stub: closes after the command is submitted` | Submission returns to idle with popup closed. |
| ACC-AUTO-011 | `live: no completion while Logo is blocked reading continuation lines` | Continuation state suppresses completion; closing list restores idle completion. |
| ACC-AUTO-012 | `live: no completion while a Logo program is running` | Evaluator busy during `wait 180`, then idle completion restored; retrying readiness replaces the old 2-second assumption. |
| ACC-AUTO-013 | `live: completes a real command from primitives with help` | Complete and execute a real repeat/forward/right command. |
| ACC-AUTO-014 | `live: suggests procedures and variables defined in the session` | New session names and colon-prefixed variable completion. |
| ACC-AUTO-015 | `layout: input and popup stay inside the window, fit, and are touch sized` | Input/popup visible bounds, document scroll; same case as next four IDs. |
| ACC-AUTO-016 | `layout: input and popup stay inside the window, fit, and are touch sized` | Popup/option width and no horizontal overflow. |
| ACC-AUTO-017 | `layout: input and popup stay inside the window, fit, and are touch sized` | Both tested option heights >=44 CSS px. Geometry, not native touch evidence. |
| ACC-AUTO-018 | `layout: input and popup stay inside the window, fit, and are touch sized` | Narrow-width branch: long `foreach` syntax wraps below word when it cannot fit inline. Exercise with phone emulation/portrait device. |
| ACC-AUTO-019 | `layout: input and popup stay inside the window, fit, and are touch sized` | Sufficient-width branch: syntax beside word. Exercise desktop width; do not claim both width branches ran on one fixed-width remote device. |
| ACC-AUTO-020 | `performance: popup renders within the measured latency budget` | Six in-page samples logged. **Default `POPUP_LATENCY_BUDGET_MS=0` disables threshold assertion**; only a configured positive measured budget gates latency. A default pass is not proof of a performance target. |
| ACC-AUTO-021 | `stub: keyboard navigates, accepts, and dismisses suggestions` | Up/down active option and highlight, Enter acceptance, Escape dismissal, input/focus unchanged as appropriate. |
| ACC-AUTO-022 | `stub: Tab accepts the first suggestion without leaving the input`; `stub: Tab accepts the active suggestion without leaving the input` | Two distinct cases; first vs keyboard-selected option. |
| ACC-AUTO-023 | `stub: Ctrl+[ dismisses suggestions like Escape` | Same dismissal semantics without losing input/focus. |
| ACC-AUTO-024 | `global: Ctrl+[ closes an open modal dialog like Escape` | Settings modal closes. |
| ACC-AUTO-025 | `global: Ctrl+[ with nothing to dismiss leaves the command line alone` | Value/focus unchanged. |

## Startup And Keyboard Simulation

Destination: [ipad-keyboard.test.js](ipad-keyboard.test.js), fixture `iPad startup and keyboard viewport simulation`, included by shared discovery on desktop and remote alike. Its historical filename does not establish that it has run on an iPad.

| ID | Exact Destination Title | Assertions / Limits |
| --- | --- | --- |
| ACC-IPAD-001 | `Logo starts and displays its welcome message` | Startup smoke: welcome text, enabled input, no startup exception status. |
| ACC-IPAD-002 | `simulated keyboard resizes the app without losing output position` | Temporarily override Visual Viewport height and add an overlay; verify app height/output scroll and restore descriptors/overlay afterward. **Simulation only**, even if executed in remote Safari. |

Real native keyboard transitions, portrait -> landscape -> portrait with keyboard shown/hidden, physical touch, background/resume, storage/reload, and installed-PWA relaunch/offline behavior require the human checkpoints and result template in [README.md](README.md). They remain pending, not silently excluded or counted as automated passes.

## Separate Lighthouse Audits

Destination: [lighthouse.test.js](lighthouse.test.js), Mocha suite `Lighthouse acceptance`. These two audits are not discovered as TestCafe fixtures, not included in 57/49, and not part of `npm test` or `test:built`. Run `npm run test:lighthouse` explicitly against known-fresh artifacts.

| ID | Exact Destination Title | Thresholds / Artifacts |
| --- | --- | --- |
| ACC-LH-001 | `mobile meets baseline scores and writes HTML and JSON reports` | Performance >=0.567; CLS <0.1; accessibility, best-practices, SEO >=0.8. HTML/JSON in `reports/lighthouse/mobile/`. |
| ACC-LH-002 | `desktop meets baseline scores and writes HTML and JSON reports` | Performance >=0.882; CLS <0.1; accessibility, best-practices, SEO >=0.8. HTML/JSON in `reports/lighthouse/desktop/`. |

## Historical Baseline And Retirement

The original Selenium baseline was **63 passing, 7 failing**. Its seven failures were **two running-state waits** (one per desktop/phone profile: a 2-second idle timeout versus Logo `wait 180`, approximately 3 seconds), **four cascading live cases** after those timeouts contaminated shared runtime/input state, and **one phone tap-helper `Pointer` error**. Total: 2 + 4 + 1 = 7. There were not two pointer failures. Those were baseline harness failures, not seven accepted feature exclusions.

The shared fixtures now isolate test state and wait on observable runtime conditions; all old assertions are mapped above, including the formerly failing running/live checks. Historical `settings-persistence.test.js`, `autocomplete.test.js`, and `error-reporting.test.js` are superseded by these destinations. Their retirement and removal of the `selenium-webdriver` dependency are consolidation cleanup; no docs link depends on those retired files. Mocha/Chai remain solely for the separate Lighthouse harness in this acceptance directory.

Older partial-suite pass totals do not establish the expanded integrated suite or real-device coverage. Keep actual execution reports distinct from this implementation inventory. The shared runner writes `reports/testcafe.json`; request screenshots with `--screenshots path=reports/testcafe/screenshots`, retain per-platform reports, and record filters, missing capabilities, browser/OS versions, and fresh build identity. See [README.md](README.md) for the build-once/built-artifact command contract, deprecated environment warnings, bounded cleanup, and LAN remote workflow.
