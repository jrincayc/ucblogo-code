---
name: agent-usage-reporting
description: "Use when: reading, summarizing, or attributing GitHub Copilot agent usage costs from docs/accounting/agent-usage-ledger.md, including granular product/SKU, checkpoint, date, and Git commit breakdowns."
---

# Agent Usage Reporting

Use `scripts/report-agent-usage.py` for repeatable, read-only usage analysis. Do not edit the ledger or make billing estimates from incomplete data.

## Workflow

1. Run the reporter from the repository root:
   - Current day: `python3 scripts/report-agent-usage.py`
   - Explicit UTC/date range: `python3 scripts/report-agent-usage.py --since '2026-11-20T00:00:00Z' --until '2026-11-21T00:00:00Z'`
   - Alternate ledger: pass `--ledger PATH`.
2. Report the latest cumulative gross, discount, net, and product/SKU rollup.
3. List snapshot-to-snapshot net deltas in timestamp order.
4. Attribute each interval to the commit/HEAD recorded by the preceding snapshot.
5. Distinguish measured API values from authoritative user-supplied billing UI values and blocked/unavailable measurements.
6. When a snapshot lacks a detailed rollup, do not invent one.
7. State that account-level cumulative deltas are temporal commit attribution, not proof of causal cost ownership.
8. Apply the repository's approved CO2e rule only to monetary cost: `dollars * 0.20`; label it as an estimate rather than measured emissions.

## Output Shape

Include these sections when data is available:

- Reporting period and source
- Latest cumulative total
- Product/SKU breakdown
- Snapshot progression
- Git commit attribution
- Measured interval total
- Estimated CO2e
- Balance or remaining-budget line
- Data limitations and ordering caveats

Preserve nine decimal places for ledger currency values. Deduplicate identical snapshots for analysis, but never alter or reorder the append-only ledger.

<!-- Co-authored by GitHub Copilot. Model: MAI Code 2; reasoning-effort setting unavailable. -->
