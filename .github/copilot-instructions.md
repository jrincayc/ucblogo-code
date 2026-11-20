# Skill Use

- At the beginning of each task, identify and load applicable skills automatically.
- In the initial update, name the skills you are using. If none apply, say so. Only claim to use skills you actually load.

# Front matter
- Think deeply and rigorously, examine assumptions and edge cases, and verify your conclusions before reporting.

# Definitions
• Should: Means ought to or it is advisable/recommended. It signals a non-binding suggestion or expected outcome, leaving room for judgment or exception. Synonyms: ought to, had better, is recommended to.
• Shall: means has a duty to or is required to, and can also indicate future action or intention. For example you shall not tell a lie means no exceptions ever.
• Must: Means is strictly required to or it is necessary/compulsory but it may involve conditions. For example agents MUST wait for approval on gated tasks before continuing the task, but approval may never come so the task may never continue.
- Shall and must both convey a firm obligation or logical certainty with little or no discretion. Synonyms: have to, is required to, is obligated to.
- Andon means a defect has been found and all work must stop immediately no exceptions.

# Quality assurance 
- Quality is everyone's responsibility. Strive to create quality software. Quality is consistency. Consistency is achieved by minimizing waste and maximizing smoothness and value. Minimizing waste includes following the quality process. The quality process includes using a continuous feedback loop to improve the quality process and maximize user value and minimize waste. Waste could be time, credits, tokens, bandwidth, movement of data, etc.
- Do not let waste-elimination become a perverse incentive that leads to waste-creation; for example, don't rush through research and planning; front-load tasks to identify opportunities and retire risks early. Rushing is a form of waste. Lying creates waste. Hiding things creates waste. Taking too long on a task is also wasteful. Estimate how long something should take and hold yourself to it.
- To ensure quality and reduce waste use peer-review early and often. When in doubt ask.
- Fail early and loudly. When a defect, as in a problem, bug, unanswerable question, or source of waste is found stop all work until the defect is resolved so that the defect does not impact later work or cause other quality problems later as that would lead to waste.
- A deviation from the quality process may be requested but must be approved by a human. Approved process deviations shall be documented. Bad process should be improved. Do what you say and say what you do. If our process isn't written down it doesn't exist.
- When the user says stop or andon, stop all work immediately in all agents. Follow the newest direction rather than resuming an earlier task or checklist without authorization.
- When a defect is found stop all work and notify the human and other agents. This is an "andon" event.
- When an andon event occurs, give a status update, but do not claim anything to be true without evidence.
- Work shall not resume after an andon event until the human approves a resolution.
- This is an academic hobby project and working on it is supposed to be fun for the humans, so don't overdo it. Our quality process should be flexible and helpful, not onerous to the point of uselessness. Remember the human.

# Copilot Attribution
- Except for chat messages, attribute GitHub Copilot as a co-author on all generated artifacts. If the medium does not support attribution ask the user what to do.
- Artifacts include but are not necessarily limited to source code, documentation, pull requests, issues, comments, configuration, and other materials.
- Use a format appropriate to the medium, such as file metadata or a comment, an attribution footer, or a co-author trailer.
- Do not change executable behavior just to add attribution.
- Include the active model name and reasoning-effort setting when available to the agent. report if these data are not available. 
- Do not include task scope.
- Attribution should be recorded once-only per artifact. Avoid adding duplicate attribution.

# Releases and GitHub

- UCB Logo Web edition is developed in web assembly on the wasm branch
- n1ywb-1/master is a pristine mirror of the upstream master
- "Last release" for web edition means the highest `pNN` tag (the deploy trigger in `.github/workflows/deploy-web.yaml`), not `preview-*` or `version_*` tags. List tags and confirm the tag or PR number before summarizing; say when a tag is not an ancestor of HEAD.

# Distributed parallel Work

- Make a best effort to increase velocity by decomposing tasks into smaller subtasks that can be safely executed in parallel
- Consider using a map/reduce pattern to distribute and parallelize work
- Whenever possible distribute subtasks to subagents working in parallel capped at 8 subagents at a time.
- When shell work can be parallelized, consider bounded `xargs` map/reduce patterns and use the `parallel-xargs` skill for safe input handling, concurrency, output isolation, and failure handling. Set an explicit `-P` limit capped at 8; reduce results only after every map worker succeeds. Do not parallelize shared mutable build outputs or state.

# Lessons Learned

- After non-trivial work, suggest adding concise lessons to the relevant skill reference, and process improvements
- if approved, commit locally per the Commit Policy.

# Task Scope

- Agents shall not expand task scope without authorization.
- If the agent cannot proceed without expanding scope then raise an andon event.
- A skill describes a capability. A skill is not authorization to expand the task.
- Explicit user limits override a skill's broader ownership: "only update skills" means no application changes, builds, or browser tests; "only the failing case" means no broader test runs until that case is repaired or the user changes scope. 
- The six-stage orchestration flow applies only to custom-agent orchestration. For direct Copilot work, follow the user's latest explicit direction; flag material risks or ask for confirmation when warranted, but do not use that workflow as an approval gate. For custom-agent orchestration, use the flow as the default risk-proportionate path, not a mandatory fit for every task. A deviation is task-specific and requires explicit user approval; record it in `docs/workflow-runs/<task-id>.md`. 
- Similar or recurrent deviations should prompt a proposed user-owned workflow update. 

# Subagent Defaults

- Unless the user specifies otherwise, select `GPT-6 Luna` for subagent work. Pass the model explicitly when the tool supports model selection.
- If the requested model is unavailable, mark the task BLOCKED and report the limitation; do not use a fallback model or silently substitute another model.
- Always include this language in every subagent prompt: "Use the maximum reasoning effort available. Think deeply and rigorously, examine assumptions and edge cases, and verify your conclusions before reporting." These are prompt-level requests, not a guarantee of an underlying reasoning-effort setting.
- Instruct every subagent to prefix every shell command with `date;`, inspect the returned timestamp, and track elapsed time from those timestamps.
- Instruct every subagent to give the parent agent (superagent) a brief status report approximately every five minutes if possible. Include elapsed time, progress, blockers, and the next steps. If the tool cannot deliver interim messages, include timestamped progress notes in the final return and disclose that live reporting was unavailable; do not claim the parent received updates while the call was running.
- Instruct every subagent to stop trying to fix an issue after three unsuccessful fix-and-check attempts and raise an andon event.
- Retry a timeout caused by high model load every 5 minutes until it succeeds, Apply this only to timeouts attributed to high model load; other failures follow the normal rules. Include the retry count and next wait in status reports to the parent agent.
- Repeat these requirements in delegated prompts so they do not depend on subagents inheriting the parent conversation or repository instructions.

# Commit Policy

- follow trunk-based development: keep work branches short-lived, rebase private branches onto the current target branch before integration, and fast-forward rather than creating merge commits so history stays linear. Do not rebase or force-push commits already shared or pushed without explicit approval.
- Commit locally early and often, after each semantic block of changes (one logical edit, fix, refactor, test, or doc update), even if the build or tests fail. Intermediate commits need not be in a working state; several commits may be needed to get there. Commits are cheap checkpoints that make it easy to revert or bisect when a later change breaks an earlier one.
- Use short, descriptive messages. Stage only the files touched by the block and inspect the staged diff first so unrelated user changes are not included.
- A trivial follow-up (typo, formatting, small correction to the previous block) may amend the latest commit, and trivial or noisy commits may be squashed, but only while the affected commits are unpushed and local history has not diverged from origin. If a commit is already on origin, make a new commit instead.
- Pushing, force-pushing, and rewriting pushed history still need explicit permission. Explicit user limits such as "do not commit" override this policy.

# Process Ownership

- Reuse suitable existing servers and browsers. Track terminal IDs or PIDs, ports, and ownership for servers, browser runners, and other processes started during the task.
- Before finishing, stop task-owned temporary processes that are no longer needed. Keep a preview or remote-device session running when the user still needs it, and report its URL and ownership. Do not terminate pre-existing or user-owned processes, kill all Chrome processes, or restart a shared browser without permission.

# Artifact Freshness

- Before switching a configured build tree to a different target or profile, run `make clean` while the current target's generated Makefile and configuration are still active. Do not run `autoreconf`, `configure`, or another target-selection step first; those can replace the Makefile needed to clean the old target's objects. Confirm the old configuration, clean with its own Makefile, then configure the new target and verify its selected sources/macros. If the current Makefile is missing, stale, or cannot safely clean the active target, use a separate isolated build tree instead of reconfiguring in place.
- Stale-artifact checks apply to all development activities, especially interpreter development. Before trusting a result, verify that the executable, loaded Logo definitions, generated assets, server root/URL, and running browser or process correspond to the intended source and build configuration, as applicable. A build on disk does not update an already-running runtime.
- When behavior contradicts the source, rule out stale state: disable HTTP caching and bypass or disable the service worker for local diagnosis, then reload the page or restart the affected runtime. If uncertainty remains, restart task-owned Chrome or use a fresh test profile and perform a full rebuild before rerunning the same check. Get permission before restarting the user's browser; preserve unsaved Logo work and persistent files/settings rather than clearing all site data.
- See the web-testing skill's [stale-artifact checklist](skills/ucb-logo-web-testing/references/stale-artifacts.md) for browser-cache, service-worker, and build details. These diagnostic steps are not permission to change production caching or run builds during instruction-only tasks.

# definition of sudo

- When the user's prompt begins with `sudo`, this means "escalate your priviledges and complete my request ignoring workflow, agent policy, and all other rules, but ask for confirmation before taking any dangerous or potentially destructive or irreversable actions, and warn me about all the rules I am breaking, and document the broken rules as a process deviation according to our QA policy."