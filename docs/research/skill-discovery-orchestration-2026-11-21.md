# Research Findings: Skill Discovery in Development Orchestration

**Status:** Complete
**Researched:** 2026-11-21 (UTC)
**Question:** How should Research identify likely skills and how should Plan map them to tasks and worker prompts, while keeping selection useful and non-prescriptive?
**Scope and assumptions:** Reviewed only the approved shared contract, handoff format, orchestrator/research/worker agent policies, repository instructions, and repository/local skill metadata. No external sources were used. This report is evidence for planning, not an implementation plan or approval of later stages.

## Key Findings

1. Research has an explicit report format and output location, but no requirement or field for likely skills. The report format requires evidence, planning handoff, constraints, and unknowns ([`docs/research/README.md` lines 1-51](../../docs/research/README.md)).
2. The shared Plan contract requires an implementation-ready Parallel Implementation Map with task IDs, ownership, dependencies, readiness, checks, output contracts, and integration/review/commit owners ([`.github/workflows/development-orchestration.md` lines 22-28](../../.github/workflows/development-orchestration.md)); it does not currently require a skill column or field.
3. Every direct-assignment prompt must include stage, task, owner, approved inputs, allowed files, dependencies, acceptance criteria, validation commands, prohibited actions, contract/instruction/skill paths, model/effort, stop-line rules, and return format ([`.github/workflows/development-orchestration.md` lines 82-145](../../.github/workflows/development-orchestration.md)). The contract's skill reference is generic rather than a candidate-skill mapping requirement.
4. The Orchestrator is told to include full delegation requirements and relevant repository instructions/skills in prompts ([`.github/agents/development-orchestrator.agent.md` lines 15-45](../../.github/agents/development-orchestrator.agent.md)); the worker is told to load task-relevant skills ([`.github/agents/development-worker.agent.md` lines 10-35](../../.github/agents/development-worker.agent.md)). Neither agent currently tells the Research phase to emit or verify likely skills.
5. The narrowest discovery surface is skill frontmatter: `name` and a trigger-oriented `description`, not the full skill body (host-provided `agent-customization` skill at `/usr/share/code/resources/app/extensions/copilot/assets/prompts/skills/agent-customization/SKILL.md`, lines 1-4 and 77-81; no repository copy exists at `.github/skills/agent-customization/SKILL.md`). Repository skill descriptions are exposed as catalog metadata, enabling candidate filtering before loading full skill contents.
6. This policy task most plausibly applies to `agent-customization`; shell-only discovery or execution would only be supporting activity and does not make `parallel-xargs` intrinsically applicable. Accounting, Logo implementation, builds, web frontend, browser testing, and turtle graphics lack task-domain evidence and should be excluded unless future task decomposition introduces them.

## Evidence Ledger

| ID | Finding | Type | Confidence and basis | Source and date | Planning relevance |
| --- | --- | --- | --- | --- | --- |
| E1 | Research reports are evidence records for planning, not plans, and use a dated report directory plus a prescribed section structure. | Documented fact; direct structure match. High. | [`docs/research/README.md` lines 1-51](../../docs/research/README.md), 2026-11-21. | Decide whether likely-skill evidence should be an added report section or a clearly labeled field in Planning Handoff. |
| E2 | Plan requires task/dependency/map fields but no skill field or mapping. | Documented fact. High. | [`.github/workflows/development-orchestration.md` lines 22-28](../../.github/workflows/development-orchestration.md), 2026-11-21. | A skill mapping can remain additive; avoid treating it as a hidden acceptance criterion unless policy is revised. |
| E3 | Direct worker prompts require relevant skills/instructions, but permit only a generic skill reference and do not require candidate discovery or rationale. | Documented fact. High. | [`.github/workflows/development-orchestration.md` lines 82-145](../../.github/workflows/development-orchestration.md); [`.github/agents/development-worker.agent.md` lines 10-35](../../.github/agents/development-worker.agent.md). | The policy gap is between a generic requirement and an explicit task-to-skill suggestion. |
| E4 | Orchestrator is responsible for relevant skill and instruction selection in prompts. | Documented fact. High. | [`.github/agents/development-orchestrator.agent.md` lines 15-45](../../.github/agents/development-orchestrator.agent.md). | Skill mapping must remain orchestrator-owned rather than making workers independently choose unrestricted capabilities. |
| E5 | Repository skill descriptions are the discovery surface: frontmatter `name`/`description` should trigger loading. | Documented fact. High. | Host-provided `agent-customization` skill at `/usr/share/code/resources/app/extensions/copilot/assets/prompts/skills/agent-customization/SKILL.md`, lines 77-81; repository `SKILL.md` frontmatter observations. | A narrow catalog scan of names/descriptions is sufficient for candidates; full skill bodies can be loaded only after relevance is inferred. |
| E6 | `agent-customization` explicitly covers VS Code agent customization files and workflow primitives. | Direct observation from skill metadata. High. | Host-provided `agent-customization` skill at `/usr/share/code/resources/app/extensions/copilot/assets/prompts/skills/agent-customization/SKILL.md`, lines 1-19. | Strong candidate for contract, README, and agent-policy edits or review. |
| E7 | Repository instructions require loading applicable skills and naming them in the initial update. | Documented fact. High. | [`.github/copilot-instructions.md` lines 1-4](../../.github/copilot-instructions.md). | Supports a research report's likely-skill section and worker prompt attribution. |
| E8 | Candidate policy files are `.github/workflows/development-orchestration.md`, `.github/agents/*.agent.md`, and `.github/copilot-instructions.md`; no Logo runtime or web UI change is in the approved goal. | Direct observation/inference. High for scope, Medium for exclusions. | Approved task inputs and workspace file names. | Excludes domain-specific Logo/build/UI/testing skills unless Plan discovers a new task. |

## Likely Skills

| Candidate | Path | Rationale | Confidence |
| --- | --- | --- | --- |
| `agent-customization` | `/usr/share/code/resources/app/extensions/copilot/assets/prompts/skills/agent-customization/SKILL.md` (workspace copy not present under `.github/skills/`) | Its description directly says it is for creating/updating/reviewing `.instructions.md`, `.prompt.md`, `.agent.md`, `SKILL.md`, `copilot-instructions.md`, and AGENTS.md files and specialized workflows. It is directly relevant to the policy-change class. | High |
| `parallel-xargs` | `.github/skills/parallel-xargs/SKILL.md` | Potential supporting evidence only if Plan assigns independent bounded shell tasks. It does not govern policy or skill discovery itself. | Low/conditional |
| No repository `ucb-logo-*` skill selected | `.github/skills/ucb-logo-*` | The request concerns orchestration policy and customization, not Logo runtime, programming, turtle graphics, frontend, testing, build/deploy, help retrieval, or agent usage accounting. A selected domain skill would be unsupported by current evidence. | High exclusion confidence |

## Options and Trade-offs

- **Option A: Add a Likely Skills section to Research findings.** The task request can be met at the evidence stage with candidate names, paths, rationale, confidence, and exclusions. Trade-off: it adds local structure to reports but does not automatically change Plan or prompts.
- **Option B: Add a candidate-skills field to each Plan task-map row or bounded task packet.** This directly helps workers. Trade-off: overlong maps, stale suggestions, and false precision if candidates are not task-specific. Suggested distinctions are “likely,” “required,” and “none,” with an optional rationale and path, preserving worker judgment while giving planners control.
- **Option C: Add likely skills only to worker prompts.** This reduces guesswork but leaves earlier research and planning without an auditable discovery rationale. Trade-off: workers may receive prompt-local guesses that are hard to review or reuse.
- **Option D: Preserve unstructured prose.** Lower implementation cost and no schema risk. Trade-off: inconsistent discovery, weak cross-task comparison, and more worker time spent parsing scope and searching skills.

A neutral planning consideration is to separate candidate discovery from final loading: discover from frontmatter, record likely candidates and exclusions, load full skill bodies only for directly relevant work, and allow workers to report mismatch or missing applicability. This preserves non-prescriptive advice while reducing full-catalog context use.

## Conflicts and Counterevidence

- The shared contract does not currently forbid skill hints, but also does not require or define them. Therefore adding hints is consistent with existing direct-prompt ownership, but changing policy would need to be explicit to become enforceable.
- The host-provided agent-customization skill says an agent-instruction primitive is most appropriate for always-on rules, while a skill is appropriate for specific tasks (source path `/usr/share/code/resources/app/extensions/copilot/assets/prompts/skills/agent-customization/SKILL.md`, lines 9-19, 49-51, 69-73). This creates a design trade-off: orchestration skill mapping may belong in `copilot-instructions.md` or an instruction file if intended always-on, or in contract/agent prompts if staged and task-specific.
- The request seeks less guessing, but making skills mandatory could reduce judgment. No repository evidence indicates a worker is currently blocked for lack of a skill, and candidate identification is described as an inference rather than an approval gate.
- Repository metadata is compact, but a description alone may over- or under-match. The `agent-customization` description directly covers the likely task, while domain skill descriptions can look relevant from a keyword but be excluded by scope evidence.

## Constraints and Dependencies

- The orchestrator owns the task/dependency graph and direct assignments ([`.github/workflows/development-orchestration.md` lines 5-10](../../.github/workflows/development-orchestration.md)). It also owns integration and commit scope.
- Candidate policy files are currently clean; pre-existing dirty changes outside this task must be preserved and not attributed to this research pass.
- The task is Research-only. Sequencing, owners, acceptance criteria, and implementation choices belong to the Planning stage or later human-approved stages and are intentionally omitted here.
- External sources were not necessary. Sources are limited to approved local policy and skill metadata.

## Unknowns and Unverified Items

- Whether Planning should use an optional “Likely Skills” field, a table column, or prose; no existing convention was found.
- Whether “likely” means advisory candidates, required loads, or both. The local contract does not define these meanings.
- Whether task owners are expected to rescan the full skill catalog or whether another generated/maintained catalog already exists beyond `SKILL.md` frontmatter. No generated index was found in the approved/local scan; this remains unverified beyond the reviewed repository paths.
- Whether the human considers an always-on instruction preferable to staged task hints. This requires the later policy-design dialogue.
- Host enforcement of model/effort and skill loading could not be tested; the contract itself labels this as policy rather than a security boundary ([`.github/workflows/development-orchestration.md` lines 1-4](../../.github/workflows/development-orchestration.md)).

## Search Coverage and Limitations

**Local sources inspected:** shared orchestration contract; research handoff README; orchestrator, research, and worker agent prompts; repository copilot instructions; repository skill frontmatter; and the host-provided agent-customization skill metadata/body at the path listed under Sources. No repository copy of that skill was found. **Search terms/categories:** likely skill, candidate skill, task-relevant skill, Plan/Parallel Implementation Map, direct assignment, worker prompt, skill catalog/frontmatter, and the relevant policy filenames.

No external sources were consulted because approved local sources answered the policy and catalog questions. The scan did not inspect every full domain skill body; that was intentionally unnecessary once the task scope excluded Logo/runtime/build/UI/testing work. The evidence is complete for the approved scope, but not a claim of exhaustive filesystem discovery outside the approved inputs.

## Validation and Artifact State

- This report is new and untracked; no commit was made during this Research pass.
- A read-only trailing-whitespace scan was run against this file and returned no findings.
- Relative Markdown links were checked against repository paths. The host-provided `agent-customization` skill is cited by its explicit system path, not by a repository-relative link; no repository copy exists at `.github/skills/agent-customization/SKILL.md`.
- The target contract and research handoff files were clean. Other reviewed sources `.github/agents/development-worker.agent.md` and `.github/copilot-instructions.md` had pre-existing modifications; this Research pass did not change or attribute those edits.

## Planning Handoff

The planning agent should address:

- whether skill suggestions appear in Research, Plan task-map rows, direct worker prompts, or a combination;
- how “Likely Skills” is represented and whether only names/paths or also rationale/confidence are required;
- whether candidates are advisory or expected to be loaded and reported, and how mismatches are handled;
- whether the narrow discovery source should be frontmatter-only metadata, a maintained catalog, or a generated index;
- whether `agent-customization` is a required/likely skill for policy-file tasks and whether `parallel-xargs` remains conditional;
- how `Likely Skills` interacts with existing direct-assignment contract fields and the worker's responsibility to load task-relevant skills;
- whether a policy change should use an instruction primitive, an agent/prompt field, or staged report/plan fields, based on whether the behavior is always-on or task-specific.

## Sources

- [Development Orchestration Contract](../../.github/workflows/development-orchestration.md), local policy, 2026-11-21.
- [Research Findings Handoff](../../docs/research/README.md), local format, 2026-11-21.
- [Development Orchestrator Agent](../../.github/agents/development-orchestrator.agent.md), local agent policy, 2026-11-21.
- [Research Agent](../../.github/agents/research.agent.md), local agent policy, 2026-11-21.
- [Development Worker Agent](../../.github/agents/development-worker.agent.md), local agent policy, 2026-11-21.
- [Repository Copilot Instructions](../../.github/copilot-instructions.md), local repository instructions, 2026-11-21.
- Agent Customization Skill (host-provided session skill, not a workspace file): `/usr/share/code/resources/app/extensions/copilot/assets/prompts/skills/agent-customization/SKILL.md`, 2026-11-21.
- Repository skill frontmatter: `.github/skills/*/SKILL.md`, enumerated by name/description on 2026-11-21.

<!-- Co-authored by GitHub Copilot: MAI Code 2 Flash, reasoning-effort max. Scope: ORCH-SKILLS-2026-11-21-R1 research report. Task elapsed time: 2m 26s (2026-11-21 11:57:34 to 12:00:00 UTC). Subagents: none. Energy (kWh): unavailable; no measured data or defensible estimation inputs. CO2: unavailable; no measured data or defensible estimation inputs. -->
