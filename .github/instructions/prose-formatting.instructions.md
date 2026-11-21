---
description: "Use when editing Markdown or prose text. Applies deterministic whitespace normalization and prevents manual trailing-whitespace loops."
applyTo:
  - "**/*.md"
  - "**/*.markdown"
  - "**/*.mkd"
  - "**/*.mdown"
  - "**/*.txt"
  - "**/*.text"
---

# Markdown And Prose Formatting

Use the repository normalizer as the single source of whitespace behavior. Do not manually search for trailing blanks or repeated terminal blank lines.

After editing a matching file, run:

```sh
node scripts/format-whitespace.js --changed
```

For a final verification without rewriting, run:

```sh
node scripts/format-whitespace.js --check --changed
```

For an explicit set of files, pass the paths instead of `--changed`. The PostToolUse and Stop hooks invoke the same script automatically where supported; unsupported, binary, or outside-workspace files are skipped with a warning.

The formatter converts CRLF/CR line endings to LF, removes ordinary trailing horizontal whitespace, preserves exactly one final newline, removes trailing blank lines, and preserves exactly two trailing spaces only on Markdown lines that introduce a following line. It does not format source code or generated/binary files.

<!-- Co-authored by GitHub Copilot. Scope: Markdown and prose whitespace instructions. Model: MAI Code 2 Flash; reasoning-effort setting unavailable. Energy (kWh) and CO2: unavailable; no measured data or defensible estimation inputs. -->
