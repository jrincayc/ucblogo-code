# Plan: Implement WASM Upstream Integration Phase 1

> **Status:** Proposed implementation plan. It is consistent with the approved Phase 0 and upstream-integration source documents, but does not authorize implementation, branch creation, commits, rebases, pushes, pull requests, builds, dependency installation, or releases. Work begins only after the maintainer explicitly authorizes the selected Phase 1 scope and approval gates.

## Objective

Restore and protect the wxWidgets and Unix terminal builds on short-lived `master`-based branches, prove the combined native result in an isolated private integration branch, and then extract independently valid PRs.

Phase 1 is complete when the native matrix is known-good on a current upstream baseline and the intentional `wasm` native-restoration work has been divided into coherent, independently reviewable PR units.

## Scope and non-goals

### Phase 1 includes

- Native build selection in `configure.ac` and `Makefile.am`.
- Platform guards and shared declarations needed by native builds.
- Native terminal backend restoration.
- Native input/output compatibility.
- Native GC/stack-root contracts and directly coupled callers.
- Demonstrated warning, overflow, and defensive fixes.
- Native Logo regressions and native CI wiring.
- An independent private integration branch used only to validate the combined native result.
- Short-lived PR branches derived from current `master` or a named predecessor PR.

### Phase 1 excludes

- Browser, Node, and Emscripten runtime work.
- Web-only C/JavaScript, HTML/CSS, PWA, service worker, deployment, or generated Web artifacts.
- `webterm.c`, web event queues, Asyncify/JSPI, `runjs`, animation, JS primitives, and Web graphics.
- macOS, Windows, BSD, native X11 runtime validation, and wx HTML rendering.
- Reworking or rewriting `wasm` history.
- Retiring or superseding PR #268 except as explicitly directed by the maintainer.

## Preconditions

Before application work starts:

1. Verify `origin` and `upstream` remote URLs and the current `jrincayc/ucblogo-code:master` and `wasm` refs.
2. Record immutable starting SHAs, the merge base, and PR #268 head/base/state.
3. Complete the Phase 0 full-delta inventory and dependency graph.
4. Complete fresh baseline evidence for `master` and the anchored `wasm` snapshot.
5. Establish the ongoing post-anchor `wasm` commit ledger.
6. Obtain explicit approval for the private integration branch and the requested approval gates below.

Phase 0 is authoritative for baselines and inventory. Use `docs/plans/wasm-upstream-phase-0.md` and `docs/plans/wasm-upstream-integration.md` as planning sources; do not silently alter their scope.

## Execution sequence

### 1. Refresh and freeze the backport inventory

- Use a fresh isolated checkout and refresh refs without changing history.
- Re-record the upstream and `wasm` SHAs because `master` may advance.
- Classify `git diff master...wasm` by path and meaningful hunk:
  - `master`-existing;
  - new on `wasm`;
  - deleted or renamed;
  - generated or binary;
  - legacy restoration;
  - shared or Web-only.
- Build the compile/link and test dependency graph.
- Do not use a bulk cherry-pick or copy the complete branch.

### 2. Create the private integration branch

- Start from the latest verified upstream `master` SHA.
- Name and record the private branch explicitly before creation.
- Do not modify, merge into, rebase, or cherry-pick a batch onto `wasm`.
- Keep local commits semantic and focused so that the validated result can be partitioned later.
- Do not open a PR or publish the private branch until the integrated pre-PR gate passes.

### 3. Backport the minimum native restoration

Apply changes in dependency order, not merely in eventual PR order:

1. **Build selection** — `configure.ac` and `Makefile.am`
   - Preserve explicit target detection.
   - Select exactly one terminal implementation.
   - Exclude Emscripten options and Web link recipes.

2. **Platform guards** — `logo.h`, `globals.h`, `init.c`, `eval.c`, and `main.c`
   - Guard all Web/Emscripten dependencies from native targets.
   - Preserve native event, scheduler, entry-point, and callback behavior.
   - Do not add blanket native stubs merely to hide accidental Web dependencies.

3. **Native GC/stack contract** — `mem.c` and directly coupled callers
   - Follow `.github/instructions/gc-memory.instructions.md`.
   - Preserve native stack-root ranges and register/spill behavior.
   - Exclude Web yield, poisoned-node, large-Segment, ASan, and GC redesign work unless independently necessary for native correctness.

4. **Native input/output** — `files.c`, `parse.c`, and `print.c`
   - Preserve native/wx reader selection and EOF/unget behavior.
   - Preserve null-prompt handling and prompt flushing.
   - Keep ordinary output literal on terminal and wx targets.
   - Preserve dribble/file/string semantics.
   - Exclude browser DOM formatting and Web event-queue behavior.

5. **Native terminal backend** — `term.c`
   - Restore the non-web/non-wx implementation.
   - Preserve capability lookup, character-mode transitions, termios restoration, cursor/margin operations, signals/exits, and redirected I/O.
   - Do not introduce `webterm.c` in this phase.

6. **Small defensive fixes**
   - Backport only warning, overflow, prototype, or defensive fixes proven necessary for native builds.
   - Keep unrelated cleanup and behavior changes out.

### 4. Add native regressions

- Extend existing `tests/UnitTests.lg` and `tests/UnitTests-*.lg` suites only for established native behavior.
- Cover evaluator, file, memory, output, terminal, and compatibility cases as changes land.
- Do not import Web runner assumptions or adopt Web-only OOP exclusions on native targets.
- Run object-enabled and default/object-disabled configurations where affected.

## Approval gates

### Gate A — Unix PTY support

Explicitly approve the exact path `tests/native/pty_terminal.py` before implementation.

If approved, the harness should cover only the checks needed for known-good status:

- termios restoration after character-mode reads;
- termios restoration on normal exit;
- signal/interrupt behavior;
- clear/cursor/margin sequences;
- redirected input/output;
- EOF behavior;
- the documented upstream Ctrl-C SIGSEGV baseline where still applicable.

If denied, maximize existing-file coverage and record the interactive gap. Do not silently present PTY coverage as complete.

### Gate B — wx smoke support

Explicitly approve the exact path set, including:

- `tests/native/wx_smoke.py`;
- `tests/native/wx-smoke-baseline/README.md`;
- every PNG under `tests/native/wx-smoke-baseline/`.

Approval must cover binary/baseline additions. Use CI captures and Xvfb baselines; do not use workstation captures to replace them. Do not change interpreter behavior merely to satisfy screenshots.

### Gate C — native CI wiring

Add native CI to `.github/workflows/ci.yaml` only after repaired builds and tests are usable locally.

Required jobs or equivalent gates:

- wx with objects;
- Unix terminal with objects;
- Unix terminal without objects;
- approved PTY checks;
- approved wx smoke checks.

CI must:

- assert the effective target macros;
- regenerate help before native builds where required;
- reject missing, empty, timed-out, or non-`Ok` reports;
- retain logs and screenshots on failure;
- keep Web jobs outside Phase 1 edits.

## Integrated pre-PR gate

A fresh checkout of the private integration branch must pass all required native evidence:

1. Regenerate Autotools files without stale active configuration.
2. Build wx explicitly:

   ```text
   --disable-wasm --enable-wx --enable-objects --enable-gitid --disable-docs
   ```

   Assert `HAVE_WX=1`; run and grade the full wx Logo suite under Xvfb; run approved smoke comparisons.

3. Build Unix terminal with objects:

   ```text
   --disable-wasm --disable-wx --disable-x11 --enable-objects --enable-gitid --disable-docs
   ```

   Assert native terminal selection; run and grade the suite, literal-output checks, redirected I/O checks, and approved PTY checks.

4. Build Unix terminal without objects:

   ```text
   --disable-wasm --disable-wx --disable-x11 --disable-objects --disable-gitid --disable-docs
   ```

   Assert `WASM_MISSING=1`; run and grade the object-free suite.

5. For GC/stack changes, run MemMgr, allocation-heavy, and repeated-GC checks at the native CI optimization level.
6. Compare failures against untouched `master`.
7. Record upstream-existing exceptions rather than hiding or silently repairing them.
8. Review the integrated diff for accidental Web imports, generated artifacts, mixed concerns, missing distribution entries, and inseparable hunks.

Do not open or submit a PR until this gate passes. The private integration branch proves the combined result but does not itself become a PR.

## PR slicing

Derive separate short-lived branches from current `master` or an explicitly named predecessor PR. Recommended dependency order:

1. **Native test infrastructure and CI**
   - Existing Logo test-path improvements.
   - Only the separately approved PTY and/or wx smoke additions.
   - Native CI that passes against the declared base.

2. **Native build selection and platform guards**
   - Native-only `configure.ac` and `Makefile.am` changes.
   - Shared declarations and Web guards required for native builds.

3. **Native terminal backend restoration**
   - `term.c` and focused build wiring.
   - Terminal regressions.

4. **Native I/O compatibility**
   - `files.c`, `parse.c`, `print.c`, and focused file/output regressions.

5. **GC and stack-root contract**
   - `mem.c` and only coupled callers.
   - Native memory-manager and allocation-heavy regressions.

6. **Independent defensive fixes**
   - One concern per PR where practical.
   - Exclude hunks that belong naturally to an earlier subsystem.

Adjust these boundaries from the Phase 0 hunk graph. Merge hunks into one PR if either cannot build or pass tests independently. Keep semantic cohesion ahead of a fixed line-count target.

### PR requirements

Every PR must:

- target `master` or a named predecessor PR;
- contain one coherent review question;
- include exactly the tests/CI needed for that block;
- build from its own base in a fresh isolated checkout;
- run focused regressions plus the applicable common native gates;
- explain excluded neighboring changes;
- not depend on unpublished work left only in the private integration branch;
- be revalidated after any rebase onto a changed `master` or predecessor.

## Reconciliation and completion

Maintain:

- the immutable starting inventory;
- the ongoing `wasm` commit ledger;
- a maintainer-readable PR-stack tracker;
- base/predecessor and validation evidence for each PR;
- an account of intentional exclusions and later-phase dependencies.

Before Phase 1 closes:

- refresh `master`, `wasm`, and PR #268 state;
- reconcile every post-anchor `wasm` commit;
- verify the remaining native delta is explained;
- confirm all Phase 1 PRs are merged or explicitly deferred;
- confirm integrated upstream state passes the native matrix;
- preserve unresolved upstream baseline failures as documented exceptions unless scope is changed.

PR #268 is not to be retired or rewritten automatically. Its state must follow maintainer direction after the PR series replaces or supersedes it.

## Relevant files

- `docs/plans/wasm-upstream-integration.md` — integration source of truth.
- `docs/plans/wasm-upstream-integration-summary.md` — maintainer-facing summary.
- `docs/plans/wasm-upstream-phase-0.md` — approved Phase 0 procedure.
- `docs/plans/native-build-restoration.md` — native repair history and evidence.
- `docs/plans/pr-268-description.md` — source inventory and native-risk evidence.
- `docs/plans/wx-smoke-ci.md` — wx smoke and CI evidence.
- `scripts/upstream-diff.sh` — delta helper, subject to remote/merge-base verification.
- `configure.ac`, `Makefile.am` — native and Web build selection.
- `logo.h`, `globals.h`, `init.c`, `eval.c`, `main.c` — target guards and shared contracts.
- `mem.c` — native stack/GC contract.
- `files.c`, `parse.c`, `print.c` — native input/output contracts.
- `term.c` — native terminal backend.
- `tests/UnitTests.lg`, `tests/UnitTests-*.lg` — existing Logo regressions.
- `.github/workflows/ci.yaml` — native CI and result grading.

## Stop conditions

Stop and report an andon for:

- ambiguous or conflicting upstream refs/SHAs;
- a change that would require rewriting or modifying `wasm` history;
- a proposed PR that cannot pass independently;
- a mixed Web/native hunk whose scope cannot be separated;
- an approval gate whose exact path or scope is unconfirmed;
- stale or unverifiable build/test artifacts;
- an accounting, privacy, or commit-scope defect;
- three unsuccessful attempts at the same repair, as required by repository workflow policy.

Work shall not resume after an andon until the human approves a resolution.

<!-- Co-authored by GitHub Copilot. Scope: WASM upstream integration Phase 1 implementation plan. Model: MAI Code 2; reasoning-effort setting unavailable. Elapsed wall-clock time: to be recorded at task close. Energy (kWh) and CO2: unavailable; no measured data or defensible estimation inputs available. -->
