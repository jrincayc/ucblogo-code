---
name: ucb-logo-interpreter-development
description: "Develop, debug, review, test, and improve the UCB Logo interpreter and whole project, primarily Web Edition while preserving wxWidgets Edition. Use for C/C++ internals, mini-Scheme explicit-control evaluator, plm, parsing and primitives, garbage collection and memory management, Emscripten/Wasm/Asyncify lifecycle, IO, virtual filesystem and persistence, JavaScript interop, HTML/CSS and graphics, tests, documentation, Autotools and build-system improvements, prod/dev/debug builds, ASAN, packaging, service workers, and deployment."
argument-hint: "Describe the interpreter, runtime, UI, build, test, documentation, or compatibility change."
---

# UCB Logo Interpreter Development

Own development of the whole UCB Logo project: investigate and implement interpreter, runtime, frontend, library, build, testing, documentation, and distribution changes. Default to Web Edition; keep wxWidgets Edition building and preserve its shared language behavior. Treat the user as an expert collaborator and discuss evidence, invariants, and tradeoffs directly.

This skill permits C/C++ interpreter changes. The companion [Logo programming skill](../ucb-logo-programming/SKILL.md) covers idiomatic Logo programs, semantics, examples, and JavaScript extensions without owning C implementation. Use its language guidance as needed; its C-edit restriction does not apply to interpreter development under this skill.

Load the [project reference](./references/project-map.md) for source ownership and validation pointers. Whole-project responsibility does not require surveying the whole repository on every task. Source pointers and known limitations must be checked against current code.

## Working Procedure

1. Identify the observable behavior and target editions. Start at a failing command, primitive, test, entry point, or owning source; follow forwarding code to the actual decision point.
2. Read nearby implementation and tests. Form a falsifiable hypothesis and choose the cheapest discriminating check before editing. Use `plm` for the conceptual model, then verify against source.
3. Choose the smallest coherent layer: Logo library, shared C interpreter, platform adapter, JavaScript runtime, HTML/CSS, or build system. Do not hide shared semantic defects in web-only wrappers.
4. State relevant invariants: Logo semantics, NODE lifetime, evaluator state, platform guards, suspend/resume boundaries, IO ordering, or persistence readiness. Preserve backward compatibility unless a breaking change is explicitly approved.
5. Implement a focused change and immediately run its narrow regression test or executable validation. Repair that slice before broadening scope. Scale coverage with risk, especially for GC, evaluator, IO, and cross-platform contracts.
6. Rebuild affected artifacts and validate in the actual target runtime. Browser-only behavior needs browser checks; Node success does not establish DOM, IndexedDB, rendering, or event-loop correctness.
7. Update affected docs and report exactly what passed, failed, or could not be checked. Preserve unrelated work; do not commit, regenerate unrelated outputs, or overwrite build configurations casually.

## Choosing An Implementation Language

New Logo words can be implemented in **C, Logo, JavaScript, or any mixture of the three**. Choose based on semantics, runtime cost, platform boundaries, and existing APIs rather than imposing one language on every extension.

- The maintainer likes extending Logo in Logo and often tries pure Logo or a Logo/`runjs` hybrid first for non-performance-critical features. These wrappers have high runtime overhead: do not use the Logo/`runjs` extension approach in tight loops or performance-critical paths. Move repeated work into an appropriate C or JS implementation and minimize bridge crossings.
- Otherwise, consider a pure JS implementation first when it can use established web/runtime APIs without exposing substantial interpreter internals. Keep NODE representation, evaluator registers, and GC machinery behind the C boundary rather than exporting Logo's guts just to implement a browser feature.
- A pure C implementation may already be trivial or naturally reuse existing interpreter machinery. Use it directly when that is the simplest correct solution; do not reinvent it in Logo or JS merely to follow a preference. Most web features in practice become C/JS hybrids.
- Calling JS from C through Emscripten is straightforward and often the appropriate direction: keep argument validation, Logo values, and interpreter state in C, and delegate browser/platform operations to JS. Inspect nearby `EM_JS`, `EM_ASM`, `EM_ASYNC_JS`, or JS-library calls for the established pattern and suspend/resume contract.
- Put larger JS implementations in `liblogo.js` when they belong to Emscripten's library/import machinery, or, more commonly, in `pre.js` under `Module` when they belong to application/runtime services. Prefer a small C-to-JS adapter calling an existing `Module` API over embedding a large JS body in C. Preserve browser/Node guards, library dependencies, data conversion, errors, and cleanup.
- Respect existing patterns and APIs where practical, including wx-port APIs already adapted for the web. A historically wx-exclusive feature can gain a web backend using the same public Logo interface and compatible platform contract; inspect the existing wx implementation before inventing a competing web-only API. Keep unsupported differences explicit.
- Evaluate the chosen approach with a focused behavior test and, for hot paths, representative measurements of execution and bridge overhead. A convenient prototype need not dictate the final implementation language.

## Language And Mini-Scheme Foundations

- Be fluent in UCB Logo rather than generic Logo pseudocode: words, lists, arrays, quotation/colon syntax, escaping and parity, case handling, procedure arity, templates, macros, dynamic scope, and commands versus operations. Verify primitives versus library procedures in this dialect.
- Read repository `plm`, the Program Logic Manual, particularly its SICP metacircular/explicit-control evaluator discussion, continuations, tail calls, shallow binding, data structures, and collector. It is slightly out of date in places. Use it as architectural guidance, not an exact specification of current functions, layouts, roots, or platform behavior; reconcile discrepancies with source and tests.
- Understand the user's "mini-Scheme" characterization as the Lisp-family substrate and explicit-control evaluator machinery underlying Logo. Do not assume a separate Scheme runtime or import Scheme's lexical scope and syntax into Logo.
- Distinguish reader/tokenization, runparsing, treeification, and evaluation. Arity changes and procedure redefinition can affect cached procedure representations; test relevant invalidation behavior.
- Follow evaluator labels, registers, saved state, continuations, and tail-call transitions before changing control flow. Preserve dynamic binding/unbinding, macro caller context, `output`/`stop`/`throw`/`catch`, command/operation status, and useful error attribution.
- Logo evaluation frames, the native C stack, GC marking work stack, and Asyncify save area serve different purposes. Do not equate their limits or assume increasing one fixes overflow in another.
- For primitives, verify registration, aliases, arity, dispatch type, prototypes, platform guards, errors, NODE ownership, tests, and docs. Follow established conventions rather than inventing a primitive framework.

## Garbage Collection And Memory Management

- Treat `logo.h`, `mem.c`, `logodata.c`, `intern.c`, evaluator state, and entry-point stack setup as a coupled memory contract. Inspect actual NODE kinds, sentinels, strings, arrays, segments, free lists, and generation metadata before manipulating them.
- The current collector is generational with conservative C-stack scanning and explicit roots. Trace marking, promotion, sweeping, old-to-young tracking, and reserve-tank/allocation failure handling in current source. Do not assume historical reference-counting advice describes current NODE lifetime.
- Use mutation helpers such as `setcar`, `setcdr`, and `setobject` where appropriate to preserve write barriers/old-to-young tracking. Audit direct field writes against the owning allocation or GC path; do not mechanically replace initialization or collector internals.
- Account for every live NODE across allocation, GC, evaluator saves, errors, yielding, and callbacks. C locals, globals, evaluator registers, interned symbols, arrays, and temporary aggregates can have different rooting requirements. A JavaScript-held Wasm address does not keep a NODE alive.
- Investigate stack roots across optimization, Wasm, Asyncify suspension, `setjmp`/`longjmp`, and sanitizers. Verify Emscripten-specific scanning and `set_bottom_stack`; do not assume suspended stack contents are scanned automatically.
- Separate managed NODE lifetime from C heap buffers and JS objects. Check allocation sizes/overflow, NUL termination, byte versus character counts, ownership transfer, and exit paths. Preserve EOF as an integer distinct from valid input bytes; check signedness at IO boundaries.
- Wasm memory growth can invalidate cached JS views. Reacquire heap views after growth/yielding and use current Emscripten access/conversion helpers; avoid hard-coded NODE layouts and pointer widths in JS.
- Add forced/repeated GC tests for live values, inter-generation mutation, arrays/strings, and allocation-heavy/deep evaluation as relevant. Test optimized builds too when root visibility or undefined behavior is implicated.
- Use `gdb.rc`, `address-sanitizer.txt`, and current `logoGC`/C debug exports as investigation aids, not stable application APIs. Inspect live structures only at safe runtime states; addresses and snapshots can become stale after collection or reuse.

## Web Runtime And Asyncify

- **Asyncify is required for garbage-collector correctness, not merely asynchronous IO.** It spills Wasm registers/VM locals to the heap so the collector can discover live NODE pointers. Trace `mark_wasm_stack`, `emscripten_scan_stack`, and `emscripten_scan_registers` in `mem.c`. JSPI cannot be used with the current collector; do not add JSPI flags or treat it as an interchangeable runtime option.
- Future TODO: investigate migration to `ASYNCIFY=2`. The maintainer's last attempt did not work. Do not enable it by default or infer support from existing stack-switching comments; first establish that register spilling/root scanning, GC under optimization, yielding, and IO still work with the installed toolchain.
- Trace startup through the generated loader, `Module.preRun`, filesystem/environment setup, `Module.onRuntimeInitialized`, and `webmain.c`. Distinguish loader readiness, persistent-storage readiness, Logo initialization, input/prompt readiness, and shutdown.
- Check `EM_ASYNC_JS`, `Asyncify.handleSleep`, yielding helpers, `LOGO_GIL`, evaluator/GC flags, input events, and animation scheduling together. Follow each callback's safe-entry protocol; `Asyncify.state === 0` alone does not permit evaluator reentry.
- Preserve single-interpreter coordination. Queue work through existing input paths where appropriate; do not call exported evaluation functions while an invocation is active, unwinding, rewinding, collecting, or saving evaluator state.
- **The Asyncify-based Logo C runtime is non-reentrant.** JS may enter it only after the interpreter invocation has returned and its C stack is empty. Yielding or suspending an invocation does not satisfy this condition: a saved stack still represents active interpreter work. C-to-JS calls are supported, but JS called from C must not synchronously call back into the Logo C runtime; queue that work until a permitted entry point instead. Retain the existing lock/state checks as well as this stack-lifetime requirement.
- **Use the UI event queue to signal the interpreter while it is running.** `Module.uievents` and `Module.uieventypes` in `pre.js`, the `peek_uievent`/`get_uievent` bridge in `liblogo.js`, and interpreter-side dispatch implement `buttonact` and other mouse, keyboard, animation, pause/stop behavior. Enqueuing a JS event is not C reentry: C consumes it at its own event-processing points. This is the safest existing JS-to-interpreter signaling mechanism during execution; extend the established event contract instead of directly invoking C handlers.
- Follow existing event producers: enqueue the appropriate payload, then use guarded `Module.run_evaluator` when scheduling or waking is needed. When the interpreter is idle and has returned, this wrapper can start it; when active/suspended, it can wake a wait without starting a nested invocation. Inspect queue order, capacity, priority/coalescing, event layout, and dispatch before adding events. Keep UI-event signaling distinct from `runLogo`/stdin source queuing, and test both active and idle delivery where relevant.
- Balance suspension/resumption and lock/state transitions on success, failure, cancellation, pause/stop, and shutdown. Ensure waits can be woken and readiness-check/listener ordering cannot lose events.
- Preserve responsiveness without tight polling or nested runtime entry. Check teardown of listeners, timers, animation frames, and pending promises after errors and `bye`.
- Verify exports/runtime methods, JS-library dependencies, Asyncify instrumentation, stack sizes, and link settings for browser and Node targets. Check assumptions against the installed Emscripten version.

## IO, Filesystems, And JavaScript

- Trace browser input through controls/`runLogo`, character buffering, readiness events, `em_getc`/`sysGetC`, reader, and evaluator. Test command input separately from character/line reads, EOF, unget, multiline definitions, and file streams.
- Trace output through Logo printing, streams/flushing, terminal adaptation, and DOM rendering. Preserve ordering, partial lines, stream redirection, prompt/cursor state, errors, and explicit HTML-string handling.
- Distinguish browser MEMFS/embedded assets, persistent IDBFS at `/d`, temporary files, and Node `NODERAWFS` host access. Respect working directory, Logo prefixes/library paths, escaping, and platform path semantics.
- `FS.syncfs(true, callback)` populates storage asynchronously; synchronous FS access does not prove persistent data is ready or saved. Check mount/readiness ordering, auto-persistence, concurrent writes, reload behavior, and storage failure paths. localStorage settings and IndexedDB files are separate persistence mechanisms.
- Validate upload/download, browser editing, workspace load/save, and packaged resources as actual workflows. Browser filesystem edits do not update repository files automatically. Avoid loss or overwriting when transferring changes.
- Inspect current `runjs` binding/serialization. Prefer trusted stable source with bound `arg0`, `arg1`, etc. rather than interpolated Logo data. Verify supported inputs, return conversion, promises, exceptions, stream restoration, escaping, and encoding constraints. `runjs` is not a sandbox.
- Guard browser APIs in code shared with Node. Maintain explicit platform differences rather than faking unsupported browser features on wx.

## Frontend And Graphics

- Edit source `shell.html`, `pre.js`, and relevant graphics/interop code, not generated app/loader outputs as permanent fixes. Preserve existing UI/CSS conventions.
- Keep shared turtle semantics in the shared graphics layer and rendering in platform backends. Verify affected coordinates/scaling, heading, pen modes/colors, wrapping/clipping, labels, fills, visibility, and redraw.
- Validate affected terminal, editor, dialogs, settings, focus/history, keyboard/touch/mobile input, resize/themes, accessibility, and error display. Check SVG/DOM graphics and assets in a real browser.
- Distinguish ordinary text from intentional HTML markup. Preserve escaping and safe text APIs for untrusted values; do not bypass HTML-string semantics to satisfy display tests.
- Reuse a suitable server/shared browser. Confirm served artifacts match source and account for service-worker/cache staleness before diagnosing regressions. Do not erase user persistence just to obtain a clean test.

## Build-System Ownership

Improving the build system is an explicit responsibility of this skill. Make builds reproducible, easier to debug, fast to iterate, and maintainable while preserving web and wx builds.

### Requested Web Profiles

The maintainer currently runs `emscript.sh` for clean builds and edits its flags/debug options and service-worker generation as needed. This accelerated earlier compiler-flag iteration; those flags have now settled down. The desired replacement is three selectable builds, not continued script editing:

| Profile | Required Configuration |
| --- | --- |
| `prod` | `-O3`; generate and include the service worker. |
| `dev` | `-g`; no service worker. |
| `debug` | `-g`; AddressSanitizer (ASAN), plus as many compatible runtime checks as possible. |

- These profiles are a requested build-system goal, not a claim that profile commands already exist. Implement them when asked to improve the build; keep the existing workflow usable during migration.
- Do not silently add optimization to `dev`/`debug` or change production semantics. Define compile and link settings consistently. For debug, investigate assertions, stack-overflow checks, Asyncify diagnostics, undefined-behavior checks, and other supported instrumentation; validate compatibility with the toolchain, Asyncify, custom GC/stack scanning, memory growth, and browser/Node targets.
- "As many checks as possible" means the strongest useful, compatible checked configuration, not every flag blindly enabled. `SAFE_HEAP` and sanitizer combinations may have restrictions; diagnose conflicts and document exclusions and costs. Do not silently disable ASAN to make a debug build pass.
- Production explicitly includes the service worker and dev excludes it. Unless the user specifies otherwise, use no service worker for debug too to prevent stale diagnostic artifacts; state that assumption when implementing profiles.
- Prevent a prior production worker from controlling dev/debug pages or caching their artifacts. Skipping generation alone does not disable an already-registered worker. Use profile-aware registration/output isolation and targeted cleanup that preserves user files/settings.
- Expose profile selection and clean versus incremental builds without repeated source edits. Regenerate Autotools configuration only when inputs/toolchain configuration require it, not on every flag experiment. Keep profile outputs/objects and effective settings distinguishable to prevent cross-profile contamination.

### Build Engineering Rules

- Read `configure.ac`, `Makefile.am`, `emscript.sh`, npm scripts, and relevant CI before changing recipes. Treat generated configuration, Makefiles, loaders/Wasm, and service-worker outputs according to actual repo policy; fix source recipes rather than generated symptoms.
- Prefer explicit prerequisites, accurate dependency tracking, clear options, shared source/object/export definitions, and target-specific settings over duplicated link lists and implicit shell state. Add abstractions only when they remove real complexity.
- Isolate native/wx and Emscripten configurations/objects. Check the active configuration before `make`; do not reuse Wasm objects in native links or overwrite working builds casually. Verify out-of-tree support before relying on it, particularly for root-relative custom web/embed recipes.
- Separate configuration, compilation, linking, asset staging, tests, and packaging where it improves iteration. Preserve required library/help/CSLS files, startup overrides, exports/debug assets, and browser/Node differences.
- Validate clean/incremental builds, dependency-triggered rebuilding, failure exit status, missing-tool diagnostics, and feasible parallel execution. Measure performance claims.
- Remove obsolete flags/comments in the relevant recipe without silently changing Asyncify. Treat optimization, sanitizers, debug symbols, memory, and exception settings as deliberate validated configurations.
- Check Workbox/PWA cache generation, registration, manifest/assets, deployment paths, and artifact consistency. Never mix old loaders and new Wasm in a cache.
- For shared interpreter/build changes, compile/link wx when available and smoke-test startup, input, basic evaluation, and affected behavior. Node/headless success is not wx GUI validation. Report missing wx dependencies/display honestly.

## Development Cadence And Platform Coverage

- **Default iteration is web-first and Chrome-only.** During ordinary development, rebuild the affected web artifacts and run focused tests in Chrome. Do not require Firefox, Safari, or wx checks after every edit or interrupt rapid iteration with the entire platform matrix.
- **Expand validation when ready to commit.** Run the relevant broader tests on Linux wx and web Chrome, Firefox, and Safari, including mobile versions where applicable; debug platform differences as necessary. Do not commit unless explicitly requested. Earlier targeted platform or sanitizer checks are appropriate when a specific failure or shared C/GC/build change makes them discriminating, not as a blanket per-edit requirement.
- The maintainer has Linux wx and an iPad for real Safari testing, but no local macOS or Windows. Use the iPad for affected Safari, touch, keyboard, storage, and layout workflows. Report unavailable targets rather than claiming compatibility from Chrome alone.

| Target | Broader Validation Goal |
| --- | --- |
| wx on Linux | Native build, relevant Logo regressions, GUI startup/input and affected behavior. |
| wx on macOS and Windows | Prefer native builds and relevant tests through CI; GUI checks where runner support permits. |
| Desktop web | Chrome, Firefox, and actual Safari; obtain desktop Safari coverage through macOS CI or a hosted service. |
| Mobile web | Chrome and Firefox mobile coverage on relevant platforms, plus real iOS/iPadOS Safari; local Safari checks use the iPad. |

- Aim to automate this matrix on GitHub Actions; use another CI or hosted browser/device service where necessary. This is requested coverage, not a claim that the jobs already exist. Prove runner, driver, display, and device capabilities before promising automation; get approval for paid services or infrastructure provisioning.
- Prefer existing harnesses and reusable focused tests. Separate native-platform and web-browser/build-profile jobs, keep relevant checks fast, and use scheduled or release coverage for expensive device matrices where appropriate. Include prod behavior/service-worker tests and compatible debug/ASAN checks without contaminating dev sessions.
- Distinguish native compilation from wx GUI execution, headless tests from event-loop coverage, Chromium from branded Chrome, and automated WebKit from shipping Safari. Responsive/device emulation is useful but does not establish real mobile keyboard, storage, or lifecycle behavior. iPad Safari does not establish desktop Safari or iPhone coverage.
- Record actual browser/engine, OS/device, profile, and versions. Do not infer mobile engine diversity from browser branding. Report required, optional, manual, skipped, and unavailable checks explicitly.
- Synchronize CI on readiness and observable results, propagate test failures through exit status, and retain compiler/sanitizer output, browser console errors, test reports, and useful screenshots as artifacts. Keep service credentials in CI secrets. When automation is unavailable, retain a reproducible manual workflow and make the coverage gap visible.

## Upstream Integration Roadmap

The eventual integration target is **`jrincayc/ucblogo-code`, base branch `master`**, not the maintainer's fork's `master`. Verify remote URLs and refreshed branch refs when integration is actually requested; local remote names are not universal.

The maintainer reports that wx is currently broken on the development branch. Preserve wx compatibility as a goal, but do not assume a passing native baseline. Upcoming work is:

1. Rebase the development changes onto upstream `master`, protecting local work and resolving conflicts without dropping intentional Web Edition changes.
2. Reproduce and repair the branch's Linux wx build with native, isolated build state. Verify compile/link, startup/input, relevant Logo regressions, and affected GUI behavior while retaining web/Chrome correctness; pursue macOS/Windows checks through available CI.
3. Clean up integration-related code, obsolete configuration, tests, docs, and artifact handling. Review the diff against upstream `master`, run the relevant broader platform checks, and prepare a PR to the upstream repository with explicit validation results and coverage gaps.

This roadmap is context, not standing permission to rebase, commit, push, or create a PR during skill editing or unrelated development. Act on the task explicitly requested; implementation may be used to verify skill guidance when agreed, without automatically expanding into the whole roadmap. Before rewriting published history, agree on branch strategy and protection of uncommitted work. Never silently stash, discard changes, or force-push. Update known branch status and completed roadmap items as the work progresses.

## Tests And Documentation

- Use Logo regressions for language/primitives/files/GC, Node Wasm tests for non-DOM runtime, browser acceptance for interop/events/storage/UI, and wx checks for native compatibility.
- Start narrow and inspect the harness: the programming skill notes incomplete failure aggregation in existing Logo tests. Review individual `Ok`, `Failed`, and `Error` results, not only process exit or completion text. OOP is currently broken/omitted in web builds; skipped tests are not passing coverage.
- Exercise async lifecycle failures and reload/persistence where relevant. Synchronize on real readiness/output, not arbitrary sleeps. Distinguish source regressions from stale artifacts/state.
- Avoid broad npm scripts that rebuild unnecessarily when a built-artifact check suffices; conversely, rebuild C/packaged-source changes before testing. Check test/server/browser prerequisites.
- Maintain `webedition.md`, help, `usermanual`, `docs/ucblogo.texi`, build docs, tests, and examples as appropriate. `makehelp` regenerates help from `usermanual`; check regeneration before editing help that it would overwrite. Resolve duplicate documentation deliberately.
- Update materially misleading `plm` explanations when changing architectural contracts, preserving useful historical context. Document semantics, compatibility, prerequisites, and known limits rather than speculative promises.