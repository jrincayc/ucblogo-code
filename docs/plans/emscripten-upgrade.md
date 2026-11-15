# Project Plan: Upgrade To The Latest Emscripten

Status: Proposed; implementation has not started. Risk analysis and test
gaps are recorded below and feed Milestones 1 and 3.
Date: 2026-11-14

## Objective

Upgrade the Web Edition build and release workflows from Emscripten 4.0.5 to
the latest stable release, currently 7.0.0, with a reproducible exact version
pin and verified browser and Node.js outputs. Preserve the supported native and
wx builds, current Asyncify behavior, and Logo garbage-collector root scanning.

The target version is based on the [official Emscripten releases](https://github.com/emscripten-core/emscripten/releases/latest)
checked on 2026-11-14. Recheck that page immediately before implementation; if a
new stable release has appeared, update the target and review every intervening
release note.

## Baseline And Constraints

- [ci.yaml](../../.github/workflows/ci.yaml) and
  [deploy-web.yaml](../../.github/workflows/deploy-web.yaml) both pin the
  `mymindstorm/setup-emsdk@v14` action to Emscripten 4.0.5. Keep build and
  release toolchains identical.
- [webedition.md](../../webedition.md#L717) currently states Emscripten >= 4.5,
  which conflicts with the CI pin. Resolve the documented minimum and tested
  version policy as part of this upgrade; do not leave contradictory claims.
- [configure.ac](../../configure.ac#L281) enables Asyncify and a 20 MB Asyncify
  stack. [mem.c](../../mem.c) scans the Wasm stack/register roots for GC.
  Asyncify is a correctness requirement here, not just an async-I/O option.
- [Makefile.am](../../Makefile.am) produces browser and Node.js WebAssembly
  builds with different environment and filesystem settings. Validate both.
  Both link with `em++` and `-sSTRICT_JS`; the Web sources are C only (the
  `.cpp` files are wx-only), compiled with `-std=gnu23`. The compiler-frontend
  jump (Clang 20 to 23) is therefore a larger source risk than libc++.
- [pre.js](../../pre.js), [shell.html](../../shell.html), and
  [liblogo.js](../../liblogo.js) depend on Emscripten internals: `Asyncify.state`,
  `FS`, `IDBFS`, `UTF8ArrayToString`, `Module._*` exports, `noExitRuntime`,
  `Module.preRun`, `window.onerror`, and an `unhandledrejection` handler. Treat
  these as the compatibility surface to audit.
- Do not combine this upgrade with JSPI, `ASYNCIFY=2`, broad compiler-warning
  cleanup, or unrelated runtime refactoring. Do not silently disable sanitizer
  checks or alter runtime/stack settings to make the new compiler pass.
- Keep native CI and the wxWidgets build path unchanged. Generated loaders,
  Wasm binaries, and service-worker output are build artifacts, not hand-edited
  upgrade targets.

## Implementation Milestones

### 1. Confirm The SDK And Compatibility Surface

- [ ] Recheck the latest stable Emscripten release and its official release
  notes. Record the selected version, release date, and relevant SDK/LLVM/Clang
  component versions in the implementation change.
- [ ] Review all release notes from 4.0.5 through the target for breaking
  changes affecting C/C++, libc++, compiler flags, Asyncify, exception handling,
  JavaScript glue, Node.js output, generated Wasm features, and browser support.
- [ ] Specifically check 7.0.0's LLVM 23.1.3 changes. libc++ transitive-include
  removal mostly affects C++ and the Web build is C only, so the main exposure
  is new Clang diagnostics and C23 (`-std=gnu23`) semantics across Clang
  21, 22, and 23. Build with the existing flags first; do not use
  `-D_LIBCPP_KEEP_TRANSITIVE_INCLUDES_LLVM23` unless a C++ file in the Web
  build is demonstrated to need it.
- [ ] Decide the browser floor explicitly. 7.0.0 raises defaults to Chrome 93,
  Firefox 92, and Safari 15.4; confirm the maintainer's iPad iPadOS version
  meets Safari 15.4 before accepting it, and get maintainer sign-off on the
  user-visible floor.
- [ ] Audit the repository's link flags and JS against the changelog (see the
  risk table): `$addOnExit`, `EXPORTED_RUNTIME_METHODS`, `NODERAWFS`,
  `-lidbfs.js`, `STRICT_JS`, `SEPARATE_DWARF_URL`, `Asyncify.*`, and
  `UTF8ArrayToString` used from `--pre-js`.
- [ ] If 7.0.0 fails, bisect via the intermediate majors (5.0.x, 6.0.x) rather
  than debugging the whole jump at once.
- [ ] Verify the GitHub Actions setup action supports the chosen SDK. Evaluate
  moving from the old `mymindstorm/setup-emsdk` reference to the maintained
  `emscripten-core/setup-emsdk` action; pin the action by commit SHA (or
  record why a tag is acceptable), pass an exact Emscripten version, and
  confirm SDK caching so CI time does not regress. Do not use an unpinned
  `latest` in CI or release builds.
- [ ] Record the current baseline build/test results and compiler diagnostics
  before changing the SDK. Use an isolated build tree or confirm the active
  generated Makefile can safely clean the current target; do not overwrite a
  native/wx build configuration or the user's local work.
- [ ] Add the baseline-detectable tests from
  [Test Coverage Improvements](#test-coverage-improvements) (GC/Asyncify
  stress, file modes and persistence, startup failure reporting, link-surface
  and toolchain-identity checks) and confirm they pass on 4.0.5 first, so a
  later failure is attributable to the SDK and not to a new test.
- [ ] On the 4.0.5 baseline, record file modes produced by `save`/`openwrite`
  and IDBFS so the 5.0.7 umask change can be compared directly.

Exit criterion: one exact SDK version is selected, its breaking changes and
browser floor are understood, the new tests pass on 4.0.5, and a fresh baseline
is attributable to the current source and 4.0.5 toolchain.

### 2. Upgrade The Web Build And Documentation

- [ ] Update the SDK setup and exact version in both CI and deployment
  workflows. Log `emcc --version`, `emcc -v`, and `node --version` so failures
  include reproducible toolchain identity. Keep the tooling Node policy aligned
  with current project CI; do not introduce older Node matrix entries as part
  of this task.
- [ ] Build browser and Node outputs from a clean configuration with the normal
  production flags. Keep the existing `-sASYNCIFY` and stack setting unless a
  separate, evidence-backed change is approved.
- [ ] Repair only incompatibilities demonstrated by the new SDK. Prefer source
  fixes and explicit includes over compatibility workarounds that suppress
  diagnostics. Preserve the C/browser/Node split and existing JS-library
  exports.
- [ ] Update [webedition.md](../../webedition.md) with the verified minimum and
  tested SDK version, setup instructions, and any changed browser requirements.
  Remove or correct references that imply an unsupported compiler baseline,
  including the "Emscripten >= 4.5" line and the deploy description stating
  the workflow pins "the same version as `ci.yaml`".
- [ ] Keep the output artifact set coherent: browser HTML/JS/Wasm, Node loader
  and Wasm, required runtime/data files, and production service-worker assets
  must all come from the same successful build.

Exit criterion: a clean Web Edition build succeeds for both browser and Node
targets, with the selected compiler version and expected Asyncify configuration
visible in build logs.

### 3. Validate Runtime And Release Behavior

- [ ] Run `npm test` from the clean upgraded build. This covers the Web build,
  `make check`, source checks, and the functional browser suite.
- [ ] Run `npm run test:logo:node` against the generated Node artifact and
  `npm run test:runner:browser`; run `npm run test:lighthouse` as the existing
  CI/release workflow does. Grade actual Logo test results, not only process
  completion.
- [ ] Exercise optimized production behavior in browser and Node, including
  evaluator yield/resume, asynchronous input/output, file access, and garbage
  collection under allocation pressure. Confirm stack/register root scanning
  still retains live Logo values across yields and collections.
- [ ] Build the compatible debug/ASan profile when supported by the new SDK;
  run focused GC and lifecycle checks. Report an actual incompatibility as a
  blocker rather than silently dropping the check. Keep production optimization
  coverage even if debug checks pass.
- [ ] Verify generated browser and Node glue has the expected imports, exports,
  runtime methods, Asyncify instrumentation, and memory-growth behavior. Check
  both outputs for accidental stale files or artifacts from the old compiler.
- [ ] Run the new tests from Milestone 1 against the upgraded build, including
  file-mode comparison with the 4.0.5 baseline, startup-failure reporting
  (`onerror` and `unhandledrejection`), and the link-surface check.
- [ ] Run the full fixtures in desktop Chrome, then Firefox and emulated mobile
  Chrome, then the maintainer's iPad Safari pass last. Report emulation as
  emulation and desktop Safari as an uncovered gap.
- [ ] Exercise deployment staging and its version/artifact verification without
  publishing. Confirm the deploy workflow uses the same pinned SDK as CI.
- [ ] Confirm native/wx CI remains unchanged and green on the same revision;
  distinguish native coverage from browser and Node coverage.

Exit criterion: all required browser, Node, GC/lifecycle, and native checks pass
on artifacts built by the exact target SDK, and CI logs identify that toolchain.
Any unavailable or incompatible debug/browser target is documented and accepted
explicitly rather than counted as passing.

### 4. Roll Out And Keep The Pin Fresh

- [ ] Document a rollback: revert the SDK pin in both workflows together, and
  keep the last release built with 4.0.5 deployable until the new toolchain
  has shipped one release.
- [ ] Land the SDK and source changes together so CI and release builds cannot
  diverge. Do not publish a release until the upgraded deploy workflow has
  passed its build, tests, and staging verification.
- [ ] After successful CI, confirm the deploy workflow can build and stage the
  same revision. Publishing remains governed by the existing release-tag flow.
- [ ] Add Emscripten release review to periodic toolchain maintenance. Every
  future pin change should update both workflows and documentation together,
  inspect intervening release notes, and run the same focused validation.

## Risk And Breaking-Change Analysis

Based on the Emscripten ChangeLog from 4.0.5 through 7.0.0 (7.0.1 in
development) checked 2026-11-14 against this repository's flags and JS. Items
are hypotheses to confirm during Milestone 1, not verified failures.

Tool versions: 4.0.x uses LLVM 20, 5.0.0 LLVM 21, 6.0.x LLVM 22, and 7.0.0
LLVM 23.1.3.

| Risk | Source release | Exposure here | Severity |
| --- | --- | --- | --- |
| Asyncify instrumentation or register-spill behavior changes with the newer Binaryen/LLVM; `emscripten_scan_stack` and `emscripten_scan_registers` semantics | all | [mem.c](../../mem.c) GC roots depend on both | High |
| Newer Clang diagnostics and C23 semantics under `-std=gnu23` | 5.0.0, 6.0.x, 7.0.0 | all C sources; `-Wno-comment`, `-Wno-typedef-redefinition` already needed | High |
| New files honor a global umask that defaults to 0o222 | 5.0.7 | `save`, `openwrite`, editor writes, IDBFS persistence, `NODERAWFS` host files | High; verify file modes and writability |
| Startup uses more `async`/`await`; startup errors surface as unhandled rejections rather than `onerror` | 6.0.1 | [shell.html](../../shell.html) has `onerror` and `unhandledrejection` handlers; confirm both still report startup failures | Medium |
| Minimum browsers rise to Chrome 93, Firefox 92, Safari 15.4 | 6.0.0, 6.0.7, 7.0.0 | user-visible support floor; iPad iPadOS version | Medium |
| `INCOMING_MODULE_JS_API` trimmed (for example `wasmBinary`, `INITIAL_MEMORY`, `wasmMemory`, `logReadFiles`, `preloadPlugins`) | 5.0.1, 6.0.2 | confirm nothing in `shell.html`, `pre.js`, or `web-config.js` passes these | Low to medium |
| Runtime internals used from `--pre-js` and `shell.html` (`Asyncify.state`, `UTF8ArrayToString`, `IDBFS`, `FS`, `noExitRuntime`) must still be linked in | 3.1.x to 7.0.0 | [pre.js](../../pre.js), [shell.html](../../shell.html), [testcafe-helpers.js](../../tests/acceptance/testcafe-helpers.js) | Medium |
| Wasm exports are not callable before instantiation; `Module._*` captured early break | 4.0.10 | [liblogo.js](../../liblogo.js), `pre.js` `Module._logo_gc_debug_*`, tests using `Module._LOGO_GIL` | Medium |
| `FS.write` accepts only typed arrays; `FS.findObject`, `FS.filesystems`, `FS.allocate` removed; `FS.analyzePath` deprecated | 5.0.3, 6.0.11, 6.0.13 | current code uses `FS.writeFile`, `readFile`, `readdir`, `mount`, `syncfs`; commented code references `FS.filesystems` | Low |
| Pre-run and post-run callbacks run in registration order | 4.0.7 | `Module.preRun` is a single function; review any ordering assumptions | Low |
| `addRunDependency`/`removeRunDependency` and HEAP views are no longer exported by default | 4.0.7, 4.0.14 | `weblib.c` uses `HEAPU8` inside JS library code; confirm any use from `pre.js` | Low |
| Node output requires Node 18.3 or later; CI uses the latest Node | 5.0.6 | acceptable | Low |
| libc++ transitive include removal | 7.0.0 | C only, so not expected | Low |
| Pipe capacity 64 KiB; `FAKE_DYLIBS` off; `DEFAULT_TO_CXX` off; Closure needs Java 21; `NODE_CODE_CACHING` removed | 6.0.x to 7.0.0 | not used by current flags (`em++` links C objects) | Low |

Opportunities, not part of this upgrade: `IDBFS.onAutoPersistStateChanged`
(6.0.0) could replace polling of persistence readiness; `JSPI` is no longer
experimental but stays out of scope.

## Test Coverage Improvements

Existing coverage: `UnitTests-*.lg` suites via `make check` and Node, TestCafe
browser fixtures, Lighthouse, runner and staging tests. The upgrade exposes
gaps; add these before or with the SDK change so a regression is detectable.

- **GC and Asyncify stress.** Only `REPEAT 10 [GC]` in
  [UnitTests-Constructors.lg](../../tests/UnitTests-Constructors.lg) and
  [UnitTests-MemMgr.lg](../../tests/UnitTests-MemMgr.lg) cover collection. Add a
  test that holds live lists, arrays, and words in local variables and
  evaluator frames across forced `GC`, a yielding primitive (`wait`, input
  read), and allocation pressure, then verifies contents. Run it at the
  production optimization level in both Node and browser.
- **Toolchain identity test.** A source test that asserts the built artifacts
  record the expected `emcc` version (producers section is already emitted by
  `-sEMIT_PRODUCERS_SECTION`) and that CI and deploy workflows pin the same SDK
  version and action reference, so the two cannot drift.
- **Link-surface test.** A source test that checks the generated loader
  defines every runtime symbol `pre.js` and `shell.html` use
  (`UTF8ArrayToString`, `Asyncify`, `IDBFS`, `FS.*`) and that the exports list
  in [Makefile.am](../../Makefile.am) is present in the Wasm, for both targets.
- **File creation modes.** Extend [UnitTests-Files.lg](../../tests/UnitTests-Files.lg)
  to create, overwrite, rename, and reopen files and to confirm they remain
  readable and writable. Add a browser check that a file saved to `/d`
  persists across a reload and is writable afterward; add the same for
  `NODERAWFS`.
- **Startup failure reporting.** Add a TestCafe case that forces a startup
  failure (for example a failing Wasm fetch) and asserts the error UI appears,
  covering both `onerror` and `unhandledrejection` paths, since startup error
  delivery changed in 6.0.1.
- **Persistence readiness.** Add a test around IDBFS mount, sync from storage,
  and auto-persist after the upgrade, including the Safari-like path with
  storage unavailable.
- **Browser matrix.** Keep desktop Chrome as the default gate; add Firefox for
  the full fixtures at upgrade time, run emulated mobile Chrome, and schedule
  the maintainer's iPad Safari pass last. Report emulation as emulation and
  desktop Safari as an uncovered gap.
- **ASan and debug profile.** Add a CI or documented local job that builds with
  `--enable-asan` and runs the Node Logo suite so sanitizer coverage is
  recorded, not only mentioned.
- **Test result grading.** [logo-results.test.js](../../tests/acceptance/logo-results.test.js)
  already grades Logo results; require it to fail on zero parsed tests so a
  silently skipped suite cannot pass.
- **Native and wx.** Keep `tests/native` and the wx smoke tests running
  unchanged on the same revision to prove the Web toolchain change did not
  affect them.

## Risks And References

Primary risks are the Clang frontend and C23 diagnostics, Asyncify and GC
root-scanning behavior, file-creation permission changes, altered
Emscripten-generated runtime glue and startup error delivery, and the raised
minimum browser baseline. A successful compile alone is not evidence that the
Wasm GC roots, browser callbacks, or release artifacts remain correct.

- [Emscripten 7.0.0 release notes](https://github.com/emscripten-core/emscripten/releases/tag/7.0.0)
- [Emscripten releases](https://github.com/emscripten-core/emscripten/releases)
- [setup-emsdk action](https://github.com/emscripten-core/setup-emsdk)
- [Web build ownership and validation guidance](../../.github/skills/ucb-logo-interpreter-development/SKILL.md#build-system-ownership)
- [Web Edition build instructions](../../webedition.md#building)