# Skill Use

- At the beginning of each task, identify and load applicable skills automatically.
- In the initial update, name the skills you are using. If none apply, say so. Only claim to use skills you actually load.

# Copilot Attribution

- Do not include attribution or attribution-related cost, time, energy, or emissions footers in private one-to-one chats with the user, including progress updates and final responses.
- Attribute GitHub Copilot as a co-author on artifacts generated or materially revised for an audience beyond the user, including documentation, pull requests, issues, shared code, comments, configuration, and other published or shared prose. This applies to shared artifacts drafted within a private chat, but not to the surrounding conversation. Use a format appropriate to the medium, such as file metadata or a comment, an attribution footer, or a co-author trailer; do not change executable behavior just to add attribution.
- For those shared-artifact attributions, include the active model name and reasoning-effort setting when available to the agent. Never guess either value; omit details that are unavailable.
- Omit credit and dollar costs from attribution for now unless the user explicitly requests them.
- Include elapsed wall-clock time, energy consumed in kWh, and CO2 produced (with mass units) when available. State the accounting scope, such as the current task, and include subagent usage when known without double-counting.
- Measure elapsed time using a clock: record a start timestamp at the beginning of each task (for example, with `date -u`), take an end reading before reporting, and calculate the difference. Do not mark elapsed time unavailable merely because billing or model telemetry is absent. If timing starts late, label the measured interval as partial rather than presenting it as the full task duration. Report task wall-clock duration separately from summed subagent durations, which may overlap.
- Prefer measured data. Estimate missing metrics when there is a defensible basis; clearly label estimates, state their assumptions and sources concisely, and use ranges when uncertainty warrants. Distinguish CO2 from CO2-equivalent emissions.
- Do not invent telemetry, prices, hardware power consumption, or grid carbon intensity. If a metric cannot be measured or reasonably estimated, mark it unavailable, not zero. Do not infer inference energy or emissions from elapsed wall-clock time alone.

<!-- Co-authored by GitHub Copilot. Scope: attribution-policy updates. Energy (kWh) and CO2: unavailable; no measured data or defensible estimation inputs available. Elapsed time was not recorded for the original update. -->

# Releases and GitHub

- "Last release" means the highest `pNN` tag (the deploy trigger in `.github/workflows/deploy-web.yaml`), not `preview-*` or `version_*` tags. List tags and confirm the tag or PR number before summarizing; say when a tag is not an ancestor of HEAD.
- Run `gh` in a terminal for CI logs, runs, PRs, and issues instead of scraping web pages or APIs.

# Lessons Learned

- After non-trivial debugging or test work, add concise lessons to the relevant skill reference, not to this file, and commit locally per the Commit Policy.

# Task Scope

- Explicit user limits override a skill's broader ownership: "only update skills" means no application changes, builds, or browser tests; "only the failing case" means no broader test runs until that case is repaired or the user changes scope. A skill describes capability, not authorization to expand the task.
- Use the six-stage orchestration flow as the default risk-proportionate path, not a mandatory fit for every task. A deviation is task-specific and requires explicit user approval; record it in `docs/workflow-runs/<task-id>.md`. Similar or recurrent deviations should prompt a proposed user-owned workflow update. VS Code Ask, Plan, and Agent are separate host supervisory modes; repository text guides but cannot enforce host behavior.
- When the user says stop, stop work immediately. Follow the newest direction rather than resuming an earlier task or checklist without authorization.

# Subagent Defaults

- Unless the user specifies otherwise, select GPT-6 Luna for subagent work. Pass the model explicitly when the tool supports model selection. If that model is unavailable, report the limitation instead of silently substituting another model.
- Always include this language in every subagent prompt: "Use the maximum reasoning effort available. Think deeply and rigorously, examine assumptions and edge cases, and verify your conclusions before reporting." This is a prompt-level request, not a guarantee of an underlying reasoning-effort setting.
- Instruct every subagent to prefix every shell command with `date;`, inspect the returned timestamp, and track elapsed time from those timestamps.
- Instruct every subagent to give the parent agent (superagent) a brief status report approximately every five minutes, or at the earliest opportunity after a blocking operation. Include elapsed time, progress, blockers, and the next step. If the tool cannot deliver interim messages, include timestamped progress notes in the final return and disclose that live reporting was unavailable; do not claim the parent received updates while the call was running.
- Instruct every subagent to stop trying to fix an issue after three unsuccessful fix-and-check attempts. Return the attempted fixes, validation results, remaining blocker, and any useful next-step recommendation to the parent agent instead of starting a fourth attempt on the same issue.
- Repeat these requirements in delegated prompts so they do not depend on subagents inheriting the parent conversation or repository instructions.

# Commit Policy

- For parallel agent work, follow trunk-based development: keep work branches short-lived, rebase private branches onto the current target branch before integration, and fast-forward rather than creating merge commits so history stays linear. Do not rebase or force-push commits already shared or pushed without explicit approval.
- Commit locally early and often, after each semantic block of changes (one logical edit, fix, refactor, test, or doc update), even if the build or tests fail. Intermediate commits need not be in a working state; several commits may be needed to get there. Commits are cheap checkpoints that make it easy to revert or bisect when a later change breaks an earlier one.
- Use short, descriptive messages. Stage only the files touched by the block and inspect the staged diff first so unrelated user changes are not included.
- A trivial follow-up (typo, formatting, small correction to the previous block) may amend the latest commit, and trivial or noisy commits may be squashed, but only while the affected commits are unpushed and local history has not diverged from origin. If a commit is already on origin, make a new commit instead.
- Pushing, force-pushing, and rewriting pushed history still need explicit permission. Explicit user limits such as "do not commit" override this policy.

# Process Ownership

- Reuse suitable existing servers and browsers. Track terminal IDs or PIDs, ports, and ownership for servers, browser runners, and other processes started during the task.
- Before finishing, stop task-owned temporary processes that are no longer needed. Keep a preview or remote-device session running when the user still needs it, and report its URL and ownership. Do not terminate pre-existing or user-owned processes, kill all Chrome processes, or restart a shared browser without permission.

# Browser Test Priority

- Never use the VS Code embedded/integrated browser. Use external Chrome or headless runs, and show screenshots when the user asks to see UI.
- Default iteration uses focused desktop Chrome checks. For mobile-first design features or mobile-specific issues, use mobile Chrome, Chrome device emulation, or the affected mobile browser as the primary iteration target, according to what best exercises the feature or reproduces the issue.
- For broader validation, the default browser-test order is desktop Chrome, mobile Chrome (or Chrome device emulation), Firefox, then mobile Safari on the maintainer's iPad. Follow explicit user scope and prioritize the affected browser when diagnosing a browser-specific failure; do not run the whole matrix after every edit. Report emulation as emulation, not real-device coverage.
- Mobile Safari testing requires a human-in-the-loop session on the maintainer's iPad with the current no-Mac setup. Because device connection and interaction require coordination, do it last in broader validation, after the automated checks pass. Coordinate with the maintainer rather than assuming an unattended run; test earlier when specifically diagnosing a mobile Safari issue or explicitly requested.
- Desktop Safari is supported, but the maintainer cannot test it locally because there is no Mac. Report that coverage gap; use actual Safari through available macOS CI or an approved hosted service when feasible, without treating automated WebKit or iPad Safari as desktop Safari validation. See the web-testing skill's [platform matrix](skills/ucb-logo-web-testing/references/platform-matrix.md).

# Artifact Freshness

- Before switching a configured build tree to a different target or profile, run `make clean` while the current target's generated Makefile and configuration are still active. Do not run `autoreconf`, `configure`, or another target-selection step first; those can replace the Makefile needed to clean the old target's objects. Confirm the old configuration, clean with its own Makefile, then configure the new target and verify its selected sources/macros. If the current Makefile is missing, stale, or cannot safely clean the active target, use a separate isolated build tree instead of reconfiguring in place.
- Stale-artifact checks apply to all development activities, especially interpreter development. Before trusting a result, verify that the executable, loaded Logo definitions, generated assets, server root/URL, and running browser or process correspond to the intended source and build configuration, as applicable. A build on disk does not update an already-running runtime.
- When behavior contradicts the source, rule out stale state: disable HTTP caching and bypass or disable the service worker for local diagnosis, then reload the page or restart the affected runtime. If uncertainty remains, restart task-owned Chrome or use a fresh test profile and perform a full rebuild before rerunning the same check. Get permission before restarting the user's browser; preserve unsaved Logo work and persistent files/settings rather than clearing all site data.
- See the web-testing skill's [stale-artifact checklist](skills/ucb-logo-web-testing/references/stale-artifacts.md) for browser-cache, service-worker, and build details. These diagnostic steps are not permission to change production caching or run builds during instruction-only tasks.

# Multi-Target Test Runs

- wx tests exist only to detect regressions caused by Web Edition changes. Pre-existing wx failures and a janky harness are out of scope; take screenshot baselines from the CI environment, not a local machine.
- When asked to run all tests across the main targets, delegate wxWidgets, Unix terminal, WebAssembly browser, and Node/WebAssembly validation to separate subagents and run them in parallel.
- Give each target its own detached worktree or isolated build directory. Native and Emscripten configurations rewrite shared generated build files and object outputs; do not reconfigure a shared checkout concurrently or reuse objects across targets.
- Use `.github/workflows/ci.yaml` as the test inventory and command source of truth. Include each relevant job and grading step, report skipped or unavailable coverage, and do not install missing dependencies without user approval.
- Treat wx unit and smoke tests, both Unix terminal configurations and PTY/literal-output checks, Web build and browser/source/audit checks, and a separately built Node/Wasm suite as distinct coverage. Avoid duplicating target work unless a build dependency requires it.

<!-- Co-authored by GitHub Copilot. Scope: multi-target test workflow guidance. -->

<!-- Co-authored by GitHub Copilot. Scope: risk-proportionate process and approved deviations. Model identity and selected reasoning effort are unavailable; maximum effort was requested. See docs/workflow-runs/ORCH-AGILE-SUPERVISORS-01.md for task accounting. -->

<!-- Co-authored by GitHub Copilot. Scope: parallel-agent trunk-based integration guidance. Model identity and selected reasoning effort are unavailable. Measured task interval from 2026-11-19 01:39:04 to 01:41:28 UTC: 2m 24s. Energy (kWh) and CO2: unavailable; no measured data or defensible estimation inputs available. -->