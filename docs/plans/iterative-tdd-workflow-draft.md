# Plan: Add Adaptive Iterative Candidate Workflow

Status: Draft for future refinement; not an approved or active workflow change.

Build a self-contained candidate workflow from the proven legacy workflow as a compatibility and policy baseline. Do not modify or retire the legacy workflow. Treat Research, Plan, Test Plan, Implement, and Code Review as evidence-producing concerns connected by an explicit iterative transition graph. Combine test and code implementation into `Implement`, but select implementation and exploration strategies according to task risk and ambiguity. Workers own coherent packets through applicable tests, production code, documentation, and in-scope refactoring.

## Delivery Strategy

- Work on a short-lived candidate branch from an explicitly recorded approved `wasm` base, suggested name `workflow/iterative-tdd-candidate`.
- Confirm the worktree is clean or preserve unrelated changes before creating the branch. No branch, push, merge, or rebase operation occurs during planning.
- Add candidate contract/role artifacts; keep every legacy workflow file untouched during development and cutover.
- Legacy remains the production default until the candidate passes independent QA, dry runs, Code Review, and explicit human acceptance.
- After acceptance, a selection-only change makes the candidate the default. Legacy remains available as fallback.
- Legacy removal, retirement, or consolidation is a future todo requiring a separate plan and approval.
- Task assignments explicitly select exactly one workflow; do not mix stage rules.
- Use legacy behavior as the baseline for gates, QA/review independence, approvals, accounting, attribution, push authorization, evidence identity, Andon, artifact freshness, and process ownership. Copy/adapt those controls into a self-contained candidate rather than depending on the legacy stage contract.

## Bootstrap Strategy

Develop and validate the candidate through the legacy workflow's proven control plane. This is controlled bootstrap use, not a claim that the candidate is already authoritative.

- Legacy remains authoritative for Research, Plan, Test Plan, QA/review, human approvals, Andon, evidence, accounting, and push authorization during candidate construction.
- Candidate packets use TDD: add a failing policy/static/dry-run check, make the smallest candidate change, rerun, refactor while green, and repeat.
- This test-first mandate applies only to constructing and validating the bootstrap candidate. Workers may not change implementation strategy on the fly; strategy changes require planning or explicit human direction.
- Candidate exercises begin as simulations/dry runs. Label them pilot/simulated evidence; do not imply candidate approvals or production authority.
- A minimum validated candidate slice may govern only later work within its explicitly tested scope. All unproven behavior falls back to legacy.
- A failed or unsafe bootstrap returns to legacy without blocking `wasm` or the legacy default.
- Candidate acceptance remains legacy-governed; cutover then switches the default while legacy remains untouched.

## Effort Estimate

- Phase 1: 1–1.75 hours implementation plus 20–35 minutes validation.
- Phase 2: 45–75 minutes plus 20–30 minutes review/validation.
- Phase 3: 45–75 minutes plus 20–30 minutes review/validation.
- Phase 4: 1–2 hours plus 30–45 minutes dry-run.
- Phase 5: 2–4 hours evidence/dry runs, 30–60 minutes acceptance, 15–30 minutes cutover.
- Bootstrap work adds 30–60 minutes but reduces migration/recovery risk.
- Total: approximately 6–11 hours wall clock. Work can stop safely after any completed phase. Monetary cost is unavailable without current measured consumption/rates.

## Cost/Benefit Phasing

### Phase 1 — Candidate Foundation

1. Create and record the candidate branch and exact `wasm` base.
2. Add a separate candidate contract with `Research -> Plan -> Test Plan -> Implement -> Code Review`; leave legacy unchanged.
3. Implement the candidate TDD red-green-refactor cycle, three-strike guardrail, worker-owned tests/harnesses/docs/code, and coherent packet handoff.
4. Add minimum candidate role/configuration and static validation without making it default.

### Phase 2 — Shared Infrastructure And Packet Ownership

Add shared harness prerequisites, active file/symbol/fixture/doc ownership, temporary worker branches, patch-only integration, in-packet documentation duties, and estimate-based timeouts capped at 30 minutes.

### Phase 3 — Candidate QA/Review Quality

Add candidate reviewer and QA criteria for tests, docs, refactors, branch/patch identity, correctness, cohesion, coupling, compatibility, and final evidence. Leave legacy roles untouched.

### Phase 4 — Cross-Worker Coordination And Andon

Add detection, one bounded consensus/counterproposal round, one designated writer, affected-line pauses, and scoped Andon for unresolved production/test refactors.

### Phase 5 — Acceptance And Cutover

Dry-run candidate behavior, complete independent acceptance through legacy governance, switch the default by selection-only change, verify legacy artifacts unchanged, and record legacy retirement as future work.

## Detailed Steps

1. Add a self-contained candidate contract with an explicit transition graph, adaptive profiles, selectable implementation strategies, issue counters, Andon triage, structured questions, and post-edit formatting.
2. Add separate workflow-independent pool contracts and tunable defaults rather than fixed model/effort/concurrency in workflow text.
3. Define worker-local exploration for low-risk packets and centralized Research spikes for high-risk uncertainty.
4. Keep bootstrap implementation test-first. Do not expose alternative implementation strategies as current bootstrap choices; defer broader strategy selection to a later approved phase.
5. Make workers own packet-local tests, harnesses, production changes, relevant documentation, and in-scope refactoring. Research/Plan/Test Plan identify documentation obligations, and review/QA verify them.
6. Define coherent feature-like or bug-fix packets with exact files/symbols, behavior, acceptance criteria, tests, docs, checks, dependencies, effort-derived timebox, branch/worktree, outputs, and owners.
7. Plan shared harnesses and fixtures as one-owner prerequisite tasks; track file, symbol, fixture, harness, and documentation ownership.
8. Use a uniquely named temporary branch/worktree for each worker from the exact assigned base. Workers may make small reviewed commits but shall not push, merge, fast-forward, rebase, or advance the target branch.
9. Integrate reviewed patches or commit ranges as new integration commits. Mechanical patch conflicts may be resolved; behavioral, scope, or dependency conflicts follow Andon.
10. Derive packet timeboxes with `min(estimated effort × 1.5, 30 minutes)` and a halfway checkpoint. Oversized packets are decomposed or BLOCKED; the earlier limit triggers Andon without a handoff.
11. Preserve three-strikes: after an unexpected failure or broken build, no more than three focused fix-and-check attempts. The third failure preserves evidence and raises Andon. Expected red is not a strike.
12. Cross-worker refactors use ownership detection, affected-line pauses, one bounded proposal/counterproposal round, one writer, and scoped Andon when consensus or scope remains unresolved.
13. Behavior-preserving production/test refactors stay within Implement; broader API, architecture, ownership, dependency, acceptance, or output changes reopen Plan.
14. Develop the candidate itself test-first under legacy governance: add a failing policy/static/dry-run check, make the smallest candidate change, rerun, refactor while green, and label unaccepted results pilot/simulated.
15. Add candidate orchestrator/worker/reviewer/QA roles and any other candidate role needed for unambiguous self-contained operation.
16. At cutover, change only default/workflow selection and verify every legacy artifact remains byte-for-byte unchanged. Legacy retirement remains a separate future plan.

## Candidate Files To Add Or Evaluate

- `.github/workflows/development-orchestration-candidate.md` — self-contained candidate contract.
- `.github/agents/development-orchestrator-candidate.agent.md` — candidate scheduling, ownership, branches, timeouts, and cutover-safe coordination.
- `.github/agents/development-worker-candidate.agent.md` — TDD, docs/tests/harness ownership, three strikes, branches, and timeouts.
- `.github/agents/development-reviewer-candidate.agent.md` and `.github/agents/development-qa-candidate.agent.md` — candidate-specific review/QA.
- Candidate lead/design/research roles only if existing roles cannot be reused safely; otherwise document explicit reuse through the candidate orchestrator.
- Candidate plan/test-plan/run/evidence artifacts under existing repository conventions.
- A minimal workflow selector, designed during implementation and activated only after acceptance.

## Legacy Baseline References — Not Modification Targets

- `.github/workflows/development-orchestration.md`
- `.github/agents/development-*.agent.md`
- `.github/copilot-instructions.md`
- `docs/plans/development-orchestration-workflow.md`

## Verification / QA Checklist

1. Workflow and pool boundaries are self-contained and distinct.
2. Legacy workflow files remain unchanged through development and acceptance; cutover changes selection only.
3. Lean, balanced, and risk-first behavior is tunable without workflow edits or deviations.
4. Generic pool model, effort, concurrency, and locality settings are independently configurable.
5. Legitimate re-entry is recorded as rework, not deviation; actual exceptions remain deviations; defects/systemic blockers remain Andon.
6. Third failed issue iteration triggers Andon and automatic read-only QA triage, with no fourth attempt before explicit resolution.
7. `askQuestions` behavior matches repository instructions, including mandatory `ANDON`/`Other` and approval choices.
8. Formatting happens immediately after edits, before checks, and is never performed by a mutating pre-commit hook.
9. Expected-red and inconclusive exploration do not count as strikes.
10. Both centralized and worker-local exploration remain bounded and evidence-producing.
11. Plan and Test Plan remain separate; Implement combines applicable tests, code, docs, and local refactoring.
12. Temporary branches, patch-only integration, and conflict/Andon rules remain consistent.
13. Acceptance/cutover leaves legacy intact; retirement remains future work.
14. Estimates, candidate paths, ownership, dependencies, and validation are actionable and consistent.

## Dry Runs

1. Failing policy check -> minimal candidate edit -> green -> refactor -> green.
2. Valid expected-red test -> smallest production/docs change -> green -> refactor.
3. Worker-owned documentation update and documentation-only packet.
4. Temporary branch with patch-only integration; no merge, fast-forward, or rebase.
5. Shared harness prerequisite and dependent packets.
6. Three-strike broken-build Andon and affected-line recovery.
7. Effort-derived timeout and 30-minute global cap.
8. Cross-worker production/test refactor consensus and failed-consensus Andon.
9. Explicit candidate/legacy selection and fallback after an unsafe candidate dry run.

## Decisions

- Stages are evidence concerns in an iterative transition graph, not a mandatory straight line.
- TDD is mandatory for candidate bootstrap; broader execution strategies are deferred to a later approved phase.
- Workers may not change bootstrap strategy on the fly; only planning or explicit human direction can authorize a change.
- The worker pool is generic and below the workflow; roles, models, effort, and parallelism are configurable.
- Candidate-compliant iteration is rework in the transition log, not deviation.
- Third failed iteration triggers Andon and automatic low-effort read-only QA triage.
- Structured questions and immediate post-edit formatting are explicit requirements.

<!-- Co-authored by GitHub Copilot. Scope: adaptive iterative candidate workflow plan. Model: MAI Code 2; reasoning-effort setting unavailable. Energy (kWh) and CO2e: unavailable; no measured data or defensible estimation inputs available. -->
