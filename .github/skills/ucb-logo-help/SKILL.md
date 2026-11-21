---
name: ucb-logo-help
description: "Retrieve UCB Logo help text for a Logo word or primitive by name. Use when someone asks for a word's help text, usage, or documentation."
argument-hint: "Logo word name"
---

# UCB Logo Help Lookup

Run the bundled script from the repository root, replacing `print` with the
requested Logo word:

```sh
python3 .github/skills/ucb-logo-help/scripts/get_help.py -- "print"
```

The lookup is case-insensitive and reads the matching file from `helpfiles/`.
Return the script's standard output as the help text without summarizing it. If
the script reports that no help file exists, report that result to the user.

<!-- Co-authored by GitHub Copilot. Scope: repository-local UCB Logo help lookup skill. Model identity and selected reasoning-effort setting are unavailable; elapsed time and environmental accounting are recorded in docs/workflow-runs/UCB-LOGO-HELP-LOOKUP-2026-11-18.md. -->
