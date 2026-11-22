# Commit-Native Accounting — Planning Reopen

- Task ID: `COMMIT-NATIVE-ACCOUNTING-2026-11-21-QA`
- UTC reopen timestamp: `2026-11-22T00:20:41Z`
- Branch: `wasm`
- Base Plan revision: `0b763a9241a0d8a2a48dd8952ca9896df008ca03`
- Current gate: **Plan — reopened/pending**
- Implementation: not started and not authorized

## Andon and Deviation

An independent Plan QA verdict found that the approved Plan did not contain an implementation-ready Parallel Implementation Map. This is an Andon at the Plan gate: implementation remains stopped until the revised Plan receives independent QA and explicit human approval.

The human approved reopening Planning and explicitly requested a compact planning pass using **medium effort** and **medium-effort language**. This means the revised map should be efficient and sufficiently complete, not merely rushed or under-decomposed.

## Scope

- Preserve `docs/plans/commit-native-agent-usage-accounting.md` as the authoritative plan.
- Add the missing implementation-ready Parallel Implementation Map with exact task IDs, file ownership, dependencies, execution waves, focused checks, output contracts, integration owner, independent review owner, commit owner, and serial-only justification.
- Keep all existing plan decisions unless the human later changes scope.
- Do not begin Research, Test Planning, implementation, test edits, code edits, commits, pushes, rebases, or dependency changes in this Planning assignment.
- The human's earlier skip-Research and skip-Replanning deviations remain recorded; this assignment is a bounded Plan repair and refinement only.

## Required Exit

1. The Plan contains a complete acyclic implementation dependency graph and worker-ready packets.
2. Independent Plan QA can verify exact ownership and coverage against the revised revision.
3. No implementation authority is implied by the revised Plan.
4. The next action is independent Plan QA, not Test Planning or implementation.

<!-- Co-authored by GitHub Copilot. Scope: commit-native accounting Planning reopen. Model: MAI Code 2 Flash; reasoning-effort setting unavailable. Energy (kWh) and CO2: unavailable; no measured data or defensible estimation inputs. -->
