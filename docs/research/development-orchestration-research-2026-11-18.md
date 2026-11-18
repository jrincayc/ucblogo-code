# Research Findings: Development Orchestration Workflow

**Status:** Partial
**Researched:** 2026-11-18 (UTC)
**Question:** Do the checked-in orchestration artifacts consistently express the requested flat worker-pool model, and what evidence or uncertainties remain before implementation resumes?
**Scope and assumptions:** Read-only review of the workflow plan, shared contract, relevant role prompts, research handoff format, and current Git state. The workflow contract and plan are treated as the intended requirements; repository text alone cannot establish runtime enforcement or historical approvals.

## Key Findings

- The plan and contract specify a flat model: the orchestrator directly assigns bounded tasks to fresh agents, agents do not delegate, and optional map/reduce scheduling is allowed when useful (E1, E2).
- The contract sets a hard maximum of eight active instances total and a soft concurrent maximum of eight, which can be lowered (E2). The orchestration plan does not state the hard total cap; its four-core constraint concerns CPU-heavy concurrency (E1, E2).
- The role prompts do not yet match the flat model. The Development Orchestrator says to use the Worker only as a fallback when a Lead cannot invoke nested agents and says not to flatten the hierarchy. The Development Lead has an `agent` tool, lists the Worker as a target, and is instructed to delegate to it (E3).
- The orchestration plan's dry-run checklist does not explicitly exercise the hard eight-instance cap or a reduced soft limit, although the contract requires both checks (E4).
- The plan is labeled “Ready for implementation handoff” (E8). That label does not itself document the separate QA and human checkpoint required for each stage (E5); this review does not determine whether approval exists elsewhere.
- Actual host enforcement, availability of required model/effort settings, and direct-assignment capability are unverified. The contract explicitly describes itself as operating policy, not a security boundary (E6).
- The work is nonvisual: the scoped artifacts are workflow documentation and agent prompts, with no planned change to layout, typography, color, imagery, animation, responsive behavior, or visible interaction states. This is a classification for QA verification, not a QA verdict (E9).
- At initial inspection, before creating this report, the worktree was clean at merge commit `d8f36a4` with parents `4b102b3` and `7b7c25e`. This is a time-bound Git observation (E7).

## Evidence Ledger

| ID | Finding | Type | Confidence and basis | Source and date | Planning relevance |
| --- | --- | --- | --- | --- | --- |
| E1 | The plan specifies direct bounded assignments, sequential approval-gated stages, and no agent-to-agent delegation; it does not specify the contract's hard total cap of eight. | documented fact | High; plan wording and concurrency limit inspected. | `/home/runner/work/ucblogo-code/ucblogo-code/docs/plans/development-orchestration-workflow.md:6-15,34` (plan dated 2026-11-17) | Establishes the intended architecture and distinguishes CPU-heavy concurrency from an agent-instance cap. |
| E2 | The contract requires direct assignment by the orchestrator, a hard total cap of eight, prohibits agents from launching or delegating to agents, and allows a lower soft scheduling limit. | documented fact | High; explicit contract text. | `/home/runner/work/ucblogo-code/ucblogo-code/.github/workflows/development-orchestration.md:7-12,55-59` | Sets role and concurrency invariants; distinguish hard total from adjustable soft scheduling limit. |
| E3 | The Orchestrator and Lead prompts authorize nested delegation, contrary to E1 and E2. | direct observation | High; prompt tool/agent metadata and instructions were inspected. | `/home/runner/work/ucblogo-code/ucblogo-code/.github/agents/development-orchestrator.agent.md:6-7,16`; `/home/runner/work/ucblogo-code/ucblogo-code/.github/agents/development-lead.agent.md:3,6-7,15-17` (checked 2026-11-18) | This is the primary consistency issue to resolve before relying on the flat model. |
| E4 | The contract requires a dry-run to check the hard cap and lowered soft limit; the current plan's enumerated dry-run cases omit those explicit checks. | direct observation | High; compared the corresponding checklists. | `/home/runner/work/ucblogo-code/ucblogo-code/.github/workflows/development-orchestration.md:98`; `/home/runner/work/ucblogo-code/ucblogo-code/docs/plans/development-orchestration-workflow.md:40-53` (checked 2026-11-18) | Plan coverage may need to be updated before the dry-run. |
| E5 | Research must report uncertainties and risks, and each stage requires QA plus explicit human approval for the specific revision. | documented fact | High; explicit stage criteria and gate text. | `/home/runner/work/ucblogo-code/ucblogo-code/.github/workflows/development-orchestration.md:17-30` | Repository plan status cannot replace approval evidence; open questions should be surfaced at the Research checkpoint. |
| E6 | Repository policy text cannot demonstrate host-enforced model choice, tool limits, or human gates. | documented fact | High; stated directly by the contract. | `/home/runner/work/ucblogo-code/ucblogo-code/.github/workflows/development-orchestration.md:1-3,11-12` | Requires runtime capability claims to remain explicitly unverified until tested. |
| E7 | Before this report was created, the worktree was clean and HEAD was `d8f36a4`, a two-parent merge commit. | direct observation | High for the recorded inspection; this describes the worktree at that time only. | `git status --short --branch` and `git log -1 --format='%H %P %s'`, 2026-11-18 02:23 UTC; repository `/home/runner/work/ucblogo-code/ucblogo-code` | Identifies the baseline reviewed; it does not describe the later untracked report or establish current remote state. |
| E8 | The plan has a “Ready for implementation handoff” status label; the contract separately requires QA and explicit human approval at every stage. | direct observation | High; both statements are explicit in the cited documents. | `/home/runner/work/ucblogo-code/ucblogo-code/docs/plans/development-orchestration-workflow.md:3`; `/home/runner/work/ucblogo-code/ucblogo-code/.github/workflows/development-orchestration.md:30` | Do not treat the plan's label as evidence of a passed gate. |
| E9 | The planned changes are limited to agent/workflow documentation and static/policy validation; no visual interface work is identified, so visual impact is classified not applicable pending QA verification. | inference | High; scope lists only workflow contracts, prompts, static checks, and policy dry-run. | `/home/runner/work/ucblogo-code/ucblogo-code/docs/plans/development-orchestration-workflow.md:76-77`; visual-impact criteria in `/home/runner/work/ucblogo-code/ucblogo-code/.github/workflows/development-orchestration.md:48-51` | QA should verify this nonvisual classification; any later UI or visible-state scope change reopens the affected gate. |

## Options and Trade-offs

- **Use the checked-in contract and plan as the intended model.** Both support direct, flat assignment and optional map/reduce; the eight-instance hard cap is stated in the contract but not the plan. This option requires resolving contradictory role prompts and exercising runtime capabilities (E1-E3, E6).
- **Treat the role prompts as authoritative instead.** This would retain nested delegation but conflict with the plan and contract; it would require an explicit human decision to revise the intended model (E1-E3).
- **Keep static and runtime claims separate.** Static checks can establish what the repository declares; only a suitable runtime exercise can provide evidence about tool/model availability or host enforcement (E6).

## Conflicts and Counterevidence

- The Orchestrator and Lead prompts are counterevidence to the plan's direct-assignment requirement (E1, E3).
- The shared contract states the direct-assignment model and eight-instance hard cap. The plan's dry-run list does not explicitly test the hard cap or reduced soft limit (E2, E4).
- The plan's handoff label does not itself document a QA verdict or approval for a particular revision (E5, E8). This review does not determine whether approval exists elsewhere.
- The in-scope work is classified as nonvisual, subject to QA verification (E9).

## Constraints and Dependencies

- The required stage order and separate QA/human checkpoint are stated in the contract (E5).
- The plan requires a human-selected small task for its dry-run; selection remains an open item for the human checkpoint (E4).
- The plan identifies two related planning documents for read-only review when they remain changed (E1). Their substantive content was not assessed here.
- No interface or visible-state changes are planned; this Research classification still requires explicit QA verification (E9).

## Unknowns and Unverified Items

- Whether a human approval or QA record exists outside the evidence reviewed here; this report does not establish prior approval (E5, E8).
- Whether the configured host supports direct assignments to all listed roles, exposes the requested GPT-6 Luna/max-effort settings, and enforces each role's tool allowlist (E6).
- Which small task the human selects for the required end-to-end dry-run (E4).
- Whether the two contradictory role prompts previously passed any applicable QA or approval gate. If they did, the contract's escaped-defect process may apply (E5, E8).

## Search Coverage and Limitations

During the initial Research pass, the plan and contract were reviewed and the Orchestrator and Lead prompts inspected for the delegation conflict (E1-E3). Other role prompts and the Research Agent prompt were consulted as context; this report makes no findings about their full contents. The research handoff format was checked against `/home/runner/work/ucblogo-code/ucblogo-code/docs/research/README.md:40-54`. Git status and history were checked (E7). The initial pass created this report and did not modify other files. The report was reopened to add the required nonvisual classification; QA and fresh human approval of this revision are pending (E5, E9).

This review did not execute an orchestration dry-run, validate YAML frontmatter, test host-enforced permissions, run builds or tests, or establish out-of-band approval history. The `docs/plans/logo-inspect-js.md` and `docs/plans/copilot-energy-accounting.prompt.md` documents were only read for scope context; their substantive topics were not independently reviewed.

## Planning Handoff

- The plan and contract support direct assignment, while the Orchestrator and Lead prompts authorize nested delegation (E1-E3); which source should govern the prompts remains a decision for the planning stage.
- The plan's dry-run list omits the hard-cap and reduced-soft-limit cases required by the contract (E4).
- Prior approval provenance and whether the role conflict escaped a passed gate remain unknown (E5, E8).
- The policy dry-run depends on the human selecting a small task (E4).
- Host/model/tool enforcement cannot be established from repository text alone (E6).
- The work is classified as nonvisual; QA verification of that Research classification is still required (E9).

## Sources

- Repository plan and contract paths cited in Evidence Ledger; dates as noted above.
- Repository role prompts cited in Evidence Ledger; inspected 2026-11-18.
- Time-bound Git state from `/home/runner/work/ucblogo-code/ucblogo-code`, inspected 2026-11-18 02:23 UTC (E7).

<!-- Co-authored by GitHub Copilot. Scope: orchestration Research-stage evidence. Model and reasoning-effort metadata unavailable. Partial measured wall-clock for the initial Research pass through its first QA PASS: 19m00s (2026-11-18 02:22:21-02:41:21 UTC); excludes this supplemental revision and QA, later staged/commit verification, and earlier merge work. Energy (kWh) and CO2: unavailable; no measured data or defensible estimation inputs. -->
