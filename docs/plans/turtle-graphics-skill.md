# Plan: UCB Logo Turtle Graphics Skill

Status: **Reopened/pending.** Screenshot-runner and PNG-evidence defects escaped into a partial implementation and invalidate the prior downstream approval assumptions. Independent Plan QA and human approval of this revision are pending.
Task: `TGS-2026-11-21-SCREENSHOT-PLAN`
Reopen task: `TGS-2026-11-20-T1`
Date: 2026-11-21
Branch: `wasm`
Base Plan SHA-256: `6b10fabda17a78bf9b9b7aac2953b99b599153e7a83b4c0fd6da4e1b1bf8a2eb`
Escaped-defect revision named by the reopen record: `6604d03e44d7be8ddee50886e52beb8c0d11f5b3`
Live verified HEAD at revision: `76e4ee45e461978fe263203df2f8a2c5eda15f57`
Supplied but not live task-context HEAD: `857601d4da59b960b6fcfdf348b98e7f0560b292`
Current stage: Plan, reopened/pending.

This assignment authorizes editing this top-level Plan only. It does not authorize
editing the Test Plan, application or test implementation, browser execution,
builds, staging, commits, or stage advancement.

## Reopened Status And Escaped-Defect Basis

The [escaped screenshot-defect reopen record](../workflow-runs/TGS-2026-11-21-screenshot-defects-planning-reopen.md)
is the reopen basis. Independent review found that the following bounded partial
implementation areas passed through the Implement Tests stage and reached
Implement Changes without detecting two defects:

- `tests/acceptance/run-testcafe.js` did not recognize disabled screenshot mode
  and could export a false effective screenshot base.
- `tests/acceptance/runner.test.js` had no complete deterministic contract for
  default, explicit, disabled, and conflicting screenshot CLI modes.
- `tests/acceptance/turtle-graphics-examples.testcafe.js` checked only file
  existence and nonzero size instead of a real PNG signature and IHDR marker.

The reopen record identifies the affected partial-repair revision as
`6604d03e44d7be8ddee50886e52beb8c0d11f5b3`. Live verification found a different
HEAD (`76e4ee45e461978fe263203df2f8a2c5eda15f57`) and also found that the
supplied task-context HEAD `857601d4da59b960b6fcfdf348b98e7f0560b292` is not
currently checked out. The implementation and browser evidence therefore must
fingerprint the exact later implementation revision; this Plan does not infer
that any named revision is in effect at a stage boundary.

The partial implementation and any downstream assumptions are not a passed
Implement Changes deliverable. This reopened Plan repairs their planning scope;
it does not claim that the implementation defects are fixed.

## Research Approval

Research was approved by the user on 2026-11-20. These exact approved reports
remain the evidence base for the approved turtle skill scope:

- [Turtle drawing state](../research/turtle-drawing-state-2026-11-20.md), SHA-256
   `1a2894ebe71a10bf54e528d72a4c9414226dd7025204ddd579a859f69d445387`.
- [Turtle primitives and patterns](../research/turtle-primitives-patterns-2026-11-20.md),
   SHA-256 `9c38af4ff6ad25dae33a12660343b3f5537e57531a70c935f2e66d97333dc69c`.
- [Turtle platform findings](../research/turtle-platform-2026-11-20.md), SHA-256
   `dcda205b538090f980a49ef9c55412c593998a2e6f0a547efcf6b86333563e85`.

The [workflow checkpoint](../workflow-runs/turtle-graphics-skill-checkpoint-2026-11-20.md)
records the approved evidence and historical deviations. It does not supersede
this reopened gate or approve this revision.

## Objective

Create a focused, user-facing UCB Logo turtle graphics skill. Default to the
Web Edition when the user's target is unspecified, teach the shared drawing core
first, and provide concise wxWidgets portability notes. Preserve the approved
four-file skill package and the separately authorized runnable-example fixture.
Repair only the bounded screenshot-base and PNG-evidence defects identified in
this reopen. Do not change the interpreter or application behavior beyond the
TestCafe runner and fixture behavior required to validate this task.

## Scope And Ownership

The approved turtle skill owns practical drawing guidance: primitive lookup,
explicit state setup, reusable patterns, and edition-specific user-observable
notes. It complements rather than replaces neighboring skill ownership:

- `ucb-logo-programming` retains general Logo syntax, style, and programming
   guidance.
- `ucb-logo-copilot-assisted-coding` retains rapid live prototyping workflows.
- `ucb-logo-web-testing` retains browser/runtime testing procedures.
- `ucb-logo-web-frontend` retains Web UI, SVG, and rendering implementation
   guidance.
- `ucb-logo-interpreter-development` retains interpreter, C/JS bridge, and
   graphics implementation guidance.

The complete approved file package remains exactly:

| Deliverable | Sole future file owner | Responsibility |
| --- | --- | --- |
| `.github/skills/ucb-logo-turtle-graphics/references/turtle-primitives.md` | Primitive-reference author | Primitive names, aliases, classifications, state mutators, palette caveats, and cleanup distinctions. |
| `.github/skills/ucb-logo-turtle-graphics/references/turtle-patterns.md` | Patterns-reference author | State-first setup, reusable motion patterns, procedure-local state, cleanup, and bounded pattern guidance. |
| `.github/skills/ucb-logo-turtle-graphics/references/platform-notes.md` | Platform-reference author | Web-default behavior and wx portability comparison with explicit runtime unknowns. |
| `.github/skills/ucb-logo-turtle-graphics/SKILL.md` | Skill-entry author | Discovery entry point integrating the accepted references. |
| `tests/acceptance/turtle-graphics-examples.testcafe.js` | Example-test author | Extract and execute approved runnable `logo` fences, capture and reconcile evidence, and write reports. |
| `tests/acceptance/runner.test.js` | Runner-test owner | Unit/static coverage for effective screenshot-base resolution and propagation. |
| `tests/acceptance/run-testcafe.js` | Runner-repair owner | Deterministic screenshot CLI resolution, disabled mode, and child environment propagation. |

The browser fixture and runner repair are a bounded regression-recovery scope.
No new application UI, wx runtime, Web frontend, interpreter, sprite demo,
canvas-dimension, or other-browser deliverable is added.

## Shared Turtle Content Contract

All four skill authors preserve this approved contract:

1. Before drawing, inspect or establish every mutable setting that the drawing
   depends on. Cover pen color and size, pen mode and up/down position, turtle
   visibility, background, position and heading, wrap/fence/window mode,
   scrunch, shape, and palette as relevant.
2. Teach pen mode separately from pen up/down. `PENDOWN` changes only down state;
   `PENPAINT` selects paint mode and puts the pen down.
3. Describe `CLEARSCREEN` (`CS`) as erase plus home to `[0 0]` heading `0`, not as
   a general reset. Describe `CLEAN` as erase without moving or changing other
   established state.
4. Describe `PEN`/`SETPEN` as a pen-state bundle, not a complete turtle, screen,
   shape, wrap/scrunch, background, or palette snapshot.
5. State that the current Web logical canvas is `640 x 480` and that wx canvas
   bounds follow the host window. Mark this as current implementation behavior,
   not permanent or pixel-accurate. Do no additional dimension research.
6. Teach only names supported by local registrations, help, libraries, or Web
   startup wrappers. Exclude stale or unregistered spellings.
7. Distinguish source availability from runtime-tested appearance. Do not claim
   visual parity across Web/wx or browsers. Do not add a runnable sprite demo.
8. Mark complete standalone examples as `logo` fences and label non-runnable
   fragments distinctly. The browser fixture extracts and executes actual fence
   content rather than copying it into the fixture.

## Bounded Screenshot Runner And Fixture Repair

The following requirements are mandatory for the reopened implementation scope.
They supplement, and do not replace, the approved turtle skill behavior.

### Effective screenshot base

The shared runner must resolve one effective mode before invoking TestCafe:

- **Default mode:** if screenshots are enabled and no explicit screenshot CLI
  mode is supplied, the effective base is TestCafe's standard `screenshots`
  base. The runner must export that exact base to the child fixture.
- **Explicit path mode:** a screenshot CLI value containing
  `path=reports/turtle-graphics-examples/screenshots` resolves to that exact
  repository-relative or accepted absolute base. The runner must export the
  exact effective value and the fixture must derive physical paths from it.
- **Disabled mode:** `--disable-screenshots`, a screenshot capability omission,
  or the equivalent deterministic runner setting produces no effective
  screenshot base. It must export an empty/null representation, not `screenshots`
  or another false base.
- **Conflicting CLI modes:** an explicit disable combined with an explicit
  screenshot path/enable, mutually conflicting repeated screenshot options,
  or an explicit screenshot path when screenshot capabilities are explicitly
  disabled must fail before server or browser startup with a deterministic,
  identifying error. The runner must not guess precedence.

The fixture must consume the runner-exported effective base. It must not
independently hard-code `reports/turtle-graphics-examples` as the physical
capture location. A relative base is resolved against the TestCafe repository
working directory; an accepted absolute base is used exactly. The effective
base and child environment must agree with the option forwarded to TestCafe.

In disabled mode, no screenshot capture is allowed or required. The fixture
must stop at startup or its first screenshot boundary with a clear diagnostic
that screenshots are disabled and there is no effective base. It must not
report success, emit a false screenshot link, or imply that TestCafe captured
anything.

### PNG evidence

A file exists and is nonzero in length is not sufficient PNG evidence. After
TestCafe captures a file, the fixture must verify all of the following:

- the exact PNG signature: bytes `89 50 4E 47 0D 0A 1A 0A`;
- the IHDR format marker: the 8-byte chunk length field at offset 8 is `13`
  (`00 00 00 0D`) and bytes 12 through 15 are `49 48 44 52` (`IHDR`);
- the file remains inside the effective screenshot base and current-run UUID
  directory.

A missing file, wrong signature, missing/incorrect IHDR marker, unreadable file,
wrong base, wrong run directory, or wrong exact capture path fails that example
and reconciliation. Zero length is also rejected. Test-only sample text or an
HTML error page copied to a `.png` name must not pass.

### Current-run isolation And prior directories

Every fixture invocation generates a fresh UUID run directory beneath the
resolved effective base. Exactly one capture is associated with each runnable
fence in that run. The fixture may not scan an entire prior screenshot history,
select the newest matching file, or reuse a prior-run file as current evidence.
Prior nonempty UUID screenshot directories must remain untouched and unchanged.
A new run UUID must differ from the prior run and must appear in both report
formats and in each exact current-run screenshot path.

### Report and link reconciliation

The report JSON and Markdown must derive their current-run base, relative
paths, absolute paths, and Markdown links from the actual effective base and
exact files that TestCafe captured. They must not derive from a guessed report
directory or from stale files.

Reconciliation must map one manifest entry to one parsed runnable fence and
one exact valid screenshot. It must reject:

- stale files outside the current UUID directory;
- files with a mismatched name, wrong base, wrong run ID, wrong document/fence
  identity, or non-PNG bytes;
- extra screenshot files in the current-run directory that are not in the
  manifest;
- entry/fence/screenshot counts that do not agree;
- Markdown links that do not resolve to the exact file named by report JSON;
- a successful report entry when the effective base, exact absolute path, or
  validated PNG evidence is absent or mismatched.

The report JSON and Markdown may be generated only for an enabled screenshot
mode. In disabled mode, they must state `effectiveScreenshotBase: null` and
`screenshotsEnabled: false` without creating a capture success link.

### Focused unit and static coverage before browser execution

Before any browser fixture run, focused checks must cover:

- default screenshot base resolution;
- explicit `path=` resolution and propagation;
- disabled screenshot resolution and propagation;
- conflicting CLI settings and deterministic rejection;
- PNG signature/IHDR validation, including corrupt and size-only `.png` samples;
- exact current-run path derivation, prior-run exclusion, and stale/mismatch
  reconciliation;
- document/fence identity and entry/fence/screenshot count agreement.

The runner unit/static cases belong in `tests/acceptance/runner.test.js` and the
existing shared runner/fixture checks. Compact temporary static harnesses may
remain under `/tmp/copilot/TGS-2026-11-21-SCREENSHOT-PLAN/`; no checked-in
fixture copy or mock Logo source is required. A browser run is not a substitute
for these discriminating checks and must not run until unit/static checks pass.

## Bounded Future Deliverables

### Primitive Reference

Own only `references/turtle-primitives.md`. Organize canonical spellings and
aliases by drawing task, distinguish primitives from library procedures and
Web startup facilities, and cite local evidence. Acceptance: every taught name
is evidenced and classified; unsupported names and parity claims are excluded.

### Patterns Reference

Own only `references/turtle-patterns.md`. Explain state-first setup,
position/heading and pen-up movement, procedure-local state, cleanup choices,
and pen-snapshot omissions. Acceptance: no unqualified pristine-state assumption,
no promise of a runnable sprite demo, and explicit state around each example.

### Platform Reference

Own only `references/platform-notes.md`. Present Web as default and wx as a
portability comparison. State current 640 x 480 Web behavior and host-dependent
wx bounds, source/documentation conflicts, and unverified runtime appearance.
Acceptance: no visual parity claim and no further dimension research.

### Skill Entry And Reference Integration

After all three references pass independent QA, own only
`.github/skills/ucb-logo-turtle-graphics/SKILL.md`. Add valid YAML frontmatter,
a concise discovery-oriented description, state-first workflow, and one-level
relative links to the references. Acceptance: the entry is under 500 lines,
links resolve, and neighboring skill ownership remains clear.

### TestCafe Screenshot Fixture

Under the approved Test Plan stage, define the exact static/browser cases,
fixture protocol, disabled-mode behavior, report schema, screenshot naming,
reconciliation rules, and expected commands. Implement only the persistent
TestCafe fixture and temporary static harness authorized for this package.
Do not copy Logo source into the fixture or generate a runnable sprite demo.

## Definite Stage Sequence

1. Complete and independently QA this revised top-level Plan.
2. Obtain explicit human approval of the exact revised Plan SHA-256.
3. Revise `docs/plans/turtle-graphics-skill-test-plan.md` for the screenshot
   runner/fixture scope only.
4. Independently QA the revised Test Plan and obtain explicit human approval of
   that exact revision.
5. Enter Implement Tests. Add only discriminating red unit/static and fixture
   cases before implementation repair.
6. Run focused unit/static checks, obtain independent review and aggregate
   Implement Tests QA, and obtain explicit human approval before repairs.
7. Enter Implement Changes only for the approved repair and skill scope.
8. Run focused static/unit validation before any browser fixture execution.
9. Run the approved browser fixture in default screenshot mode, then explicit
   path mode, preserving prior runs and separately assessing both results.
10. Obtain independent implementation review for the exact integrated revision,
    aggregate QA against the cumulative stage package, and explicit human
    approval before declaring the task complete.

No stage may merge into or advance past a later gate. Silence or an earlier
approval does not approve a later revision.

## Parallel Implementation Map

This map is a parallelism floor for later approved stages, not authorization for
agents or implementation now. It has no active worker assignment.

| ID | Exact file or edit | Isolated write/build state | Inputs / dependencies | Readiness and wave | Focused check | Output contract | Integration / review / commit owners |
| --- | --- | --- | --- | --- | --- | --- | --- |
| P1-PLAN | `docs/plans/turtle-graphics-skill.md`: reopen status, repair requirements, and this map. | Existing `wasm` worktree; no build tree or server. Only this file may change. | Base Plan hash, reopen record, partial evidence, branch/HEAD state. | Ready now and currently owned by Development Lead; complete after formatting and hash capture. Serial-only gate deliverable. | `node scripts/format-whitespace.js --changed`; `git diff --check -- docs/plans/turtle-graphics-skill.md`. | Revised Plan SHA-256 and complete diff summary. | Lead integrates; independent QA; Orchestrator commits only if separately authorized. |
| TP-REOPEN | `docs/plans/turtle-graphics-skill-test-plan.md`: exact reopened Test Plan cases and commands. | Existing `wasm` worktree; no implementation or browser execution. One tracked file owner. | P1-PLAN approved and exact Plan SHA. | Blocked by Plan approval; serial-only Test Plan stage. | Local link/diff and Test Plan completeness checks. | Revised Test Plan SHA-256 and an approval-ready test specification. | Future Test Plan owner / Development QA / no commit before approval. |
| IT-RUNNER | `tests/acceptance/runner.test.js`: default, explicit, disabled, conflict, and child-environment unit cases. | One tracked file; no browser/build state. Independent of documentation writes. | TP-REOPEN approved; current runner and unit-test seams. | Ready in Implement Tests Wave 1 after Test Plan approval. Parallel with IT-FIXTURE. | `node --test tests/acceptance/runner.test.js` and the narrow runner suite. | Deterministically failing red tests for missing/incorrect contract, or passing baseline if behavior-preserving. | Future test owner; Development Reviewer review; Orchestrator aggregate QA/commit. |
| IT-FIXTURE | `tests/acceptance/turtle-graphics-examples.testcafe.js`: startup red cases and PNG/reconciliation assertions as approved. | One tracked fixture; no browser run before unit/static pass. | TP-REOPEN approved; no fixture implementation action beyond tests. | Ready in Implement Tests Wave 1. Parallel with IT-RUNNER but serial with later IC-FIXTURE on the same file. | Focused static parser/check plus fixture startup collection, with no silent zero-example pass. | Reviewable red test/fixture contract and clear setup failures. | Future test owner; Development Reviewer review; Orchestrator aggregate QA/commit. |
| IT-STATIC | `/tmp/copilot/TGS-2026-11-21-SCREENSHOT-PLAN/screenshot-contract.test.js`: compact PNG/IHDR and reconciliation cases. | Temporary task-owned file only; no repository tracked write or build state. | TP-REOPEN approved; runner/fixture exported seams and PNG fixtures modeled in memory. | Ready in Implement Tests Wave 1. Parallel with IT-RUNNER and IT-FIXTURE. | `node --test` on the temporary harness. | Red static evidence for signature, IHDR, stale/mismatch, and exact-path requirements. | Future static-test owner; Development Reviewer review; Orchestrator aggregate QA. |
| IT-QA | Aggregate Implement Tests package: no source edit. | Reads exact test diff and validation evidence. | IT-RUNNER, IT-FIXTURE, IT-STATIC complete. | Serial-only gate after red tests. | Aggregate stage criteria and exact revision/hash; no browser repair inferred. | PASS, FAIL, or BLOCKED plus exact human-approval package. | Independent Development QA; human approval; Orchestrator integration/commit. |
| IC-RUNNER | `tests/acceptance/run-testcafe.js`: resolve effective modes and reject conflicts deterministically. | Existing `wasm` worktree; one tracked file; no application/build state. | IT-QA PASS and human approval; exact approved Test Plan. | Ready in Implement Changes Wave 1. Parallel with IC-FIXTURE and IC-REF-*; serial relative to its unit-test owner only. | Narrow runner unit/static tests, then `node --check`. | Effective base or null; conflict-free arguments; child env agrees with TestCafe. | Future runner owner; Development Reviewer; Orchestrator integration/commit. |
| IC-FIXTURE | `tests/acceptance/turtle-graphics-examples.testcafe.js`: consume base, validate PNG/IHDR, isolate UUID run, reconcile reports and paths. | Existing `wasm` worktree; one tracked file; no app-source build. Serial ownership after IT-FIXTURE. | IT-QA PASS, human approval, IC-RUNNER contract. | Ready in Implement Changes Wave 1 after test-stage approval. Parallel with IC-RUNNER and all IC-REF-* writes. | Static/unit cases pass before browser execution; focused `node --check`. | Exact PNG evidence, current-run-only manifest, valid JSON/Markdown paths, and disabled-mode diagnosis. | Future fixture owner; Development Reviewer; Orchestrator integration/commit. |
| IC-REF-PRIMITIVES | `.github/skills/ucb-logo-turtle-graphics/references/turtle-primitives.md`. | Existing `wasm` worktree; sole file owner; no build state. | Approved scope and shared content contract. | Ready in Implement Changes Wave 1. Parallel with patterns, platform, and runner/fixture tasks. | Local name/link/state/static checks. | One accepted primitive reference. | Reference author; independent QA substep; Orchestrator integration/commit. |
| IC-REF-PATTERNS | `.github/skills/ucb-logo-turtle-graphics/references/turtle-patterns.md`. | Existing `wasm` worktree; sole file owner; no build state. | Approved scope and shared content contract. | Ready in Implement Changes Wave 1. Parallel with primitives, platform, runner/fixture tasks. | Local state-pattern/static checks. | One accepted patterns reference. | Reference author; independent QA substep; Orchestrator integration/commit. |
| IC-REF-PLATFORM | `.github/skills/ucb-logo-turtle-graphics/references/platform-notes.md`. | Existing `wasm` worktree; sole file owner; no build state. | Approved scope and shared content contract. | Ready in Implement Changes Wave 1. Parallel with primitives, patterns, runner/fixture tasks. | Platform caveat/link/static checks. | One accepted platform reference. | Reference author; independent QA substep; Orchestrator integration/commit. |
| IC-SKILL-ENTRY | `.github/skills/ucb-logo-turtle-graphics/SKILL.md`. | Existing `wasm` worktree; sole file owner; no build state. | IC-REF-PRIMITIVES, IC-REF-PATTERNS, and IC-REF-PLATFORM independently QA-PASS. | Blocked until all three reference QA PASS; serial-only skill-entry Wave 2. | YAML/frontmatter, relative links, length, and discovery static checks. | One accepted entry file. | Skill-entry author; independent QA substep; Orchestrator integration/commit. |
| INT-GATE | Integrate accepted runner, fixture, and four skill files; no new semantic edit. | Shared `wasm` worktree after isolated file owners finish. | All bounded implementation tasks and substep QA PASS. | Serial-only integration/reduce task after Waves 1-2. | Complete scoped diff and conflict/whitespace inspection. | One exact integrated stage revision. | Orchestrator integration owner; independent reviewer; no commit until required checks pass. |
| VAL-STATIC | Static/unit package over integrated diff. | Existing built or source tree; no browser/server. | INT-GATE exact integrated revision. | Serial-only before browser work. | Unit/static suite plus scoped `git diff --check`. | Green static evidence or bounded repair dispatch. | Orchestrator aggregates evidence; Development Reviewer; aggregate QA. |
| VAL-BROWSER-DEFAULT | Browser fixture in default screenshot mode. | Existing `wasm` artifacts and shared runner; no app rebuild unless source/ artifact freshness requires it. | VAL-STATIC green; effective default base; fresh artifact state. | Serial-only after static pass. Blocked until prior run validation decision. | Focused desktop Chrome fixture; inspect JSON and custom report. | Valid current UUID PNG set, reconciled report, and prior directories unchanged. | Future validation owner; Development Reviewer; Orchestrator aggregate QA. |
| VAL-BROWSER-EXPLICIT | Same browser fixture with explicit screenshot path. | Same shared runner/artifacts; separate invocation/report/screenshot run. | VAL-BROWSER-DEFAULT complete; no stale cache or prior report copied. | Serial-only after default validation and reset/run isolation. | Focused desktop Chrome fixture; inspect JSON and custom report. | Valid explicit-base current UUID PNG set and reconciled report. | Future validation owner; Development Reviewer; Orchestrator aggregate QA. |
| REV-INTEGRATED | Exact integrated implementation/review package. | Read-only review of final diff/artifacts. | INT-GATE plus VAL-STATIC and both browser validations. | Serial-only final review before aggregate QA. | Severity-ordered independent review of exact final revision. | Findings resolved or explicitly residual and re-reviewed. | Development Reviewer independent of authors; Orchestrator integration/commit. |
| QA-AGGREGATE | Aggregate stage QA and exact revision binding. | Read-only QA over final diff, checks, reviews, and commit range. | REV-INTEGRATED complete. | Serial-only after review. | Stage exit criteria, artifact freshness, coverage, and diff identity. | PASS, FAIL, or BLOCKED. | Development QA independent of author/reviewer; human approval. |

Shared prerequisites are the approved Plan SHA, approved revised Test Plan, existing
runner/test/fixture files, the current `wasm` worktree, and no app-source changes
for the turtle examples. Shared mutable state is the runner/fixture/report area,
so only one owner may write each file and browser validations run serially. The
DAG is acyclic:

- Plan approval -> Test Plan -> Implement Tests unit/fixture red work ->
  Implement Tests aggregate QA/human approval -> runner and fixture repairs.
- Implement Changes runner/fixture and reference tasks can run concurrently.
- A reference cannot affect `SKILL.md` until that reference passes independent
  QA. `SKILL.md` is therefore blocked until all three references pass.
- Integration waits for all implementation substeps and QA.
- Static/unit validation waits for integration and must pass before browser.
- Default browser validation precedes explicit-path browser validation to isolate
  report/screenshot state and keep each run identifiable.
- Independent review waits for both browser runs; aggregate QA waits for review;
  human approval waits for aggregate QA.

The critical path is **P1-PLAN -> TP-REOPEN -> IT-RUNNER + IT-FIXTURE + IT-STATIC ->
IT-QA/human -> IC-RUNNER + IC-FIXTURE + three references -> IC-SKILL-ENTRY ->
INT-GATE -> VAL-STATIC -> VAL-BROWSER-DEFAULT -> VAL-BROWSER-EXPLICIT ->
REV-INTEGRATED -> QA-AGGREGATE -> human approval**. The three references and
runner/fixture repair expose multiple ready parallel opportunities after
Implement Tests approval; the stage gates, shared integration, and browser/report
state explain all serial-only items.

## Verification Strategy

This Plan revision is Plan-stage static work only. Verify the exact Plan file,
base hash, reopen links, no prohibited file edit, format normalization, and
scoped whitespace. No build, browser, test, dependency, or server action is
authorized here.

Later work follows these bounded layers:

1. Static and unit tests run before browser execution. They cover runner modes,
   child environment propagation, PNG signature/IHDR parsing, exact paths,
   stale/mismatch discrimination, and report reconciliation.
2. The persistent TestCafe fixture extracts actual runnable `logo` fences and
   executes one example in each fresh desktop headless Chrome runtime.
3. Browser validation uses the existing acceptance runner and known-fresh Web
   artifacts. It is reported separately for default and explicit screenshot
   bases. Disabled mode is a diagnostic unit/static/fixture-startup case, not
   a browser capture success mode.
4. Static unit evidence and browser report evidence are inspected individually;
   process exit alone is not success evidence.

The revised Test Plan owns exact commands and schemas after Plan approval.
Existing cross-browser functional coverage is not repeated.

## Acceptance Criteria

The revised Plan is ready for independent QA and human approval only when:

- reopened status, escaped-defect basis, exact affected artifacts, and revision
  uncertainty are recorded;
- the already-approved turtle skill scope is preserved and no application, UI,
  wx, other-browser, sprite-demo, or dimension-research expansion is present;
- every screenshot CLI mode and conflict is specified for a deterministic
  effective base;
- disabled mode returns no false base and prevents or clearly diagnoses capture;
- PNG validation requires both signature and IHDR, not size alone;
- UUID isolation and prior-directory preservation are explicit;
- report JSON/Markdown paths and Markdown links derive from the effective base
  and exact captured files;
- stale/mismatched files are rejected and entry/fence/screenshot counts reconcile;
- focused unit/static coverage precedes browser execution;
- the complete parallel map is acyclic, implementation-ready, and exposes
  dependency-ready work without overlapping file owners;
- exact gate sequence and human/QA boundaries are definite;
- visual impact is N/A and no visual parity claim is added;
- rollback and exclusions preserve unrelated work and limit corrections to
  task-owned files.

The future skill package remains acceptable only when exactly the approved four
skill documents and bounded runner/fixture repair are delivered. The browser
fixture runs only desktop headless Chrome and only complete executable `logo`
fences. Static checks remain in their separate lower-layer scope.

## Visual Impact

**N/A.** This plan proposes no product UI, layout, typography, color, imagery,
animation, responsive behavior, or interaction design. Screenshots are runtime
execution evidence for a test fixture, not a proposed visual design or evidence
of Web/wx/other-browser rendering parity. No visual design review is applicable.

## Exclusions

- No application/interpreter code, generated Web build repair, UI, wx runtime,
  manual, existing skill, sprite demo, or unrelated test change.
- No additional canvas-dimension research or pixel-coordinate promise.
- No Node Logo execution for the turtle examples, mobile Chrome, device
  emulation, Firefox, remote Safari, or cross-browser rerun in this task.
- No build, browser test, server, package install, network, or dependency action
  in the Plan stage. The later browser stage uses the existing runner only.
- No edit to `docs/plans/turtle-graphics-skill-test-plan.md` in this assignment.
- No agent launch or assignment from this Plan task, staging, commit, or push.
- No claim of visual parity, rendering parity, or wx behavior from screenshots.

## Rollback

This Plan revision edits only `docs/plans/turtle-graphics-skill.md` and makes no
commit. If independent QA requests changes, revise only this file, rerun the
Plan checks, and update this revision's SHA-256. Do not reset or revert the
worktree.

Future stage rollback is similarly narrow:

- Revised Test Plan: correct or remove only the task-owned Test Plan update.
- Implement Tests: correct only `runner.test.js`, the turtle TestCafe fixture,
  and task-owned temporary static/report evidence. Do not remove unrelated test
  files or reports.
- Implement Changes: correct only the runner, fixture, and four approved skill
  files. Preserve and exclude the accounting ledger and
  `docs/research/skill-discovery-orchestration-2026-11-21.md`.
- Generated screenshot/report artifacts may be removed only when they are
  task-owned and subsequent edits have not changed them. Prior UUID screenshot
  directories are preserved rather than rolled back.
- The Orchestrator owns integrated rollback and commits. No full-worktree reset,
  rebase, rewrite, or external operation is permitted without separate approval.

Preserve pre-existing work in `docs/accounting/agent-usage-ledger.md` and all
other unrelated modifications. A defect found after gate approval stops affected
downstream work and returns the exact artifact/revision to the reopened gate.

## Risks And Mitigations

- **Escaped CLI-mode defect:** a runner may report a default or guessed base while
  capture is disabled or uses a different value. Keep resolution deterministic,
  export it explicitly, and unit-test every mode and conflict.
- **Size-only PNG defect:** arbitrary or stale files can masquerade as captures.
  Require signature and IHDR bytes and exact current-run path, name, and count.
- **Stale evidence:** prior UUID directories and old report files can be mistaken
  for current captures. Never scan by newest or reuse prior-run evidence.
- **Path drift:** custom TestCafe paths are option-relative and may disagree with
  hard-coded report assumptions. Derive all paths from the effective child base.
- **Revision mismatch:** the reopen record, task-context HEAD, and live HEAD differ.
  Fingerprint later implementation revisions instead of assuming commit identity.
- **State leakage:** startup files or earlier examples can mutate turtle state.
  Require each example to establish relied-on state and use fresh browser runtimes.
- **Conflicting stale documentation:** names and platform claims remain evidence-
  bounded. Runtime unknowns stay explicit and no visual parity is claimed.
- **Shared mutable reports/screenshots:** separate browser invocations can overlap
  report state. Run default and explicit modes serially and preserve prior UUID
  directories.
- **Unobservable model metadata:** runtime model/effort metadata is not available
  to prove this Plan was produced by a named setting. This is a process deviation,
  not evidence of or a fallback to another model.

## Open Decisions

No scope ambiguity prevents this Plan revision. The only implementation inputs
that remain stage-owned are the revised Test Plan's exact CLI syntax, report
schema, capture names, and browser grading assertions. These belong in the
approved Test Plan, not this Plan. The Plan requires deterministic behavior but
does not choose a later implementation-specific filename format beyond exact
manifest, UUID, and current-run reconciliation rules.

The live HEAD and named partial-repair revision mismatch is recorded as a
provenance risk, not treated as authorization to amend, reset, or guess at the
affected implementation. It must be reconciled by the implementation owner from
the exact later approved revision.

## Co-author attribution

<!-- Co-authored by GitHub Copilot. Model: MAI Code 2 Flash; reasoning-effort setting: maximum supported effort requested and runtime setting unverified. -->
