# QA and Check Result Log

Status: No out-of-workflow check results have been recorded yet.

This append-only log is for checks that have no owning formal workflow-run record. Formal workflow results belong only in `docs/workflow-runs/<task-id>.md` under `## QA and Check Results`; do not mirror them here.

Use the canonical entry format in the shared workflow contract. The Orchestrator assigns one appender for a single out-of-workflow entry; that appender serializes the append and verifies placement and whitespace. Multiple agents must not edit this file concurrently. Reviewers and QA return structured entries to the appender instead of editing this log themselves.

Use one H3 entry per independent check or disposition. Multiple subcommands may share an entry only when they validate one exact revision or artifact as one atomic target and produce one disposition.

Allowed results are `PASS`, `FAIL`, `BLOCKED`, and explicitly reasoned `N/A`. Missing evidence or unavailable required review is `BLOCKED`. Corrections append a new entry and set `Supersedes` to the earlier result ID; never rewrite or delete earlier entries.

This log is manually maintained. This change creates no parser, lock script, hook, migration, or automated append behavior. Keep prompts, secrets, credentials, payment data, and unnecessary personal data out of entries.

<!-- Co-authored by GitHub Copilot. Scope: out-of-workflow QA and check-result evidence log. Model and reasoning-effort metadata unavailable. Elapsed wall-clock time and environmental accounting unavailable. -->
