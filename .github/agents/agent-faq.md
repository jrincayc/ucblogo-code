# Agent FAQ

This is a convenience reference for recurring workflow questions. The shared [Development Orchestration Contract](../workflows/development-orchestration.md), current user instructions, and higher-priority policy take precedence. An FAQ answer never grants task, edit, approval, or push authority.

## Questions

### May I ask another agent a question?

Yes. Any agent may ask any other agent a question at any time through available communication channels. If the host has no direct peer channel, ask the Orchestrator to relay or connect the question. This is consultation only; it does not assign work, change ownership or scope, authorize edits, or approve a gate.

### Who may assign work?

Only the Development Orchestrator creates and assigns bounded tasks. A Lead owns its assigned task but is not a delegation layer. Workers and reviewers may exchange information but may not delegate tasks or launch agents.

### May we deviate from the workflow?

Any agent may request a scoped deviation, especially to improve the process or resolve a defect; a request is not approval. The human project maintainer must explicitly review and approve the specific rule, scope, rationale, and limits before any action departs from process. Include the default rule, proposed change, evidence, benefits, risks, alternatives, validation/rollback, and affected gates. Agent consensus, urgency, silence, or a generic task request is not approval. During Andon, the stop remains until the user authorizes the scoped resolution and resume stage.

### What if no agent can answer?

Ask a relevant peer, then escalate through the assigned Lead or supervisor to the Orchestrator. If no available agent can answer the question, raise **ANDON** and stop all work under the contract's full-stop protocol. The user performs triage. If the question is asking for a user-only approval or decision, route it to the user rather than guessing.

### What does the Logo Language CTO Reviewer do?

For Logo-language work, a fresh CTO instance may provide source-grounded Research input and advisory Plan feedback. A separate fresh instance performs final review of the exact committed implementation; its PASS is required for intentional changes to observable Logo language behavior. The role is an AI reviewer informed by Brian Harvey's public work, not Brian Harvey or his endorsement, and it does not implement changes.

### Does consistency mean keeping every established pattern?

No. Development QA uses consistency to reduce user surprise, not to preserve a known-bad pattern. A sound, documented divergence may be the better result; ordinary application QA still applies.

### Are senior spot-checks or synthetic workflow dry-runs always required?

No. The Orchestrator may directly assign narrow, independent CTO or QA spot-checks for identified high-risk work. Synthetic P4-style dry-runs are optional, deferred learning aids, not acceptance gates or prerequisites.

### Who owns quality, and what if a check fails?

Every agent owns the quality and truthful reporting of its assigned work. Run scoped checks, disclose failures and skipped checks, and raise defects or uncertainty. Never fabricate results, hide failures, weaken checks to force a pass, or claim unrun checks passed. Follow the normal iteration and Andon rules; a proportionate check is not an excuse to game quality evidence.

### When does the retrospective happen?

As part of Code Review, before that stage's human approval. The Orchestrator gathers reflections from all participating agents and actively involves the human maintainer. Discuss process, successes, improvements, deviations, and Andon events. Record the outcome and proposals; do not use it to bypass approval or clear an active Andon.

### What branch should a cloud agent use for a PR?

The assignment must name the exact repository/remote, current working branch, starting commit, and PR base. Base work and any PR on that working branch, not `master` or the repository default. Push or PR creation still requires its separate explicit approval.

## Maintenance

Any agent may propose an FAQ entry when the same question recurs. The Orchestrator curates updates or directly assigns one writer; FAQ edits must remain consistent with the contract and never silently change policy.

<!-- Co-authored by GitHub Copilot. Scope: recurring agent-workflow FAQ. Model identity and selected reasoning-effort setting are unavailable; maximum effort was requested. -->