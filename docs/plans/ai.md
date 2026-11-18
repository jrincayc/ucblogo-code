# Project Plan: AI Use and Copilot Guide

Status: Proposed; this plan does not create the root `AI.md`.
Date: 2026-11-18

## Objective

Create a top-level `AI.md` that explains how AI is used in this repository,
sets a practical contribution policy for AI-assisted work, and gives developers
a short tutorial for using VS Code Copilot with the project, including its
orchestration workflow.

## Current Evidence

- [`.github/copilot-instructions.md`](../../.github/copilot-instructions.md)
  is the project-wide Copilot guidance. It covers applicable skills,
  attribution, scope, commits and pushes, privacy, artifact freshness, and
  validation expectations.
- [`.github/agents/`](../../.github/agents/) contains project roles for the
  development orchestrator, lead, worker, QA, reviewer, design reviewer, and
  research agent.
- [`.github/skills/`](../../.github/skills/) contains task-specific UCB Logo
  development and testing references.
- [`.github/instructions/`](../../.github/instructions/) contains
  path-specific instructions, including acceptance tests, garbage collection,
  and generated artifacts. [`.github/prompts/`](../../.github/prompts/)
  contains focused CI-triage and deployment prompts.
- [`.github/workflows/development-orchestration.md`](../../.github/workflows/development-orchestration.md)
  defines the staged workflow, independent QA, human checkpoints, and
  commit/push controls.

These files document intended agent behavior; they are not by themselves
technical enforcement or proof that a particular model, tool, or workflow was
used. Before describing AI as part of the application itself, inspect the
product code and verify that claim.

## Scope

The future `AI.md` should:

1. Summarize the repository's configured AI-assisted development workflow and
   identify the principal Copilot customization entry points.
2. Link to the operational instructions instead of copying their full
   contents or creating competing policy.
3. Give a common-sense policy: contributors remain responsible for AI-assisted
   changes; review generated code, tests, and prose; validate behavior; protect
   secrets and private data; respect project instructions and licenses; and
   obtain explicit approval for pushes, deployments, or destructive Git actions
   where the repository requires it.
4. Explain the repository's attribution expectations by linking to the
   canonical Copilot instructions. Do not invent model details, telemetry, or
   claims of enforcement.
5. Provide a brief VS Code Copilot walkthrough: describe the task, constraints,
   and acceptance checks; use the Development Orchestrator for multi-stage
   work; use relevant project skills for specialized work; review the proposed
   edits; and run the focused validation before accepting or committing.
6. Describe the orchestration sequence accurately: Research, Plan, Test Plan,
   Implement Tests, Implement Changes, and Code Review, with independent QA
   and explicit human checkpoints as defined by the shared contract.

## Non-Goals

- Do not change agent definitions, skills, prompts, instructions, or workflow
  enforcement as part of writing `AI.md`.
- Do not claim that prompts or agent instructions prevent unsafe actions.
- Do not imply that the Logo application contains an AI feature unless source
  inspection confirms it.
- Do not duplicate the full orchestration contract or make `AI.md` the
  operational source of truth.

## Implementation Steps

1. Confirm the current repository AI-related files and inspect product code
   before making claims about runtime AI features.
2. Verify any VS Code Copilot UI steps against current official VS Code
   documentation; keep interface-specific instructions short and easy to
   update.
3. Draft root `AI.md` with sections for current use, contribution policy, and
   the Copilot/orchestration tutorial. Use repository-relative links to
   detailed guidance.
4. Review the complete document for factual accuracy, consistency with the
   existing instructions, readability, and working links.

## Ownership, Dependencies, and Integration

- The implementation owner, assigned after approval of this plan, owns the
  single-file change to root `AI.md`. The orchestrator owns integration, and
  an independent Development QA agent checks the implementation and commit.
- The implementation depends on approval of this plan and verification of the
  repository guidance and any current VS Code Copilot UI details it describes.
  No application-code or package changes are in scope.
- Integrate in the numbered order above: verify evidence, draft `AI.md`, then
  check facts, links, and consistency. Do not edit the existing instructions,
  agents, skills, prompts, or application code as part of this task.
- Rollback is limited to removing the additive root `AI.md` in a separate
  reviewed commit if it proves inaccurate or conflicts with canonical policy.
  Do not revert or rewrite unrelated history; any push requires explicit
  approval for its exact destination and commits.

## Acceptance Criteria

- A reader can distinguish configured AI-assisted development from any
  verified AI functionality in the application.
- Policy statements agree with the existing Copilot instructions and do not
  turn advisory guidance into unsupported security or enforcement claims.
- The tutorial explains how to start a Copilot task in this repository, when
  to use orchestration, the stage sequence, and the role of QA and human
  approvals.
- All links resolve from the root `AI.md`; no operational policy is copied in
  a way that can drift from its canonical source.
- Only `AI.md` is added for the implementation; existing configuration and
  application behavior remain unchanged.
- The Markdown diff passes `git diff --check`; the final file receives a
  human-readable factual and link review.

## Risks and Mitigations

- **Copilot UI changes:** Verify UI-specific directions against current
  official documentation and avoid brittle menu-by-menu detail.
- **Policy drift:** Link to canonical project instructions and orchestration
  documents; keep the top-level guide introductory.
- **Overstating AI capability:** Inspect source before claiming the product has
  runtime AI functionality, and separate documented configuration from
  observed use.

<!-- Co-authored by GitHub Copilot. Model and reasoning-effort: unavailable. Energy (kWh) and CO2: unavailable; no measured telemetry or defensible estimation inputs. -->
