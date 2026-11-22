# Plan: Add Serial Three-Role Iterative TDD Candidate Workflow

Status: Draft for future refinement; not an approved or active workflow change.

Build a self-contained candidate workflow from the proven legacy workflow as a compatibility and policy baseline. Do not modify or retire the legacy workflow. Combine test and code implementation into one `Implement` stage, retain separate Research, Plan, and Test Plan stages, and use exactly three roles: one Worker that owns all productive work, plus independent Reviewer and QA roles. Execute implementation packets serially, one at a time.

## Delivery Strategy

- Work on a short-lived candidate branch from an explicitly recorded approved `wasm` base, suggested name `workflow/serial-tdd-candidate`.
- Keep every legacy workflow file untouched during candidate development and cutover.
- Legacy remains the production default until the candidate passes dry runs, independent review/QA, and explicit human acceptance.
- After acceptance, a selection-only change makes the candidate default; legacy remains an exact fallback.
- Legacy retirement is future work requiring a separate plan and approval.
- Task assignments explicitly select exactly one workflow and never mix stage rules.
- Use the legacy baseline for gates, human approvals, evidence identity, accounting, attribution, push approval, Andon, artifact freshness, and process ownership.
- Use only three workflow roles:
  - **Worker:** direct top-level productive role, not a subagent; owns Research, Plan, Test Plan, implementation, integration, documentation, tests, refactoring, validation, commits, and evidence packaging.
  - **Reviewer:** independent read-only subagent for code, policy, and artifact review.
  - **QA:** independent read-only subagent for stage-exit and acceptance evidence.
- Reviewer and QA may run concurrently against the same exact frozen revision/evidence package. Their checks must be read-only, idempotent, and must not edit, commit, delegate, or repair artifacts.

## Bootstrap Strategy

- Develop and validate the candidate through legacy Research, Plan, Test Plan, QA/review, approval, Andon, evidence, accounting, and push controls.
- The Worker uses TDD for candidate development: failing policy/static/dry-run check, smallest candidate change, rerun, refactor while green, repeat.
- Candidate exercises begin as simulations/dry runs and do not imply production authority.
- A validated candidate slice may govern only later work within its explicitly tested scope; everything else falls back to legacy.
- A failed or unsafe bootstrap returns to legacy without blocking `wasm` or the legacy default.
- Candidate acceptance remains legacy-governed; cutover switches the default while legacy remains untouched.

## Effort Estimate

- Phase 1: 40–75 minutes implementation plus 20–30 minutes validation.
- Phase 2: 25–45 minutes plus 15–25 minutes validation.
- Phase 3: 30–50 minutes plus 20–30 minutes review/QA.
- Phase 4: 1–2.5 hours dry runs/evidence, 20–45 minutes acceptance, 15–30 minutes cutover.
- Total: approximately 3–6 hours wall clock. Reducing role plumbing lowers coordination and handoff cost. Monetary cost is unavailable without measured consumption/rates.

## Role Contracts

### Worker

- Is the direct productive role and is not a subagent of another implementation agent.
- Owns Research, Plan, Test Plan, Implement, tests, documentation, fixtures/harnesses, production code, refactoring, integration, commits, validation, and evidence assembly.
- Runs one coherent packet at a time and owns it through completion or Andon.
- Runs focused validation after every substantive edit and keeps checks green after the intentional red phase.
- Uses `min(estimated effort × 1.5, 30 minutes)` as the packet timeout, with a halfway checkpoint. Oversized packets are decomposed or BLOCKED; timeout without handoff raises Andon.
- Allows at most three consecutive focused fix-and-check attempts after an unexpected failure or broken build. The third failure preserves evidence and raises Andon. Expected-red TDD is not a strike.
- Freezes an exact revision/evidence package before requesting review and QA; it cannot approve its own work.

### Reviewer

- Runs independently and read-only as a subagent over the exact supplied revision/evidence.
- Reviews correctness, security, regressions, missing tests/docs, scope, readability, cohesion, duplication, coupling, maintainability, compatibility, migration, and project conventions.
- Returns severity-ordered findings, focused checks, questions, and residual risks; it does not edit, fix, commit, or delegate.
- May run concurrently with QA against the same frozen package because both checks are read-only and idempotent.

### QA

- Runs independently and read-only as a subagent over the exact supplied revision, requirements, checks, and reviewer coverage.
- Verifies stage exit criteria, expected-red evidence, green checks/refactors, three-strike and timeout compliance, docs, exact artifact identity, and residual gaps.
- Returns PASS, FAIL, or BLOCKED with evidence; it does not edit, fix, commit, or delegate.
- May run concurrently with Reviewer against the same frozen package. Any material artifact change invalidates both verdicts and requires fresh concurrent reruns.

## Candidate Stages

1. **Research** — Worker produces evidence, affected surfaces, documentation implications, constraints, risks, and open questions.
2. **Plan** — Worker creates an ordered serial packet list with exact ownership, dependencies, checks, docs duties, estimates/timeboxes, and rollback. No parallel packet assignments.
3. **Test Plan** — Worker specifies cases, inputs/results, setup/teardown, harness/fixture needs, commands, documentation checks, and regression scope. No tests are implemented here.
4. **Implement** — Worker serially owns tests, production/docs changes, fixtures, and refactoring. The cycle is red -> green -> refactor. Shared infrastructure is an earlier serial prerequisite packet.
5. **Code Review and QA** — Freeze the exact revision/evidence and run Reviewer and QA concurrently. Worker repairs findings; material changes require both checks to rerun.

## Implementation Phases

### Phase 1 — Minimum Candidate

1. Create and record the candidate branch and exact `wasm` base.
2. Add a self-contained candidate contract with five stages and exactly three roles.
3. Add Worker, Reviewer, and QA role configurations; leave legacy roles untouched.
4. Add one small serial TDD policy task and the frozen-revision handoff format.
5. Validate wording, frontmatter, links, and whitespace.

### Phase 2 — Worker Guardrails

1. Add serial packet planning and Worker-owned Research/Plan/Test Plan/Implement/docs.
2. Add three-strike Andon, effort-derived timeout, 30-minute cap, focused-after-each-edit, expected-red handling, and artifact identity.
3. Add read-only/idempotent Reviewer and QA rules and exact frozen-package requirements.
4. Ensure Worker repairs findings; one material change invalidates both checks.

### Phase 3 — Bootstrap And Dry Runs

1. Use legacy governance to build the candidate test-first.
2. Dry-run TDD, documentation, shared prerequisites, broken-build recovery, timeout, and concurrent Reviewer/QA.
3. Verify only one implementation packet is active, no productive subagent exists, Reviewer/QA do not mutate state, and fallback to legacy works.
4. Label unaccepted candidate evidence pilot/simulated.

### Phase 4 — Acceptance And Cutover

1. Complete independent final Review and QA plus explicit human acceptance.
2. Change only workflow/default selection to use the candidate.
3. Verify legacy artifacts remain byte-for-byte unchanged.
4. Keep legacy as fallback and record retirement as future work.

## Candidate Files To Add Or Evaluate

- `.github/workflows/development-orchestration-candidate.md`
- `.github/agents/development-worker-candidate.agent.md`
- `.github/agents/development-reviewer-candidate.agent.md`
- `.github/agents/development-qa-candidate.agent.md`
- A minimal workflow selector activated only after acceptance.
- Candidate Plan, Test Plan, run, and evidence artifacts under existing conventions.

## Legacy Baseline References — Not Modification Targets

- `.github/workflows/development-orchestration.md`
- `.github/agents/development-*.agent.md`
- `.github/copilot-instructions.md`
- `docs/plans/development-orchestration-workflow.md`

## Verification / QA Checklist

1. Exactly three roles exist: Worker, Reviewer, and QA.
2. Worker is the direct productive role and not a subagent; Reviewer and QA alone are subagents.
3. Reviewer and QA are read-only, idempotent, non-delegating, and independent.
4. Reviewer and QA run concurrently only against the same frozen revision/evidence package.
5. A material repair invalidates both verdicts and requires fresh concurrent checks.
6. Worker owns Research, Plan, Test Plan, tests, docs, harnesses, production code, integration, commits, checks, and evidence.
7. Plan and Test Plan remain separate; implementation combines tests, code, docs, and refactoring.
8. Exactly one implementation packet is active; shared work is an earlier serial prerequisite.
9. Expected red is not a strike; three unsuccessful fix/check attempts trigger Andon.
10. Effort-derived timeboxes and the 30-minute global cap are monitored and dry-run-tested.
11. Candidate development uses TDD under legacy governance and labels unaccepted evidence pilot/simulated.
12. Cutover changes selection only, leaves legacy intact, and preserves fallback.

## Explicit Exclusions

- No orchestrator, lead, research-agent, planner-agent, test-implementer, or specialist productive roles.
- No parallel implementation workers, execution maps, waves, per-worker branches, worktrees, or patch-reduction protocol.
- No productive work by Reviewer or QA.
- No sequential requirement for Reviewer and QA when the same frozen package is available.
- No legacy workflow modification during candidate development or cutover.

## Decisions

- Three roles: Worker, Reviewer, QA.
- Worker owns all productive work and is not a subagent.
- Reviewer and QA remain independent subagents and may run concurrently because they are read-only and idempotent.
- Implement combines tests, code, docs, and in-scope refactoring in serial packets.
- Candidate development uses TDD under legacy governance.
- Candidate acceptance switches the default but leaves legacy untouched; retirement is future work.

<!-- Co-authored by GitHub Copilot. Scope: draft serial three-role TDD workflow plan. Model: MAI Code 2; reasoning-effort setting unavailable. Energy (kWh) and CO2: unavailable; no measured data or defensible estimation inputs available. -->
