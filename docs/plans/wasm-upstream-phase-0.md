# Plan: Implement WASM Upstream Integration Phase 0

> **Status:** Approved for implementation as a planning artifact. This document establishes the Phase 0 procedure; it does not authorize branch rewrites, pushes, pull requests, deployment, or Phase 1 application changes.

## Objective

Establish an immutable, reproducible, maintainer-auditable baseline for integrating the intentional work on `wasm` into `jrincayc/ucblogo-code:master` through small, independently reviewable pull requests.

Phase 0 is complete when the starting refs, complete branch delta, dependency graph, baseline behavior, ongoing-delta process, and maintainer approval gates are documented with enough evidence for another developer to reproduce the conclusions.

## Scope and non-goals

Phase 0 may create planning reports, structured inventories, temporary worktrees, and validation evidence.

Phase 0 must not:

- modify the interpreter, Web Edition, tests, build recipes, workflows, generated Web outputs, or deployment behavior;
- rebase, rewrite, force-push, merge, or cherry-pick onto `wasm`;
- push commits or open pull requests;
- start the Phase 1 private integration branch;
- repair baseline failures discovered by validation;
- clear browser/site state, service workers, or user data;
- imply macOS, Windows, BSD, X11 GUI, or physical-device coverage when only wx and Unix terminal were checked.

The integration target is `jrincayc/ucblogo-code:master`. The local remote name is evidence to verify, not an invariant.

## Deliverables

1. A baseline report containing immutable SHAs, ref and PR state, tool versions, commands, results, known failures, confidence, and coverage gaps.
2. A complete path and hunk inventory covering the anchored `master...wasm` delta.
3. A dependency graph and a tentative, non-authorizing set of review boundaries.
4. A reproducible baseline matrix for the anchored `master` and `wasm` snapshots.
5. An ongoing-delta ledger protocol for every commit added to `wasm` after the anchor.
6. A maintainer-readable tracker listing approval gates, deferred decisions, intended PR questions, bases/predecessors, and intentionally excluded neighboring work.

The report, inventory, and ledger may be separate files or sections of the same report. Their formats should remain machine-auditable where practical.

## Phase 0 work sequence

### 1. Lock the audit frame

1. Verify the configured `origin` and `upstream` remotes and fetch/read only the refs needed to identify the canonical upstream `master`, the canonical `wasm` development head, and PR #268’s current head, base, and state.
2. Record, without modifying history:
   - upstream `master` full SHA;
   - starting `wasm` full SHA;
   - merge base;
   - local and remote ref names;
   - PR #268 number, state, head SHA, base SHA, and observation time;
   - current branch, worktree state, and uncommitted/untracked paths;
   - exact observation timestamp.
3. Treat the recorded SHAs as immutable anchors. A later refresh creates a new, explicit `from_sha`/`to_sha` interval; it never moves the original anchor.
4. Keep phase evidence in fresh isolated worktrees under `/tmp/copilot`. Do not build from the active workspace or reuse its configured `Makefile`, generated outputs, browser cache, or executables.

**Exit evidence:** a ref table and read-only command log that another developer can repeat to recover every anchor.

### 2. Inventory the complete delta by hunk

1. Generate path-level evidence for the anchored `master...wasm` delta using name/status, rename detection, binary detection, and line-count summaries.
2. Assign a stable inventory ID to each meaningful hunk. Classify it with one primary category:
   - legacy restoration;
   - shared interpreter/build work;
   - Web runtime;
   - browser UI;
   - Logo/JavaScript bridge or primitives;
   - graphics;
   - filesystem/persistence;
   - tests;
   - CI/CD;
   - packaging/deployment;
   - documentation/help/examples;
   - generated output;
   - deletion;
   - obsolete or accidental work.
3. For each path, record whether it exists on `master`, is new on `wasm`, is deleted, was renamed, is generated, or is binary.
4. Cross-check tracked, ignored, generated, deleted, and moved paths against build recipes, deployment staging, help generation, tests, and workflows. A path outside the primary source diff still belongs in the inventory if it is required to build, test, package, or explain intentional behavior.
5. Split whole-file changes by meaningful hunk where one path contains independent concerns. Do not hide multiple dependency models behind a single path classification.
6. Mark uncertainty explicitly with an owner, question, and proposed resolution. Do not silently assign ambiguous or accidental work.

Suggested inventory fields:

| Field | Meaning |
| --- | --- |
| ID | Stable identifier for reconciliation |
| Path / range | File and hunk or line range |
| Category | Primary classification |
| Path state | Existing, new, deleted, renamed, generated, binary |
| Intent | Behavior or support supplied |
| Depends on | Inventory IDs and external prerequisites |
| Required by | Inventory IDs, targets, tests, packaging, or docs |
| Inseparability | Independent, dependent, or technically inseparable |
| Candidate phase/PR | Tentative review boundary; not authorization |
| Confidence / blocker | Evidence quality and unresolved question |

**Exit evidence:** every path in the anchored name/status diff appears in the inventory exactly once, with unexplained entries counted and required to be zero before completion.

### 3. Build the dependency graph

Trace concrete edges, not only file proximity:

- `configure.ac` and `Makefile.am` source selection, target detection, profiles, compile flags, link rules, generated help, and packaging inputs;
- C headers, node layout, ABI/export contracts, target guards, and preprocessor boundaries;
- evaluator control flow, GC roots/lifetime, yield boundaries, and runtime state;
- parser, stream, prompt, EOF, and output contracts;
- native terminal, wx, browser terminal, Node terminal, and graphics backends;
- Logo/JavaScript value conversion, primitive registration, arity, and runtime errors;
- Web lifecycle, asynchronous input, cancellation, and shutdown;
- virtual-filesystem embedding, resource paths, persistence, uploads, and downloads;
- Logo suites, Node runners, TestCafe fixtures, native PTY/smoke harnesses, result grading, CI jobs, and artifacts;
- npm commands, Workbox/service-worker inputs, deploy staging, docs, examples, and help assets.

For every inventory item, record whether it is:

- independent;
- dependent on named items;
- technically inseparable from named items;
- generated from named source files;
- intentionally excluded;
- approval-gated.

Use the graph to identify candidate later-phase boundaries and prerequisite stacks. Do not treat a mechanically small diff as independently reviewable if it requires multiple mental models or unpublished predecessor work.

**Exit evidence:** the graph contains representative traces for native-only, shared-interpreter, Web-runtime, frontend, persistence, test, and deployment slices; unresolved cycles or missing edges block completion.

### 4. Capture clean baseline evidence

Create detached worktrees for the anchored `master` and `wasm` SHAs. Record the host OS, available CPU/memory/disk at the time of scheduling, compiler/linker, Autoconf/Automake, wxWidgets, Node/npm, Emscripten, relevant browsers, and TestCafe versions. Do not record secrets.

#### Anchored `master`

In fresh native configurations, capture:

- Unix terminal with objects;
- Unix terminal without objects;
- wx with objects;
- wx without objects when the repository recipe supports it;
- clean compile/link results;
- target-macro/configuration proof;
- startup behavior;
- individually graded Logo test results;
- focused PTY and wx smoke evidence only if the harnesses already exist at that anchor or have separate explicit approval.

#### Anchored `wasm`

In fresh isolated trees, capture:

- the same wx and Unix terminal matrix;
- browser WebAssembly build;
- Node WebAssembly build;
- deterministic Node Logo suite;
- focused browser smoke and agreed functional coverage using the existing TestCafe runner;
- generated-source provenance and selected configuration/macros;
- stale-artifact, cache, and service-worker controls applied to browser evidence.

#### Result rules

For every command, record:

- exact SHA and worktree;
- command and target/configuration options;
- toolchain and relevant environment versions;
- start/end time and duration;
- exit status;
- expected and actual test counts;
- failed, missing, empty, skipped, and timed-out reports;
- log/artifact location;
- whether a failure also reproduces at the other anchor;
- confidence and uncovered scope.

A passing aggregate command is insufficient. Missing expected results, zero test results, missing reports, and unexplained skips are failures of the evidence set. Phase 0 records defects and baseline limitations; it does not repair them.

**Exit evidence:** a complete result matrix with no unlabeled failed/skipped coverage and no outputs whose provenance is unknown or stale.

### 5. Track the ongoing `wasm` delta

Keep an append-only ledger for every commit added after the immutable `wasm` anchor.

Required fields:

- observation timestamp;
- previous and refreshed `wasm` SHA;
- commit SHA and subject;
- classification: pending-work fix, follow-up to upstream-landed behavior, future feature, documentation, generated-only work, or intentional exclusion;
- affected inventory IDs;
- required predecessor or later disposition;
- evidence/command source;
- resolution status.

Record no-change checkpoints too. At a refresh boundary, prove that every commit in the interval appears exactly once. Recheck PR #268 and both canonical refs:

- before maintainer handoff;
- before any Phase 1 branch is created;
- before each later integration PR is based;
- whenever the maintainer explicitly asks for reconciliation.

Refreshed refs do not replace the original baseline. A changed upstream base or `wasm` head requires a new interval and impact analysis against the original inventory.

**Exit evidence:** a reproducible commit-range calculation accounts for every post-anchor commit, including merge, documentation, and generated-only cases.

### 6. Publish the maintainer tracker and approval gates

Publish a maintainer-readable tracker with:

- Phase 0 anchor and status;
- candidate PR purpose and one review question;
- base and named predecessor PRs;
- inventory IDs included and explicitly deferred;
- validation evidence required;
- generated/binary implications;
- approval status;
- unresolved dependency or coverage gap;
- convergence status against the current `wasm` interval.

Phase 0 must obtain or clearly request separate approval for each material new path or policy decision, especially:

- `tests/native/pty_terminal.py`;
- `tests/native/wx_smoke.py`, its README, and every screenshot baseline;
- other new binaries or generated artifacts;
- path moves or repository reorganization;
- public CI/browser matrix expansion;
- paid, hosted, device, or release infrastructure;
- any platform scope beyond wx and Unix terminal for the native integration baseline;
- any change to generated-artifact policy.

The tracker may propose Phase 1 review boundaries, but it must not create branches or claim those boundaries are approved.

**Exit evidence:** a maintainer can identify the remaining decisions and understand why each candidate PR is or is not independently valid without reading the entire `wasm` diff.

## Proposed validation matrix

| Surface | Anchored `master` | Anchored `wasm` | Required evidence |
| --- | --- | --- | --- |
| Unix terminal | Fresh objects/default builds where supported | Fresh objects/default builds | Configuration proof, compile/link, startup, graded suites |
| wx | Fresh objects/default builds where supported | Fresh objects/default builds | Configuration proof, compile/link, startup, graded suites, explicitly bounded GUI evidence |
| Browser WASM | Not expected unless present at anchor | Fresh production-equivalent clean build | Regenerated outputs, readiness, startup, focused TestCafe results |
| Node WASM | Not expected unless present at anchor | Fresh clean build | Regenerated outputs, individually graded Logo suite, exit status |
| CI/deploy wiring | Configuration/source inspection | Configuration/source inspection | Job/recipe trace; no deployment required |
| PWA/offline | Not expected unless present at anchor | Focused source/behavior evidence as approved | Cache/service-worker provenance and clear test limitations |

## Completion criteria

Phase 0 is complete only when all of the following are true:

- the upstream target and immutable `master`/`wasm` anchors are reproducible;
- the full anchored diff is inventoried with zero unexplained paths or hunks;
- generated and binary items are identified and traced to source or policy;
- dependency edges cover compile, link, runtime, tests, packaging, CI, deployment, and documentation;
- fresh `master` and `wasm` baseline evidence has explicit provenance and no stale/reused-state ambiguity;
- known failures are distinguished from regressions and left unfixed;
- all post-anchor `wasm` commits are accounted for in the delta ledger;
- approval gates and coverage gaps are visible to the maintainer;
- candidate Phase 1 review slices remain proposals, with no branch, PR, push, or application work performed.

## Implementation boundaries for this plan document

Committing this document records an executable plan and does not itself perform Phase 0 validation. A later task must explicitly authorize the build/test campaign and any additions to the repository beyond planning artifacts.

## Relationship to later phases

Phase 1 restores and validates native legacy builds on a separate private `master`-based integration branch, then derives independently valid PRs. Phase 2 and later introduce shared compatibility and vertical Web Edition slices. Follow `wasm-upstream-integration.md` as the planning source of truth; update it only through a separately approved plan correction if Phase 0 evidence exposes a material issue.

<!-- Co-authored by GitHub Copilot. Scope: WASM upstream integration Phase 0 planning document. Model: MAI Code 2; reasoning-effort setting unavailable. Energy (kWh) and CO2: unavailable; no measured data or defensible estimation inputs available. Task wall-clock time: 3m39s measured interval (2026-11-21T17:54:44Z to 2026-11-21T17:58:23Z). -->
