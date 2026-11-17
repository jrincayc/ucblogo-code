# Plan: Development Orchestration Workflow

Status: Ready for implementation handoff. Reconcile the existing workspace agents with the shared contract; preserve valid work and do not replace files wholesale.
Date: 2026-11-17

## Goal

Provide a fast, three-level software-development workflow that catches defects at the earliest useful check, supports ordinary implementation iteration, and prevents defects that escaped a gate from moving farther downstream. Keep the workflow usable under slow or interrupted networking and the user's stated workstation limits.

## Current Surfaces

- Shared contract: `.github/workflows/development-orchestration.md`.
- Workspace roles: `.github/agents/development-orchestrator.agent.md`, `development-lead.agent.md`, `development-worker.agent.md`, `development-qa.agent.md`, `development-reviewer.agent.md`, and `development-design-reviewer.agent.md`.
- Research role: `.github/agents/research.agent.md`.
- Review, without editing, the related changed planning artifacts identified at handoff, including `docs/plans/logo-inspect-js.md` and `docs/plans/copilot-energy-accounting.prompt.md` when they remain changed. Report findings separately from orchestration implementation.
- Inspect the live worktree before editing. Preserve unrelated user changes, and use `/tmp/copilot/<task-id>/` for temporary state.

## Workflow Requirements

1. Use the sequential stages Research -> Plan -> Test Plan -> Implement Tests -> Implement Changes -> Code Review. Each stage has a defined deliverable, independent QA verdict, and explicit human approval before advancement. A failed gate blocks advancement but can be repaired within its approved scope.
2. In Test Plan, prepare a handoff-ready specification for test implementers: acceptance criteria, cases and scenarios, inputs and expected results, environment, setup/teardown, focused commands, regression scope, and relevant negative cases. Create compact, deterministic mock/fixture data, builders, or seeds when useful; otherwise explain why fixtures are unnecessary. Keep fixtures local and free of secrets or unnecessary sensitive data. Test implementers follow the approved specification and raise coverage changes for approval.
3. During Research, classify visual impact and have QA verify any not-applicable decision. For visual work, `Development Design Reviewer` reviews proposed screens/mockups and affected states before Plan approval and advancement to Test Plan; QA verifies the exact artifacts and criteria coverage, and the human approves. After implementation, the reviewer inspects actual rendered output and interactions at agreed desktop/mobile viewports and relevant states before advancement to Code Review; QA verifies artifact/revision identity and criteria coverage, and the human accepts. Missing visual evidence blocks advancement. A nonvisual task records a QA-verified not-applicable result.
4. Preserve normal iteration: a failed sketch, expected-red test, or unsuccessful focused implementation attempt is not by itself a workflow-wide stop. Diagnose, record, and retry within the current approved stage. After three consecutive unsuccessful focused attempts on the same issue, escalate and stop that affected workflow line until the lead/orchestrator authorizes the next action. An issue at the current, not-yet-passed gate blocks that item from advancing; independent work may continue when unaffected.
5. Apply stop-the-line when a defect is found in an artifact, decision, test, or commit that already passed an applicable gate. Stop affected downstream work, report the escaped defect, reopen the relevant gate, and invalidate dependent approvals. Treat systemic or escalated issues as stopping their affected lines; do not confuse ordinary iteration with escalation.
6. After each semantic block, run the focused check and obtain the required independent review and QA PASS before proceeding. Before committing, stage only that cleared block, inspect the exact staged diff, and obtain the required review/QA check; after the commit, QA verifies its hash and diff before further work. Keep commits small and frequent. A preference for early, frequent pushes is not standing authorization: **every push requires fresh explicit human approval for the exact remote, destination branch, and commits/range**. No push occurs while that approval is absent or ambiguous. Do not rewrite pushed history.

## Delegation, Network, and Resources

- Use GPT-6 Luna at maximum supported reasoning effort for every role unless the user specifies or approves an alternative. Disclose unavailable settings and never silently substitute. The exact delegation prompt must request maximum reasoning effort and include scope, ownership, acceptance criteria, checks, return format, and applicable repository instructions.
- Prefer approved cloud-hosted agents for eligible independent research, planning, test, implementation, and review work. Keep local-only runtime, browser/device, or hardware checks local when cloud agents cannot access the same environment. Transfer only necessary prompts, diffs, and compact test artifacts; reuse cached data and avoid duplicate uploads. Never transmit secrets to unapproved services. Do not claim cloud delegation ran unless it did.
- On a detected network interruption, pause all task work, local and cloud, at a safe checkpoint and preserve in-progress state. Resume only when passive recovery is indicated, a necessary ordinary network operation succeeds, or the user confirms recovery. Do not generate traffic solely to test connectivity. Record the pause, recovery signal, and resume point; do not claim remote checks passed while unavailable. During ordinary slowness, avoid unnecessary requests and duplicate transfers.
- User-provided workstation profile: quad-core Intel i7, 32 GB RAM, 1 TB disk. Treat these as total capacities, not available headroom. Check current CPU, free memory, and disk for heavy work when observable; cap CPU-heavy concurrency at four cores and reduce it to preserve responsiveness or when resources are constrained. If resource data is unavailable, state that and schedule conservatively.

## Implementation and Acceptance

1. Compare every role prompt with the shared contract and make only the smallest consistency edits. Ensure the orchestrator can invoke the Design Reviewer; keep worker roles unable to delegate, and keep QA/design/code review independent of the artifacts they assess. Preserve role-specific tool boundaries and state which restrictions are procedural versus host-enforced.
2. Validate agent frontmatter with an available local YAML parser; check unique names, model/effort declarations, delegation targets, contract links, and role tool boundaries. Do not install dependencies just for validation.
3. Validate contract wording and delegation templates for the six-stage order, test-spec and fixture handoff, exact design-review checkpoints, normal iteration versus escalation versus escaped-defect stop, per-substep and commit review/QA checks, cloud preference/minimal transfer, network-interruption pause and recovery signal, workstation limits, temporary-file policy, and exact-scope human approval before every push.
4. Dry-run the policy on a human-selected small task without pushing, covering:
	- All six stages, with independent QA and explicit human approval before each advancement.
	- A normal failed attempt followed by success; a current-gate QA rejection that blocks advancement but permits in-scope repair; and an expected-red test that fails for the intended behavior.
	- Three consecutive failed attempts escalating; a defect discovered after a passed gate stopping affected downstream work; and a scope change invalidating affected approvals.
	- An unavailable model prompting a stop and approval request, and unsupported nested delegation falling back without losing lead accountability.
	- Both visual checkpoints: proposed-artifact review before Plan approval/advancement to Test Plan, and rendered-output review after implementation before Code Review, with QA and human approval at each applicable gate.
	- A test-plan handoff with useful compact mock/fixture data, or a documented reason fixtures are unnecessary.
	- Per-substep review and QA, exact staged-diff checks before commit, and post-commit QA verification of the commit hash and diff before further work.
	- Cloud delegation with minimal transfer, or an honest report that it is unavailable; a network interruption pausing local/cloud work and recovery from the checkpoint only after passive recovery, a necessary ordinary operation, or user confirmation; and constrained resources reducing concurrency.
	- A proposed push remaining blocked until fresh approval names the exact remote, branch, and commit range.
5. Independently review the implementation and changed-plan review results. Obtain QA and human approval at every stage. Report runtime capabilities that remain unverified; static validation alone does not demonstrate host-enforced gates or cloud execution.

## Scope and Safety

- Included: workspace orchestration agent prompts, their shared workflow contract, static and policy dry-run validation, and a read-only review of related changed planning documents.
- Excluded: application/interpreter changes, builds, production deployment, unrequested changes to related plans, and any push without fresh exact-scope human approval.
- Commit only task-owned files after the required checkpoints. Do not include existing unrelated worktree changes. Keep task-owned temporary files under `/tmp/copilot` and clean up only those files.

<!-- Co-authored by GitHub Copilot. Scope: development-orchestration workflow plan. Model and reasoning-effort metadata unavailable. Energy (kWh) and CO2: unavailable; no measured data or defensible estimation inputs. -->