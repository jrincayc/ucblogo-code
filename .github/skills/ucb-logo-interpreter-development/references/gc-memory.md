# Garbage Collection And Memory Management

- Treat `logo.h`, `mem.c`, `logodata.c`, `intern.c`, evaluator state, and entry-point stack setup as a coupled memory contract. Inspect actual NODE kinds, sentinels, strings, arrays, segments, free lists, and generation metadata before manipulating them.
- The current collector is generational with conservative C-stack scanning and explicit roots. Trace marking, promotion, sweeping, old-to-young tracking, and reserve-tank/allocation failure handling in current source. Do not assume historical reference-counting advice describes current NODE lifetime.
- Use mutation helpers such as `setcar`, `setcdr`, and `setobject` where appropriate to preserve write barriers/old-to-young tracking. Audit direct field writes against the owning allocation or GC path; do not mechanically replace initialization or collector internals.
- Account for every live NODE across allocation, GC, evaluator saves, errors, yielding, and callbacks. C locals, globals, evaluator registers, interned symbols, arrays, and temporary aggregates can have different rooting requirements. A JavaScript-held Wasm address does not keep a NODE alive.
- Investigate stack roots across optimization, Wasm, Asyncify suspension, `setjmp`/`longjmp`, and sanitizers. Verify Emscripten-specific scanning and `set_bottom_stack`; do not assume suspended stack contents are scanned automatically.
- Separate managed NODE lifetime from C heap buffers and JS objects. Check allocation sizes/overflow, NUL termination, byte versus character counts, ownership transfer, and exit paths. Preserve EOF as an integer distinct from valid input bytes; check signedness at IO boundaries.
- Wasm memory growth can invalidate cached JS views. Reacquire heap views after growth/yielding and use current Emscripten access/conversion helpers; avoid hard-coded NODE layouts and pointer widths in JS.
- Add forced/repeated GC tests for live values, inter-generation mutation, arrays/strings, and allocation-heavy/deep evaluation as relevant. Test optimized builds too when root visibility or undefined behavior is implicated.
- Use `gdb.rc`, `address-sanitizer.txt`, and current `logoGC`/C debug exports as investigation aids, not stable application APIs. Inspect live structures only at safe runtime states; addresses and snapshots can become stale after collection or reuse.

## Lessons Learned (GC + Wasm, 2026-11)

- For Whippet integration planning, distinguish parallel collection from concurrent collection. Current Whippet guidance and implementation focus on stop-the-world tracing with optional parallel workers; do not assume mutator-concurrent marking is available.
- If roots remain conservative (current Logo model), `pcc` is a poor first target because it requires precise roots. `mmc` with conservative roots is the viable first candidate if evaluating Whippet.
- Whippet's parallel tracer currently uses pthread APIs directly. In browser Wasm, this typically means Emscripten pthread builds (which run on Web Workers and require shared-memory deployment constraints such as COOP/COEP).
- There is no known drop-in OSS collector currently wired for Emscripten Wasm Workers as a direct replacement path for Logo. A Wasm-Workers-native parallel collector path likely implies backend-porting work rather than collector swapping.
- For animation frame pacing, a deadline-aware incremental collector is plausible in the existing GC architecture because marking already uses an explicit work stack. Treat this as cooperative incremental GC, not hard real-time GC.
- Incremental GC requires write-barrier correctness under mutator interleaving. Audit all pointer stores, not only `setcar`/`setcdr`/`setobject`, and preserve correctness for newly allocated nodes during an in-progress cycle.
- Deadline budgeting is soft unless all expensive units are sliceable. Root scanning and large object/array scans can dominate a frame budget if not made resumable.
- For frame-sensitive workloads, start GC before free-list exhaustion and run bounded GC slices at safe scheduler points (for web animation, this generally means frame/yield boundaries), with clear fallback behavior when allocation outruns collection.

## Lessons Learned: O1 + Asyncify + GC Debugging

- Do not treat `-O1` failures as evidence that optimization must be disabled. First assume a rooting/lifetime bug and prove it with instrumentation in an optimized build.
- When a web/Node test command is the reproducer, keep test parameters fixed until a concrete root-cause direction is established. Moving test inputs too early can hide temporal GC bugs.
- Guard scripted pipelines against stale binaries: run tests only when the build step succeeds (for example, by chaining build and test with `&&`).
- For Emscripten Node stack-switching runs, prefer Wasm exceptions over legacy JS exception wrappers when debugging suspend-related failures; verify generated output no longer relies on `invoke_*` wrappers before concluding the suspend path is fixed.
- For GC lifetime bugs, combine ASAN/UBSAN with stable serialized NODE IDs and targeted free/sweep probes. A deterministic ID lifecycle is usually more actionable than raw pointer logs.
- In parser/workspace construction paths, avoid nested allocation patterns that allocate a wrapper and immediately evaluate another allocator inside the same expression. Preallocate wrapper cells, then fill with `setcar`/`setcdr` to keep temporary values visibly rooted across collections.
- Treat tiny segment-size stress configurations as a diagnostic mode only. They are useful to force GC timing bugs but should not be mistaken for production behavior.
