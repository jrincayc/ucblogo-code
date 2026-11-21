---
description: "Triage a failing GitHub Actions run for this repo: find the failing job and step, read the log, and separate source failures from environment or harness causes."
argument-hint: "Run ID or URL; leave blank to use the most recent failed run"
agent: "agent"
---
Triage a failing CI run.

- Run: ${input:run:Run ID or URL, or blank for the most recent failed run}
- Use `gh run list` and `gh run view <id> --log-failed` to find the failing workflow, job, and step. Save long logs to a temporary file and search them instead of pasting them.
- Record the toolchain actually used (Emscripten, Node, npm) from the log.
- Classify the failure: build or configure, test assertion, harness or runner, timeout or hang, or infrastructure. Quote the first real error, not downstream noise.
- For browser test failures, check artifact freshness first with the [stale-artifacts checklist](../skills/ucb-logo-web-testing/references/stale-artifacts.md), and use the [web-testing skill](../skills/ucb-logo-web-testing/SKILL.md) for reproduction.
- Report the likely cause, the evidence, and the smallest next step. Do not rerun workflows, push, or change files unless asked.
