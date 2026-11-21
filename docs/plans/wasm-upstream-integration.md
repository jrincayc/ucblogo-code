# Plan: Merge the WASM Branch into Upstream `master`

> **Status:** Approved on 2026-11-21. This document is the planning source of truth on the `wasm` branch. It does not authorize implementation, branch operations, pull requests, rebases, pushes, or releases. Implementation remains delayed pending explicit upstream maintainer approval.

## Objective

Integrate the UCB Logo Web Edition developed on `wasm` into `jrincayc/ucblogo-code:master` without asking the maintainer to review the complete 40,000-line branch at once.

Splitting the work into PRs is only one mechanism. The completion criterion is that upstream `master` contains the intentional Web Edition functionality, supporting build and deployment infrastructure, tests, documentation, and the shared changes needed by both legacy and Web targets.

Success requires:

- known-good wx and Unix terminal builds;
- buildable and testable browser and Node WebAssembly targets;
- no intentional `wasm` capability lost or left as an unexplained delta;
- continued `wasm` development reconciled while smaller PRs land;
- final retirement or supersession of PR #268 by the completed reviewable series;
- upstream `master` becoming the canonical integrated target, or an explicitly approved post-integration branch policy.

## Constraints

- Target `jrincayc/ucblogo-code:master`; verify remotes and SHAs before major checkpoints.
- Do not perform legacy backport development on `wasm`.
- Do not rebase, rewrite, force-push, or silently change `wasm`.
- Treat each PR as one review question. Prefer semantic cohesion over a fixed line limit.
- Each PR must build and test from its declared base plus explicitly named predecessor PRs.
- A PR must never depend on unpublished work left only on a private integration branch.
- Fix source recipes and source files, not generated Web artifacts, unless repository policy explicitly requires an artifact update.
- Native validation and integration scope is limited to wx and Unix terminal only.
- Ongoing `wasm` commits are tracked as a delta, not silently absorbed or ignored.

## Phase 0 — Establish an auditable integration baseline

1. **Verify refs and record immutable anchors.** Record the current upstream `master` SHA and a starting `wasm` SHA. Confirm whether PR #268 has moved. All later reconciliation references the starting SHA plus an explicitly refreshed range.

2. **Inventory the complete delta by hunk.** Classify each meaningful change as legacy restoration, shared interpreter/build work, Web runtime, browser UI, bridge/primitives, graphics, filesystem/persistence, tests, CI/CD, packaging/deployment, documentation/help/examples, generated output, deletion, or obsolete/accidental work. Mark each path as existing on `master`, new, deleted, or generated.

3. **Build a dependency graph.** Record compile/link prerequisites, header/ABI contracts, tests, packaging references, and documentation dependencies. Identify technically inseparable hunks and changes that should be independent cleanup or path-move PRs.

4. **Capture baseline evidence.** In separate fresh trees, build and test untouched `master` and the recorded `wasm` snapshot. Record toolchain/browser versions and known failures reproducible on `master`. Do not trust stale executables or reused objects.

5. **Create the ongoing-delta ledger.** Every new `wasm` commit after the anchor is classified as fixing pending work, changing already-landed upstream behavior, adding a future feature, documentation, generated-only work, or intentionally excluded work. Reconcile this ledger at regular checkpoints.

6. **Publish a maintainer-readable tracker.** Once implementation begins, one tracking issue or PR-stack overview should list phase, PR purpose, base/predecessor, status, validation, and intentionally deferred neighboring work.

## Phase 1 — Restore legacy builds off `wasm`, then create small PRs

Break the `wasm` work into short-lived PR branches created from and targeting `master`; never implement, rebase, or commit application work on `wasm`. Phase 1 has one purpose: restore and continuously verify the legacy wx and Unix terminal builds using the smallest backports. Web-specific behavior is deferred unless a shared legacy file must change so native builds remain compatible with later web work.

### Recommended sequencing

1. **Freeze an auditable backport inventory** — in an isolated checkout, refresh local `master` and `wasm` refs, record exact SHAs, and classify `git diff master...wasm` into: master-existing files, wasm-new files, deletions, and generated artifacts. Treat this as evidence and a hunk catalog, never as a bulk cherry-pick source. Reverify the base SHA because `master` may have moved.

2. **Create a private phase-1 integration branch from `master`** — do not open the first PR and do not modify `wasm`. This branch receives legacy restoration work and tests as local commits so the repair can be validated as an integrated whole before review begins.

3. **Backport the minimum native restoration on that private branch** — order by dependency, not by eventual PR boundaries:
   - Native build selection in legacy portions of `Makefile.am` and `configure.ac`; exclude web recipes and Emscripten options.
   - Platform guards and shared declarations in `init.c`, `eval.c`, `main.c`, `logo.h`, and `globals.h`.
   - Native GC/stack contract in `mem.c` and directly coupled callers.
   - Native input/output compatibility in `files.c`, `parse.c`, `print.c`, and `globals.h`.
   - Native terminal backend restoration in existing `term.c`; do not add `webterm.c`.
   - Only the small warning, overflow, or defensive fixes proven necessary for clean legacy builds.

   Each local commit should be focused, but no PR is created yet.

4. **Backport existing-path regressions on the private branch** — add language, file, memory, terminal-output, and compatibility cases to `tests/UnitTests.lg` and applicable `tests/UnitTests-*.lg` as fixes land. Avoid web-runner-specific assumptions and web OOP exclusions unless independently established as native behavior.

5. **Approval gate A — Unix PTY regression support** — request approval to add `tests/native/pty_terminal.py`, absent on `master`. If approved, restore it on the private integration branch and include only the terminal checks needed for known-good status: termios restoration, signals/exit, clear/cursor/margin sequences, redirected streams, and EOF. If denied, maximize existing-file coverage and record the interactive gap.

6. **Approval gate B — wx smoke support** — request separate approval to add `tests/native/wx_smoke.py`, its README, and each PNG under `tests/native/wx-smoke-baseline/`. Approval must identify the exact added paths, especially binaries. Restore approved files on the private branch; do not change wx source merely to satisfy screenshots.

7. **Backport the native CI wiring on the private branch** — update `.github/workflows/ci.yaml` only after the repaired builds and tests are usable. Configure and grade wx with objects, terminal with objects, and terminal without objects; attach PTY and wx smoke checks only where their corresponding new-file approvals were granted. Reject missing or empty reports and individual non-`Ok` results.

8. **Integrated pre-PR gate** — from a fresh checkout of the private integration branch, clean-build and run the complete legacy matrix. Required evidence: explicit target macros, wx and terminal builds, object/default suites, approved PTY checks, and approved wx smoke comparisons. Record `master`-reproducible failures, notably #265, instead of hiding them. Do not open any PR until this branch is known good and the individual changes have been separated for review.

9. **Then create the first PR** — derive a small PR branch from current `master`, transplant the first reviewable semantic block from the validated integration branch, and include exactly the tests/wiring needed for that block. The first PR must remain independently valid: it should build, run, and either introduce or use the necessary legacy regression gate. Repeat only after the previous PR is reviewed/merged or the dependency is intentionally documented.

10. **Split phase 1 before creating the first PR** — the private integration branch proves the combined result, but it is not the first PR. Derive several independently reviewable PR branches from it, ordered by dependency. Suggested boundaries: (A) approved native test harness/CI additions that also pass on current `master`; (B) native build selection and platform guards; (C) native terminal backend restoration; (D) shared native I/O compatibility; (E) GC/stack contract; (F) independent defensive/warning fixes. Each PR must contain one mental model, build from `master` or the explicitly listed predecessor, run its focused regression plus the shared native gate, and have a short explanation of why the neighboring changes were excluded. If two hunks cannot build or pass tests without each other, merge those hunks into one PR rather than creating a broken stack.

11. **Do not use a fixed line-count rule as the primary decision** — use semantic cohesion, review load, generated/binary noise, and dependency count. As a rough review target, prefer hundreds rather than thousands of source lines per PR, keep screenshot/binary additions in their own PR, and split again when a PR touches more than one evaluator subsystem or requires a long cross-file explanation. A mechanically small PR with unrelated concerns is still too large.

12. **Revalidate each sliced PR independently** — after transplanting from the integrated branch, compare the PR source state with the corresponding validated snapshot, clean-build the affected native targets, and run the focused regression plus the common wx/terminal gates. Rebase subsequent PR branches onto current `master` as predecessors land. Never submit a PR that is known only to work when the rest of the private integration branch is present.

## Relevant files

- `.github/workflows/ci.yaml` — native jobs, explicit target selection, result grading, artifact retention; web jobs remain outside phase 1 edits.
- `Makefile.am` and `configure.ac` — native source selection and configuration only; do not backport Web target recipes yet.
- `logo.h` and `globals.h` — shared contracts/prototypes/platform guards.
- `init.c`, `eval.c`, and `main.c` — Emscripten guards, native event/scheduler boundaries, and entry-point compatibility.
- `mem.c` — native stack-root and GC call contract; use `.github/instructions/gc-memory.instructions.md` during implementation.
- `files.c`, `parse.c`, and `print.c` — stdio/wx input and literal native output.
- `term.c` — restore legacy terminal backend; do not add `webterm.c` without a later approved scope change.
- `tests/UnitTests.lg` and `tests/UnitTests-*.lg` — existing Logo regressions that may be extended without adding paths.
- `tests/native/pty_terminal.py` — approval required; absent on `master`.
- `tests/native/wx_smoke.py` and `tests/native/wx-smoke-baseline/**` — approval required for code and every baseline image; absent on `master`.
- `docs/plans/pr-268-description.md` — current native-impact summary and evidence; reuse as a source inventory, not as a claim that one PR is reviewable.
- `docs/plans/native-build-restoration.md` and `docs/plans/wx-smoke-ci.md` — detailed validation history and known baseline issues.

## Verification per PR

1. Use a fresh worktree/checkout based on the current target `master`; never build the backport PR in the active `wasm` configuration.
2. Run `autoreconf --install`, explicitly configure the intended native target, verify the selected macros, then clean build. Never trust pre-existing objects.
3. For shared C changes, run both terminal and wx builds. For GC changes, also run existing memory-manager and allocation-heavy regressions at the native CI optimization level.
4. Grade every `tests/UnitTests.lg` result and reject missing, empty, timed-out, or non-`Ok` reports. Run both object-enabled and default no-object configurations where the PR can affect them.
5. For terminal PRs, run redirected stdout and the approved PTY suite; distinguish an issue reproducible on `master` from a regression.
6. For wx smoke PRs, use the CI environment’s Xvfb baselines and archive screenshots/results; do not replace CI-approved baselines with workstation captures.
7. Keep each PR’s diff coherent and reviewable: aim for one subsystem, a focused rationale, and no more test infrastructure than required. Split before submission if review requires multiple mental models.

## Decisions and scope boundaries

- Phase 2 and later Web Edition work are planned at the capability level. Exact file-level PR slices are determined by the Phase 0 inventory and dependency graph, then refined as predecessors land.
- Do not work directly on, merge into, or cherry-pick a batch onto `wasm`; use `master`-based branches so development on `wasm` can continue independently.
- Phase 1 includes only native restoration and its regression gates. Web-only C, JavaScript, HTML/CSS, PWA, Node/browser CI, deployment, help UI, demos, and generated Web artifacts are excluded.
- A modified legacy path is allowed only when the backported hunk is necessary for native known-good status or is an unavoidable shared compatibility guard. Do not backport whole files when that imports unrelated web functionality.
- HTML-string primitives/types, web event queues, runjs, animation, JSPI/Asyncify, web terminal code, and Emscripten SDK changes are not phase-1 scope.
- New files are not permitted by default. `pty_terminal.py` and wx smoke driver/baselines require explicit maintainer approval before implementation; add an approval checklist to the PR/implementation handoff listing exact paths.

## Further considerations

1. Review target: `jrincayc/ucblogo-code:master`, not a fork mirror or `wasm`. Confirm upstream refs and base SHAs immediately before creating PR branches.
2. The strict “master-existing files only” and “backport wx/PTY tests” requirements conflict because those test harnesses are new. Recommended resolution: approve the two test-only additions explicitly, then keep every other phase-1 PR legacy-path-only.
3. Keep the approved pre-existing upstream Ctrl-C SIGSEGV as a documented baseline exception, not a phase-1 bug fix, unless the maintainer changes scope.

## Phase 2 — Land shared interpreter and build compatibility

Transfer shared changes only after the native baseline is accepted. These PRs must support the second runtime without changing legacy behavior and should exclude the complete frontend, PWA, and deployment layers.

Split by invariant:

1. **Platform boundaries:** target guards, conditional headers, export annotations, and explicit target selection.
2. **GC and lifetime:** NODE layout, root scanning, yield boundaries, poisoned nodes, and allocation behavior.
3. **Evaluator/control flow:** suspend/resume, non-reentrancy, save/restore state, event boundaries, and platform-neutral fixes.
4. **Parser and I/O:** character-source indirection, EOF/unget, streams, output flushing, prompts, and file semantics.
5. **Printing/terminal:** shared output contracts, literal native output, output contexts, and platform hooks.
6. **Graphics:** shared turtle state and backend-neutral operations while preserving wx behavior.
7. **Build foundation:** minimal Emscripten detection, source selection, profiles, and configuration checks.

Every Phase 2 PR reruns the native matrix. Web validation begins as soon as that target can build. A partial shared PR identifies the later PR that supplies its missing runtime and the invariant it establishes now.

## Phase 3 — Establish a minimal WebAssembly walking skeleton

Create the smallest independently buildable browser and Node targets, then extend them vertically.

1. **Minimal build and link:** configure Emscripten explicitly, produce browser and Node artifacts, stage minimum Logo/help data, and run one deterministic command in each target. Derive the file inventory from the dependency graph rather than copying `wasm` wholesale.
2. **Minimal terminal paths:** prove startup, input, output, failure reporting, and shutdown with one focused browser test and one Node test. Rebuild artifacts from the PR revision before testing.
3. **Runtime lifecycle:** land Asyncify/yield boundaries, C/JS non-reentrancy, queued input/UI events, pause/stop/wait/shutdown, readiness, and error/rejection reporting in coherent slices.
4. **Logo/JavaScript bridge:** separate primitive registration, arity, value conversion, hostile/multiline payloads, and browser-versus-Node behavior from unrelated frontend work.
5. **Web graphics:** add SVG rendering and observable drawing-state checks; land labels, sprites, and animation separately where practical.
6. **Filesystem and persistence:** split resource staging, IDBFS readiness, reload persistence, upload/download/editor flows, Node `NODERAWFS`, and storage-failure fallbacks as dependencies permit.

## Phase 4 — Land the browser experience in feature-sized PRs

Add browser UI only after the runtime foundation is reviewable. Suggested boundaries are terminal layout/focus/input, mobile and soft-key behavior, editor lifecycle, autocomplete, help UI, themes/text/BEL/settings persistence, accessibility, graphics controls/demos, and additional Web primitives. Each PR includes its focused source, browser, or Logo regressions rather than deferring all coverage to an omnibus PR.

## Phase 5 — Land tooling, CI, packaging, and deployment

### Tooling and dependencies

Land npm scripts and lockfile, source/browser/Node runners, the latest-supported tooling Node policy, pinned Emscripten workflows, profile selection, and dependency/license/security review in coherent slices.

### Continuous integration

Preserve native jobs and add separate Web gates for clean browser/Node builds, individually graded Logo results, source tests, functional browser tests, optional audits/debug profiles, and artifact/toolchain provenance. A skipped required check is visible and explained, not silently green.

### PWA and deployment

Land manifest/icons, service-worker generation, Workbox configuration, staging, deploy workflow, offline/reload checks, and release verification separately where practical. Generated JavaScript/Wasm/service-worker outputs follow repository policy. Staging can be tested without publishing; tag pushes and production deployment require explicit approval.

### Documentation

Update build instructions, `webedition.md`, help/manual material, platform support, test commands, examples, demos, and release notes alongside the capabilities they describe.

## Phase 6 — Repository organization and cleanup

Path organization is optional and must not be confused with upstream integration itself. If Web-only files move under a `web/` hierarchy, use move-only commits plus small reference-update PRs, preserve virtual-filesystem and deployment paths, and compare staged deploy trees before and after.

Controlled cleanup may remove obsolete configuration, duplicated adapters, dead code, stale plans, machine-specific links, and generated files. Do not bundle cleanup with behavioral change or silently alter runtime semantics.

<!-- Co-authored by GitHub Copilot. Scope: approved WASM upstream integration roadmap. Model: MAI Code 2; reasoning-effort setting unavailable. Task wall-clock time: 4m 10s partial measured interval (2026-11-21T16:49:45Z to 2026-11-21T16:53:55Z). Energy (kWh) and CO2: unavailable; no measured data or defensible estimation inputs available. -->
