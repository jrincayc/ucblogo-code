# Research Findings: Development Orchestration Workflow

**Status:** Partial
**Researched:** 2026-11-18 (UTC)
**Question:** Do the orchestration artifacts express the requested flat worker-pool model, and what remains unverified?
**Scope and assumptions:** Review of the workflow plan, shared contract, role prompts, research handoff format, and Git state. The user explicitly directed this orchestration-tooling work to proceed outside its normal six-stage workflow; this task-specific exception does not revise the workflow intended for future work. Repository text alone cannot establish runtime enforcement.

## Key Findings

- The plan and shared contract specify flat direct assignment, no agent-to-agent delegation, and optional map/reduce scheduling (E1, E2).
- The contract requires a hard cap of eight active instances total and permits a lower soft concurrency limit. The current plan explicitly tests rejecting a ninth assignment and deferring work above a reduced soft limit (E2, E4).
- At baseline commit `0694958`, the Orchestrator and Lead prompts contradicted the flat model. The current prompt edits remove nested-delegation authority, make the Orchestrator the sole assignment authority, and route follow-on task proposals back to it (E3).
- The user explicitly asked that this tooling work proceed outside the normal workflow. The plan status now records this task-specific exception; it does not claim QA or approval of a workflow stage (E8).
- Static validation passed for all seven agent prompts: YAML frontmatter, unique names, explicit model/effort, valid contract links and direct-assignment targets, and no agent-launch capability outside the Orchestrator (E10).
- The work is limited to agent/workflow documentation and static validation, with no planned application or visual-interface changes (E9).
- Host enforcement and actual availability of the requested model, effort, and direct-assignment capabilities remain unverified (E6).
- The original Research inspection was made at merge commit `d8f36a4`; that is a historical observation, not the current repository state (E7).

## Evidence Ledger

| ID | Finding | Type | Confidence and basis | Source and date | Planning relevance |
| --- | --- | --- | --- | --- | --- |
| E1 | The plan specifies sequential stages, direct bounded assignments, and no agent-to-agent delegation. | documented fact | High; explicit plan wording. | `/home/runner/work/ucblogo-code/ucblogo-code/docs/plans/development-orchestration-workflow.md:6-15` (plan dated 2026-11-17) | States the intended workflow model. |
| E2 | The contract requires direct assignment by the orchestrator, a hard total cap of eight, no agent-to-agent delegation, and a lowerable soft scheduling limit; map/reduce is optional. | documented fact | High; explicit contract text. | `/home/runner/work/ucblogo-code/ucblogo-code/.github/workflows/development-orchestration.md:7-12,55-59` | Separates the hard total limit from adjustable scheduling and CPU-heavy limits. |
| E3 | At baseline commit `0694958`, the Orchestrator and Lead prompts authorized nested delegation. Current edits make the Orchestrator the sole assignment authority and remove the Lead's agent-launch tool and target. | direct observation | High; compared baseline prompt contents with the current worktree. | Baseline: `git show 0694958:.github/agents/development-orchestrator.agent.md` and `git show 0694958:.github/agents/development-lead.agent.md`; current: `/home/runner/work/ucblogo-code/ucblogo-code/.github/agents/development-orchestrator.agent.md:16-18` and `/home/runner/work/ucblogo-code/ucblogo-code/.github/agents/development-lead.agent.md:6-17` (2026-11-18) | The static prompt changes resolve the identified delegation conflict; validation is reported separately. |
| E4 | The current plan's dry-run explicitly rejects a ninth active instance and checks a lowered soft limit while retaining the hard cap. | direct observation | High; both checks appear in the checklist and acceptance criteria. | `/home/runner/work/ucblogo-code/ucblogo-code/docs/plans/development-orchestration-workflow.md:41-43,63` (checked 2026-11-18) | The earlier checklist gap is closed; this does not demonstrate runtime enforcement. |
| E5 | The intended workflow requires independent QA and explicit human approval at each stage. | documented fact | High; explicit contract text. | `/home/runner/work/ucblogo-code/ucblogo-code/.github/workflows/development-orchestration.md:17-30` | These gates remain part of the future workflow; they were not used to gate this user-exempt tooling task. |
| E6 | Repository policy text cannot demonstrate host-enforced model choice, tool limits, or human gates. | documented fact | High; stated directly by the contract. | `/home/runner/work/ucblogo-code/ucblogo-code/.github/workflows/development-orchestration.md:1-3,11-12` | Requires runtime capability claims to remain explicitly unverified until tested. |
| E7 | Before this report was created, the worktree was clean and HEAD was `d8f36a4`, a two-parent merge commit. | direct observation | High for the recorded inspection; this describes the worktree at that time only. | `git status --short --branch` and `git log -1 --format='%H %P %s'`, 2026-11-18 02:23 UTC; repository `/home/runner/work/ucblogo-code/ucblogo-code` | Identifies the baseline reviewed; it does not describe the later untracked report or establish current remote state. |
| E8 | The user explicitly directed this orchestration-tooling work to proceed outside the normal workflow. The plan records this as a task-specific exception, not as QA or stage approval. | direct observation | High; explicit user direction and plan status. | User instruction, 2026-11-18 03:40 UTC; `/home/runner/work/ucblogo-code/ucblogo-code/docs/plans/development-orchestration-workflow.md:3` | Applies only to this tooling task and does not change the intended gates for future work. |
| E9 | The planned changes affect agent/workflow documentation and static policy validation, not application code or a visible interface. | inference | High; the included and excluded scopes identify no UI or rendered-state work. | `/home/runner/work/ucblogo-code/ucblogo-code/docs/plans/development-orchestration-workflow.md:74-78` | Records the nonvisual scope; no visual review is implied. |
| E10 | Static validation passed for seven role prompts, including YAML parsing, unique role names, explicit model/effort, valid contract links and assignment targets, and the absence of agent-launch tools or child targets on non-Orchestrator roles. `git diff --check` also passed. | direct observation | High; all checks completed without errors. | Local validation command, 2026-11-18 03:49 UTC; `/home/runner/work/ucblogo-code/ucblogo-code/.github/agents/*.agent.md` and `/home/runner/work/ucblogo-code/ucblogo-code/docs/plans/development-orchestration-workflow.md` | Establishes static consistency only; it does not establish host enforcement or perform the policy dry-run. |

## Options and Trade-offs

- **Use the plan and contract as the intended model.** They specify direct assignment, optional map/reduce, and the hard eight-instance cap; the prompt edits bring the roles into alignment (E1-E4).
- **Keep static and runtime claims separate.** Prompt and YAML checks can establish what the repository declares; only a suitable host-level exercise can establish runtime availability or enforcement (E6).

## Conflicts and Counterevidence

- At baseline, the Orchestrator and Lead prompts contradicted the flat-assignment requirement; the current edits address that static conflict (E1-E3).
- The original plan checklist omitted explicit cap checks; the current plan now includes them (E2, E4).
- Repository policy still cannot prove runtime tool/model enforcement or that a ninth assignment would actually be blocked (E6).
- The current task-specific workflow override does not remove or revise the gates specified for future work (E5, E8).

## Constraints and Dependencies

- The six-stage order and separate QA/human checkpoints remain part of the future workflow (E5); this task-specific exception was explicitly requested by the user (E8).
- The plan's end-to-end policy dry-run was not run; it requires a selected task and runtime conditions not established here (E4, E6).
- The plan identifies two related planning documents for read-only review when they remain changed. Their substantive content was not assessed here.
- No application or visible-interface changes are in scope (E9).

## Unknowns and Unverified Items

- Whether the configured host supports direct assignments to all listed roles, exposes the requested GPT-6 Luna/max-effort settings, and enforces each role's tool allowlist (E6).
- Whether the host enforces the eight-instance hard cap and any lower soft limit; static text does not demonstrate either behavior (E2, E6).
- No end-to-end policy dry-run was performed for this user-directed out-of-workflow task (E4, E8).

## Search Coverage and Limitations

The plan, shared contract, all development-role prompts, Research Agent prompt, and research handoff format were inspected. The initial Research pass identified the Orchestrator/Lead conflict; the subsequent prompt edits and plan updates are compared with baseline commit `0694958` (E1-E4). The handoff format was checked against `/home/runner/work/ucblogo-code/ucblogo-code/docs/research/README.md:40-54`. Git state was checked at the initial baseline and again during this continuation (E7). The user explicitly requested bypassing the normal workflow for this meta-work (E8).

YAML/frontmatter and prompt-consistency checks passed for all seven role prompts (E10). No end-to-end orchestration dry-run, host-enforcement test, application build, or application test was performed. The `docs/plans/logo-inspect-js.md` and `docs/plans/copilot-energy-accounting.prompt.md` documents were read only for scope context; their substantive topics were not reviewed.

## Planning Handoff

- The static artifacts now express direct assignment, no agent-to-agent delegation, optional map/reduce, and the hard eight-instance cap (E1-E4).
- Static checks verify the role metadata and direct-assignment targets, but do not prove that the host enforces them (E6, E10).
- Actual host-level capability and enforcement remain unknown and cannot be established from repository text alone (E6).
- This work's user-authorized bypass applies only to the current orchestration-tooling task; it is not evidence that any future stage gate passed (E5, E8).

## Sources

- Repository plan and contract paths cited in Evidence Ledger; dates as noted above.
- Repository role prompts cited in Evidence Ledger; inspected 2026-11-18.
- Time-bound Git state from `/home/runner/work/ucblogo-code/ucblogo-code`, inspected 2026-11-18 02:23 UTC (E7).
- User direction to continue this orchestration-tooling work outside the normal workflow, 2026-11-18 03:40 UTC (E8).

<!-- Co-authored by GitHub Copilot. Scope: orchestration Research-stage evidence. Model and reasoning-effort metadata unavailable. Initial Research pass through first QA PASS: 19m00s (2026-11-18 02:22:21-02:41:21 UTC). Current user-directed prompt/plan update: partial measured interval from first recorded timestamp through static validation, 8m04s (2026-11-18 03:41:35-03:49:39 UTC); excludes secret scanning, commit, CodeQL, and final verification; no subagents used. Energy (kWh) and CO2: unavailable; no measured data or defensible estimation inputs. -->
