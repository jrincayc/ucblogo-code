---
description: "Use approved task-scoped combined-TDD deviation prompts for Implement Changes with isolated Worker worktrees."
name: "Combined TDD Deviation"
argument-hint: "Provide task ID, approved revisions, paths, checks, base revision, and integration owner."
agent: "agent"
---
# Combined-TDD Deviation Prompts

Use these templates only for a task after the human explicitly approves the task-scoped deviation. Replace every `<...>` placeholder. Do not treat the templates as standing approval.

## 1. Human Approval Prompt

```text
For task <TASK-ID>, I explicitly approve this task-scoped workflow deviation.

Rationale: front-load and evaluate worker-owned TDD without changing repository policy.

Scope and limits:
- Begin only after the exact approved Research, Plan, and Test Plan revisions are identified.
- Do not waive Research, Plan, Test Plan, their required QA, or their human approvals.
- Skip Implement Tests only for this task.
- Record: "Implement Tests: not run under approved deviation"; never claim it passed.
- During Implement Changes, the assigned Worker owns tests, production changes, relevant documentation, packet-local fixtures/harnesses, validation, and behavior-preserving refactoring.
- Use red -> green -> refactor, with focused checks after every substantive edit.
- Preserve three-attempt Andon, artifact freshness, exact revision identity, independent Reviewer coverage, aggregate Development QA, the normal Implement Changes human checkpoint, Code Review, and final QA/sign-off.
- This deviation changes no commit ownership and does not authorize a push.
- The orchestrator must create and record one private task worktree per active Worker from the exact base revision. If isolated state is unavailable, mark the task BLOCKED.
- Record this approval, rationale, limits, affected task/revisions, private worktree, and skipped stage in the task/workflow record.

This approval applies only to task <TASK-ID>. No shorthand or earlier approval applies to another task.
```

## 2. Worker Assignment Prompt

```text
Implementation mode for task <TASK-ID>: approved combined-TDD deviation.

Approved Test Plan/revision: <TEST-PLAN-REVISION>
Base revision: <BASE-REVISION>
Private task worktree: <PRIVATE-WORKTREE-PATH>
Allowed files: <ALLOWED-FILES>
Focused checks: <FOCUSED-COMMANDS>
Regression checks: <REGRESSION-COMMANDS>
Commit/integration owner: <OWNER>

Workspace isolation:
- Work only in the assigned private task worktree at <PRIVATE-WORKTREE-PATH>.
- Do not read, edit, stage, commit, build, reconfigure, or clean in the shared user checkout.
- Do not modify another task's worktree.
- Keep temporary files under /tmp/copilot/<TASK-ID>/.
- Report the worktree path, base revision, final revision, changed paths, validation commands/results, and commit IDs or uncommitted patch.
- Do not merge, push, rebase, or integrate. Return evidence and the reviewed patch/commit range to the integration owner.
- Preserve the private worktree until the integration owner confirms its output was integrated or explicitly authorizes cleanup.

You own this coherent packet end-to-end, including tests, packet-local fixtures/harnesses, production changes, relevant documentation, refactoring, and validation.

1. Select the smallest approved falsifiable behavior.
2. Add the smallest test.
3. Run it and preserve evidence of the intended red result.
4. Make the smallest production change.
5. Reach green.
6. Refactor while checks remain green.
7. Rerun focused checks after every substantive edit.
8. Update relevant documentation or record an explicit N/A.
9. Run the approved regression checks.
10. Return the complete diff, commands/results, exact revision, blockers, risks, and evidence.

Expected red must arise from missing behavior. Setup, syntax, stale-artifact, and infrastructure failures are not valid red results. After three consecutive unsuccessful fix-and-check attempts on the same unexpected failure, stop, preserve evidence, and raise Andon; do not make a fourth attempt.

Do not self-approve, advance to Code Review, commit outside assigned authority, or push. Independent Development Reviewer and aggregate Development QA checks remain required.
```

<!-- Co-authored by GitHub Copilot. Scope: QA-approved task-scoped combined-TDD prompt templates. Model: MAI Code 2; reasoning-effort setting unavailable. Energy (kWh) and CO2: unavailable; no measured data or defensible estimation inputs available. -->
