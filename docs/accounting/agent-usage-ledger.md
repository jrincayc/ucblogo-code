# Agent Usage Accounting Ledger

Append-only snapshots of read-only GitHub usage/accounting metadata. Do not store tokens, payment information, card details, invoices, prompts, responses, or credentials. Amounts are USD unless explicitly labeled otherwise.

## 2026-11-20: Add accounting lifecycle policy — start

- UTC timestamp: `2026-11-20T22:01:50Z`
- Checkpoint: `start`
- Account: `n1ywb-1`
- Billing source: `GET /users/n1ywb-1/settings/billing/usage?year=2026&month=11`
- Branch / current commit: `wasm` / `6048bbeebd482474e5cf15b7b37ab8bb947de08f`
- Raw usage-item count: `42`
- Gross / discount / net: `$212.991372361` / `$203.534970181` / `$9.456402180`
- Remaining balance: `UNAVAILABLE` — the usage endpoint did not expose a budget or remaining-credit balance.
- Querying agent / model / reasoning effort: GitHub Copilot / MAI Code 2 / unavailable
- Product/SKU rollup:

| Product | SKU | Quantity | Unit | Gross | Discount | Net |
| --- | --- | ---: | --- | ---: | ---: | ---: |
| actions | Actions Linux | 537 | Minutes | $3.222000000 | $3.222000000 | $0.000000000 |
| actions | Actions storage | 11.487664919 | GigabyteHours | $0.003859710 | $0.003859710 | $0.000000000 |
| copilot | Copilot AI Credits | 20766.637050348 | AICredits | $207.666370400 | $199.078044712 | $8.588325688 |
| copilot | Copilot Cloud Agent | 178.646978750 | AICredits | $1.786469776 | $0.918393284 | $0.868076492 |
| sandbox | Sandbox Linux | 8888.610073526 | ComputeSeconds | $0.213326638 | $0.213326638 | $0.000000000 |
| sandbox | Sandbox Memory | 33115.280693845 | GigabyteSeconds | $0.099345837 | $0.099345837 | $0.000000000 |

<!-- Co-authored by GitHub Copilot. Model: MAI Code 2; reasoning-effort setting unavailable. -->

## 2026-11-20: Add accounting lifecycle policy — pre-commit

- UTC timestamp: `2026-11-20T23:06:06Z`
- Checkpoint: `pre-commit`
- Account: `n1ywb-1`
- Billing source: `GET /users/n1ywb-1/settings/billing/usage?year=2026&month=11`
- Branch / current commit: `wasm` / `6048bbeebd482474e5cf15b7b37ab8bb947de08f`
- Staged paths: `.github/agents/development-orchestrator.agent.md`, `.github/copilot-instructions.md`, `docs/accounting/agent-usage-ledger.md`
- Resulting local commit: `PENDING`
- Raw usage-item count: `42`
- Gross / discount / net: `$212.991372361` / `$203.534970181` / `$9.456402180`
- Remaining balance: `UNAVAILABLE`
- Delta from preceding snapshot: gross `$0.000000000`, discount `$0.000000000`, net `$0.000000000` (billing values unchanged)
- Querying agent / model / reasoning effort: GitHub Copilot / MAI Code 2 / unavailable
