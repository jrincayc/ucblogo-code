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
- `emscript.sh` currently supplies optimized Asyncify settings, regenerates/configures Autotools with wx/X11/docs/objects disabled, cleans/rebuilds Node and browser, runs the Node Logo suite, and generates the service worker. It changes the root configuration and is not a narrow harmless test command.
- Maintainer workflow: clean build via `emscript.sh`, previously editing flags/debug levels and toggling service-worker generation. Requested future profiles: `prod` (`-O3`, service worker), `dev` (`-g`, no worker), `debug` (`-g`, ASAN and maximal compatible checks). They are goals, not existing commands. Debug defaults to no worker unless directed otherwise.
- Asyncify is required to spill Wasm registers to the heap for GC roots. `mem.c` uses `mark_wasm_stack`, `emscripten_scan_stack`, and `emscripten_scan_registers`; JSPI cannot replace this with the current collector. Direct links must retain the script's Asyncify settings. A future `ASYNCIFY=2` migration is unproven and failed on the maintainer's last attempt.
- Browser links embed staged `/share/ucblogo` resources, tests, `web.lgo`, an icon, and `webstartup.lg` as `/share/ucblogo/logolib/startup.lg`. Inspect staging prerequisites; embedding does not itself guarantee staging is current.
- Node uses shared Wasm objects but different environment/filesystem flags. Check both links for shared exports/dependencies.
- `ucblogo.html`, loaders, Wasm/debug files, generated workers/Workbox assets are build outputs. Check artifact/configuration/cache consistency when observed behavior disagrees with source.
- Native build instructions: `README.md`; wx checks: `configure.ac`. Inspect `wx-config`, Autotools macros, docs tools, compiler and active configuration. Verify custom recipe out-of-tree support before relying on it.
- Read `.github/workflows/`, `winbuild.sh`, and platform packaging files when their pipeline/platform is affected.

## Validation Entry Points

Run from the repository root unless a verified isolated build supports otherwise. Check dependencies and active build state first; do not run every command on every task.

| Goal | Existing Command | Caveat |
| --- | --- | --- |
| Full web build / Node suite | `./emscript.sh` | Requires Emscripten, Autotools, Node/npm dependencies and staged resources; cleans/reconfigures root build. |
| Browser relink | `emmake make ucblogo.html` | Needs valid Wasm objects/configuration/staging and the intended Asyncify link flags; current script environment is not automatically preserved. |
| Existing Node Logo suite | `node ./ucblogo-node.js tests/test.lg` | Review individual results; suite ends with `bye`. |
| Existing browser acceptance, no rebuild | `npm run test:settings:built` | Currently Mocha/Selenium settings-persistence and error-reporting tests; inspect server/browser prerequisites. |
| Focused browser case | `npx mocha tests/acceptance/settings-persistence.test.js --grep 'CASE'` | Select an actual test name; inspect test-managed server and `BROWSERS` settings. |
| Rebuild plus acceptance | `npm run test:settings` | Runs `emscript.sh` first; Chrome/Firefox wrapper scripts exist. |
| Lighthouse | `npm run test:lighthouse` | Separate browser performance/audit checks, not language correctness coverage. |
| iPad keyboard | `npm run test:ipad:remote` | TestCafe remote browser workflow requires device/browser participation. |
| Native/wx build | `autoreconf --install`, `./configure --enable-wx`, `make` | Preserve isolated build state; optionally `--disable-docs` when intentionally skipping unavailable docs tools. Confirm wx was selected. |
| Serve built browser app | `emrun --no-browser --port PORT ucblogo.html` | Reuse suitable server/browser; use available alternative if emrun fails. Wait for input readiness. |

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