# Project Map And Validation

Paths below are relative to the repository root. Verify symbols, recipes, prerequisites, and limitations in current source; these are navigation anchors, not immutable contracts.

## Interpreter

| Concern | Source Anchors |
| --- | --- |
| Architecture / mini-Scheme | `plm`: SICP 4.1/5.4, explicit control, dynamic scope, continuations, tailforms, data types, GC. Some details are out of date. |
| Native/wx interpreter entry | `main.c`: `main`/wx `start`, `set_bottom_stack`, initialization, startup, reader/evaluator loop, signals/shutdown. |
| Web entry | `webmain.c`: `main`, `run_evaluator`, stack roots, `LOGO_GIL` transitions and callback-driven evaluation. |
| wx application entry | `wxMain.cpp`, `wxTerminal.cpp`: application registration, `LogoApplication::OnInit`/`OnRun`, `start`, event loop/working directory. |
| Initialization/primitive registration | `init.c`, `globals.h`, `logo.h`: primitive table/arity/dispatch, symbols, types, globals, prototypes, guards/library paths. |
| Reader/runparser/treeification | `parse.c`, `paren.c`, `wrksp.c`: `reader`, `parser`, `runparse`, input/escaping, call boundaries, arity and cached procedure representations. |
| Evaluator | `eval.c`, `eval.h`, `logo.h`, `coms.c`: labels, registers, stacks/save/restore, continuations, tail calls, dynamic binding and control primitives. |
| Errors/nonlocal exit | `error.c`, `coms.c`, entry points: error recovery, throw/catch, pause/stop and restored state. |
| Memory/data | `mem.c`, `logo.h`, `logodata.c`, `lists.c`, `intern.c`: NODE/sentinel/string/array representation, allocation, segments, roots, mark/sweep, generations/oldyoung, write barriers. |
| GC debug aids | `gdb.rc`, `address-sanitizer.txt`, `mem.c`, `pre.js`: `logo_gc_debug_*` exports and `Module.logoGC`; inspect at safe runtime states. |
| Workspace/ordinary primitives | `wrksp.c`, `math.c`, `lists.c`, `logodata.c`, `coms.c`: definition/variable/property state, arithmetic/list/control semantics and errors. |

## Runtime And UI

| Concern | Source Anchors |
| --- | --- |
| Asyncify helpers | `liblogo.js`, `files.c`, `weblib.c`: `Asyncify.handleSleep`, `EM_ASYNC_JS`, bridge dependencies, coordination. |
| JS setup/lifecycle | `pre.js`: `Module.preRun`, `Module.onRuntimeInitialized`, environment, TTY overrides, browser/Node guards. |
| C/JS bridge | `weblib.c`, `init.c`, `webstartup.lg`, `webedition.md`: `runjs`, bindings/serialization, primitives and wrappers. |
| Queued stdin | `pre.js`, `files.c`, `parse.c`, `webmain.c`: `runLogo`, `FS_runjs_getChar_buffer`, `getcharready`/`awaitgetcharready`, `blockedOnGetc`, `em_getc`/`em_ungetc`, `sysGetC`. |
| Animation/callbacks | `pre.js`, `webmain.c`, `weblib.c`, `shell.html`: scheduling/cancellation, `LOGO_GIL`, `Asyncify.state`, evaluator/GC flags and guarded entry. |
| Safe UI-event signaling | `pre.js`: `UIEventQueue`, `Module.uievents`, `Module.uieventypes`, event producers and guarded `Module.run_evaluator`; `liblogo.js`: `peek_uievent`/`get_uievent`; `eval.c`, `webmain.c`: consumption/dispatch and `buttonact`. Queue events while running; start the interpreter normally when idle, never reenter an active C invocation. |
| File operations/persistence | `files.c`, `wrksp.c`, `pre.js`: streams/prefixes, load/save/edit/upload/download, IDBFS `/d`, `FS.syncfs(true, ...)`, `autoPersist` and localStorage settings. |
| Browser/Node VFS split | `Makefile.am`, `pre.js`, `files.c`: embedded browser assets, MEMFS/IDBFS versus `ENVIRONMENT=node`/`NODERAWFS=1` host access. |
| Output/terminal | `print.c`, `term.c`, `pre.js`, `shell.html`: streams, flush/partial lines, HTML strings, output contexts/cursor and DOM. |
| wx terminal/editor | `wxterm.c`, `wxTerminal.cpp`, `wxTerminal.h`, `wxCommandHistory.cpp`, `TextEditor.cpp`: C/C++ bridge, input/event yielding/history/editor. |
| Shared graphics | `graphics.c`, `turtleshape.c`, `turtleshape.h`: turtle state/semantics and backend selection. |
| Web graphics/UI | `webgraphics.c`, `webgraphics.h`, `pre.js`, `shell.html`: SVG/DOM/sprites/labels, CSS/themes, controls/dialogs/editor and mobile input. |
| wx graphics | `wxTurtleGraphics.cpp`, `wxTurtleGraphics.h`, `wxGraphics.h`: native drawing/redraw/events. |
| PWA/deployment | `workbox-config.js`, `manifest.json`, `shell.html`, `webserver.js`: cache generation/registration, assets and serving. |

## Build State

- `configure.ac` owns Autoconf platform/options checks; `Makefile.am` owns source selection and custom Node/browser links. Generated outputs are not primary edit targets.
- `emconfigure ./configure --enable-wasm` selects the Web Edition (default target remains wx): wx/X11/docs off, gitid on, prefix `./dist`, `-O3`, Asyncify link settings in `WEB_LDFLAGS`. `npm run build:web` runs autoreconf, configure, clean, `make`, `make check`. It changes the root configuration and is not a narrow harmless test command. `emscript.sh` is legacy and no longer used by npm scripts.
- Profiles map to configure options: prod = default (service worker on); dev = `--with-debug-info=dwarf|separate-dwarf|source-map`; debug = add `--enable-asan`/`--enable-ubsan` and `--enable-assertions`/`--enable-stack-overflow-check` (`--enable-safe-heap` is rejected with ASAN). Any non-production option defaults to `-Og`, forbids the service worker, and makes the page unregister an old worker via `web-config.js`. `EXIT_RUNTIME` is unchanged under ASAN.
- Asyncify is required to spill Wasm registers to the heap for GC roots. `mem.c` uses `mark_wasm_stack`, `emscripten_scan_stack`, and `emscripten_scan_registers`; JSPI cannot replace this with the current collector. Direct links must retain the script's Asyncify settings. A future `ASYNCIFY=2` migration is unproven and failed on the maintainer's last attempt.
- Browser links run `make install-data` first and embed `$(pkgdatadir)` as `/share/ucblogo`, plus tests, `web.lgo`, an icon, and `webstartup.lg` as `/share/ucblogo/logolib/startup.lg`. Library-file edits alone do not trigger a relink; delete `ucblogo.html`. `makehelp` uses `CC_FOR_BUILD`; a fresh tree needs `make regenerate-help-index` once.
- Node uses shared Wasm objects but different environment/filesystem flags. Check both links for shared exports/dependencies.
- `ucblogo.html`, loaders, Wasm/debug files, generated workers/Workbox assets are build outputs. Check artifact/configuration/cache consistency when observed behavior disagrees with source.
- Native build instructions: `README.md`; wx checks: `configure.ac`. Inspect `wx-config`, Autotools macros, docs tools, compiler and active configuration. Verify custom recipe out-of-tree support before relying on it.
- Read `.github/workflows/`, `winbuild.sh`, and platform packaging files when their pipeline/platform is affected.

## Validation Entry Points

Run from the repository root unless a verified isolated build supports otherwise. Check dependencies and active build state first; do not run every command on every task.

| Goal | Existing Command | Caveat |
| --- | --- | --- |
| Full web build / Node suite | `npm run build:web` | Requires Emscripten, Autotools, Node/npm dependencies; cleans/reconfigures root build. Incremental: `emmake make` then `make check`. |
| Browser relink | `emmake make ucblogo.html` | Uses configured `WEB_LDFLAGS`; restages data via `install-data`. |
| Node Logo suite, no rebuild | `npm run test:logo:node` | Runs `node tests/acceptance/run-logo-node.js` against the existing Node build; checks individual results, 65 tests with OOP skipped; suite ends with `bye`. |
| Source and runner checks, no rebuild | `npm run test:source` | Runs `node --test tests/acceptance/source.test.js tests/acceptance/logo-results.test.js tests/acceptance/runner.test.js`; no browser application rebuild. |
| Build plus source and all functional browser tests | `npm test` | Runs `npm run build:web` then `npm run test:built`; no Lighthouse/audits. |
| Source and all functional browser tests, no rebuild | `npm run test:built` | Source checks plus shared TestCafe suite: all 57 functional browser cases, 49 mobile-capable; includes the Logo suite with explicit OOP skip. Requires current application artifacts. |
| Build plus all functional browser tests | `npm run test:browser` | Runs `npm run build:web` then `npm run test:browser:built`; no audits. |
| All functional browser tests, no rebuild | `npm run test:browser:built` | Runs `node tests/acceptance/run-testcafe.js`; owns an available-port `emrun` server with HTTP readiness checks, or reuses `TESTCAFE_APP_URL`. Fixtures wait for runtime/input readiness. |
| Focused browser case | `node tests/acceptance/run-testcafe.js --test 'exact test name'` | Exact name, not a substring; use `--test-grep 'regex'` for a regex. Shared runner provides a portable Node whole-run watchdog; GNU `timeout` is not required. |
| Browser Logo language suite | `node tests/acceptance/run-testcafe.js --fixture 'Bundled Logo regression suite'` | Uses `tests/test.lg`; checks completion and individual failures/errors. 65 Logo tests with OOP explicitly skipped, not passing coverage. |
| Settings fixture | `npm run test:settings`, `npm run test:settings:built` | Build/built-only wrappers select the settings fixture through the shared runner; Chrome/Firefox wrappers use `TESTCAFE_BROWSER`. |
| Autocomplete (TestCafe) | `npm run test:autocomplete`, `npm run test:autocomplete:desktop`, `npm run test:autocomplete:phone` | Desktop/phone wrappers select the autocomplete fixture through the shared runner; use current application artifacts, not raw `--app`/fixed startup-delay commands. |
| Lighthouse | `npm run test:lighthouse` | Separate browser performance/audit checks, not language correctness coverage. |
| Remote iPad browser suite | `npm run test:ipad:remote` | Runs `node tests/acceptance/run-testcafe.js --browser remote`; requires LAN-reachable `TESTCAFE_HOSTNAME` and `TESTCAFE_APP_URL`, proxy ports `38079,38080`, and human device connection. Separate connection and whole-run watchdogs. |
| Native/wx build | `autoreconf --install`, `./configure --enable-wx`, `make` | Preserve isolated build state; optionally `--disable-docs` when intentionally skipping unavailable docs tools. Confirm wx was selected. |
| Serve built browser app | `emrun --no-browser --port PORT ucblogo.html` | Reuse suitable server/browser; use available alternative if emrun fails. Wait for input readiness. |

The shared runner uses `TESTCAFE_BROWSER` (default headless Chrome), `TESTCAFE_HEADLESS=0` for headed runs, and `TEST_DELAY_MS` for slow observation. `TESTCAFE_APP_URL` is preferred for reusing an existing server; legacy `UCBOLOGO_TEST_URL` warns, as do `BROWSERS` and `HEADLESS` (migrate to `TESTCAFE_BROWSER` and `TESTCAFE_HEADLESS`). `TESTCAFE_WATCHDOG_MS` bounds the whole run using portable Node supervision and cleanup of owned processes; `TESTCAFE_BROWSER_INIT_TIMEOUT` separately bounds remote-device connection waiting. Do not replace readiness checks with a fixed startup delay. Built-only commands do not rebuild: new browser fixtures alone need no application rebuild, but embedded Logo/application-source changes do. Keep Lighthouse separate and retain its existing audit options.

For a single Logo regression, read `tests/_how-to.txt`, `tests/UnitTests.lg`, `tests/test.lg`, and the nearest `UnitTests-*.lg` for setup/invocation. Start collector tests at `tests/UnitTests-MemMgr.lg`; `tests/runjs-multiline.lg` is a bridge regression example.

The programming skill records this browser full-suite entry point:

```logo
ignore runjs "|FS.chdir('/')| load "tests/test.lg
```

It changes the VFS working directory and ends with `bye`, terminating Logo. Prefer a dedicated test session, distinguish post-exit loader errors from test results, and inspect current failure aggregation/OOP exclusions before claiming success.

## Documentation

- `webedition.md`: web features, interop semantics, limits and usage.
- `usermanual`, `makehelp.c`, `helpfiles/`: help generation and indexes; changes can overwrite manual help edits.
- `docs/ucblogo.texi`, `docs/Makefile.am`: Texinfo manual and generated formats; reconcile overlapping manual sources deliberately.
- `plm`: conceptual interpreter/GC documentation; verify its outdated details and update misleading contracts when changing architecture.
- `README.md`, `changes.txt`, `TODO`, `address-sanitizer.txt`: build/user/development context requiring current-source verification.
- `logolib/`, `csls/`, `webstartup.lg`, `contrib/`, `animationdemo.lg`, `animationdemo2.lg`, `htmldemo.lg`: dialect/library conventions and runnable examples.