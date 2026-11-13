# Skill Use

- At the beginning of each task, identify and load applicable skills automatically.
- In the initial update, name the skills you are using. If none apply, say so. Only claim to use skills you actually load.

# Task Scope

- Explicit user limits override a skill's broader ownership: "only update skills" means no application changes, builds, or browser tests; "only the failing case" means no broader test runs until that case is repaired or the user changes scope. A skill describes capability, not authorization to expand the task.
- When the user says stop, stop work immediately. Follow the newest direction rather than resuming an earlier task or checklist without authorization.

# Process Ownership

- Reuse suitable existing servers and browsers. Track terminal IDs or PIDs, ports, and ownership for servers, browser runners, and other processes started during the task.
- Before finishing, stop task-owned temporary processes that are no longer needed. Keep a preview or remote-device session running when the user still needs it, and report its URL and ownership. Do not terminate pre-existing or user-owned processes, kill all Chrome processes, or restart a shared browser without permission.

# Browser Test Priority

- Default iteration uses focused desktop Chrome checks. For mobile-first design features or mobile-specific issues, use mobile Chrome, Chrome device emulation, or the affected mobile browser as the primary iteration target, according to what best exercises the feature or reproduces the issue.
- For broader validation, the default browser-test order is desktop Chrome, mobile Chrome (or Chrome device emulation), Firefox, then mobile Safari on the maintainer's iPad. Follow explicit user scope and prioritize the affected browser when diagnosing a browser-specific failure; do not run the whole matrix after every edit. Report emulation as emulation, not real-device coverage.
- Mobile Safari testing requires a human-in-the-loop session on the maintainer's iPad with the current no-Mac setup. Because device connection and interaction require coordination, do it last in broader validation, after the automated checks pass. Coordinate with the maintainer rather than assuming an unattended run; test earlier when specifically diagnosing a mobile Safari issue or explicitly requested.
- Desktop Safari is supported, but the maintainer cannot test it locally because there is no Mac. Report that coverage gap; use actual Safari through available macOS CI or an approved hosted service when feasible, without treating automated WebKit or iPad Safari as desktop Safari validation. See the interpreter skill's [platform coverage guidance](skills/ucb-logo-interpreter-development/SKILL.md#development-cadence-and-platform-coverage).

# Artifact Freshness

- Stale-artifact checks apply to all development activities, especially interpreter development. Before trusting a result, verify that the executable, loaded Logo definitions, generated assets, server root/URL, and running browser or process correspond to the intended source and build configuration, as applicable. A build on disk does not update an already-running runtime.
- When behavior contradicts the source, rule out stale state: disable HTTP caching and bypass or disable the service worker for local diagnosis, then reload the page or restart the affected runtime. If uncertainty remains, restart task-owned Chrome or use a fresh test profile and perform a full rebuild before rerunning the same check. Get permission before restarting the user's browser; preserve unsaved Logo work and persistent files/settings rather than clearing all site data.
- See the interpreter skill's [stale-artifact checklist](skills/ucb-logo-interpreter-development/SKILL.md#checking-for-stale-artifacts) for browser-cache, service-worker, and build details. These diagnostic steps are not permission to change production caching or run builds during instruction-only tasks.