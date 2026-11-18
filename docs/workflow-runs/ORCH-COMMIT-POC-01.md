# ORCH-COMMIT-POC-01: Task Evidence

Status: Implementation in progress; post-commit QA and review pending.
Assigned role: Development Lead.
Task type: User-directed orchestration workflow policy edit, outside the ordinary stage gates as specified in the assignment.
Start timestamp: 2026-11-18 15:15:55 UTC (initial scoped worktree check).

## Goal

Update the orchestration contract, role prompts, and workflow plan to authorize task-scoped commits by every role, move independent QA/review after commit, enable bounded POCs, preserve stage approval before advancement, require separate exact-scope push approval, and record durable task evidence.

## Policy decisions

- Every directly assigned role may commit its own task-scoped work after a focused self-check and staged-diff inspection. No prior QA, reviewer, or human approval is required for a local commit.
- QA and applicable code/design review check committed work by exact hash. QA records PASS, FAIL, or BLOCKED and actionable findings against that hash. FAIL/BLOCKED blocks dependent work and stage advancement, not in-scope repair commits by the assigned owner.
- Request human stage approval after relevant commits and post-commit QA/review, before advancement. Request separate exact-scope push approval after commit/QA and before push.
- Prefer a shared task integration branch/worktree with disjoint file ownership and serialized Git-index operations. Avoid concurrent edits to the same file; use isolated worktrees/build directories for overlapping ownership or incompatible build state.
- All roles may construct/test small bounded POCs within assigned scope and isolated state. Use `/tmp/copilot/<task-id>/poc/` for exploratory work, and record path, command, outcome, and limitations. A POC is not product acceptance or stage approval.
- QA/reviewer roles may commit only their own findings/verdict records and separately scoped POC artifacts. They must not modify/fix the artifact under review or approve a POC they authored.

## Owned paths

- `.github/workflows/development-orchestration.md`
- `.github/agents/development-orchestrator.agent.md`
- `.github/agents/development-lead.agent.md`
- `.github/agents/development-worker.agent.md`
- `.github/agents/development-qa.agent.md`
- `.github/agents/development-reviewer.agent.md`
- `.github/agents/development-design-reviewer.agent.md`
- `.github/agents/research.agent.md`
- `docs/plans/development-orchestration-workflow.md`
- `docs/workflow-runs/ORCH-COMMIT-POC-01.md`

The pre-existing untracked dry-run artifact was not opened, modified, staged, or committed. Other unrelated branch changes were not inspected or included.

## Validation

- Shared contract: `git diff --check` and focused text checks passed for post-commit exact-hash QA, owner repair commits, distinct stage/push approval, durable task records, and bounded POCs. Obsolete pre-commit QA and single-commit-owner wording was absent.
- Role prompts: `git diff --check` and focused checks passed for scoped commits, QA/reviewer record boundaries, POC self-approval restrictions, and role-specific tools. The Research Agent's existing assignment-only and network-pause changes were preserved and its focused checks passed.
- Plan: `git diff --check` and focused checks passed for post-commit policy, preserved human-selected dry-run prerequisite, remaining-agent TODO, and owned paths.
- Frontmatter/cross-file validation: `ruby -ryaml -e '<inline assertions>'` passed for all seven roles: YAML parsing, unique names, exact assignment targets, model/effort, contract links, tool/delegation boundaries, Research assignment-only status, scoped commit/POC policy, no pre-commit QA gate, exact push approval, durable task record, and preserved plan TODO/dry-run scope.
- Final `git diff --check -- <explicit owned paths>`: passed for tracked diffs; the untracked task evidence file separately passed its focused check.
- Staged-diff inspection and `git diff --cached --check`: pending.
- Builds/application tests: not run; out of scope for workflow configuration/documentation.

## POC

None needed. This is a policy-documentation edit and no concrete implementation risk requires a proof of concept.
## Commits

- Implementation commit: `8912c9f2345f36527d9611a4812ebbc69b51c910` (`Allow early task commits and bounded POCs`); local commit created 2026-11-18, not pushed.
- Evidence follow-up commit: pending.

## QA, review, and approvals

- Post-commit QA: pending; the parent/orchestrator will assign independent QA against `8912c9f2345f36527d9611a4812ebbc69b51c910`.
- Independent code/configuration review: pending against `8912c9f2345f36527d9611a4812ebbc69b51c910`.
- Stage approval: no stage advancement is requested by this out-of-gate policy task. Any later stage advancement still requires post-commit QA/review and explicit human approval.
- Push: not authorized and not performed. No push approval was requested.

## Next action

Complete static validation and the scoped diff check, commit only the listed paths, then return the exact commit hash and this evidence record to the parent for independent post-commit QA/review. Address required findings in new task-scoped commits. Do not push.

<!-- Co-authored by GitHub Copilot. Scope: ORCH-COMMIT-POC-01 workflow policy and task evidence. -->