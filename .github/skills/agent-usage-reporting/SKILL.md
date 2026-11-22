---
name: agent-usage-reporting
description: "Use when: reading, summarizing, or attributing GitHub Copilot agent usage costs from commit-message accounting blocks, including granular product/SKU, checkpoint, date, and Git commit breakdowns."
---

# Agent Usage Reporting

Use `scripts/report-agent-usage-v2.js` or the read-only Python compatibility wrapper `scripts/report-agent-usage.py` for commit-native usage analysis. Do not create or append to a ledger and do not make billing estimates from incomplete data.

## Workflow

1. Run the reporter from the repository root:
   - Default commit range: `node scripts/report-agent-usage-v2.js --repo .`
   - Exact PR range: `node scripts/report-agent-usage-v2.js --repo . --range BASE..HEAD`
   - Explicit UTC/date range: `node scripts/report-agent-usage-v2.js --repo . --since '2026-11-20T00:00:00Z' --until '2026-11-22T00:00:00Z'`
   - Concise issue summary: add `--issue`.
2. Report commit count, gross, discount, net, product/SKU/model rollups, and unavailable caveats.
3. Deduplicate `accounting_id` values across the exact range.
4. Distinguish measured API values from authoritative user-supplied billing UI values and unavailable or blocked measurements.
5. When a commit lacks a valid accounting block, fail closed or use the explicit unavailable fallback; do not invent a value.
6. State that account-level totals are temporal attribution, not proof that a commit caused the usage.
7. Apply the repository's approved CO2e rule only to available net monetary usage: `dollars * 0.20`; label it as an estimate rather than measured emissions.

## Commit Format

Accounting blocks are appended to commit messages by the managed hooks:

- `COPILOT-ACCOUNTING-BEGIN v1`
- exactly one canonical UTF-8 JSON object
- `COPILOT-ACCOUNTING-END`

The JSON object is the machine-readable source. Human-readable totals are redundant and must not be parsed as accounting source.

<!-- Co-authored by GitHub Copilot. Model: MAI Code 2; reasoning-effort setting unavailable. -->
