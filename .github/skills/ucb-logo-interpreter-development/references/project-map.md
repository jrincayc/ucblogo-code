# Project Map

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

## Documentation

- `webedition.md`: web features, interop semantics, limits and usage.
- `usermanual`, `makehelp.c`, `helpfiles/`: help generation and indexes; changes can overwrite manual help edits.
- `docs/ucblogo.texi`, `docs/Makefile.am`: Texinfo manual and generated formats; reconcile overlapping manual sources deliberately.
- `plm`: conceptual interpreter/GC documentation; verify its outdated details and update misleading contracts when changing architecture.
- `README.md`, `changes.txt`, `TODO`, `address-sanitizer.txt`: build/user/development context requiring current-source verification.
- `logolib/`, `csls/`, `webstartup.lg`, `contrib/`, `animationdemo.lg`, `animationdemo2.lg`, `htmldemo.lg`: dialect/library conventions and runnable examples.
- Maintain `webedition.md`, help, `usermanual`, `docs/ucblogo.texi`, build docs, tests, and examples as appropriate. `makehelp` regenerates help from `usermanual`; check regeneration before editing help that it would overwrite. Resolve duplicate documentation deliberately.
- Update materially misleading `plm` explanations when changing architectural contracts, preserving useful historical context. Document semantics, compatibility, prerequisites, and known limits rather than speculative promises.
