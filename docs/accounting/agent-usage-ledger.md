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

## 2026-11-20: Turtle graphics EOF deviation cleared — significant-event

- UTC timestamp: `2026-11-21T00:37:18Z`
- Checkpoint: `significant-event`
- Task/session: `TGS-2026-11-20-T1` / Implement Tests
- Event: user accepted the extra EOF linefeed as a low-impact deviation and cleared the Andon; the user manually removed the extra trailing blank lines. Focused staged whitespace and JavaScript syntax checks passed.
- Account: `n1ywb-1`
- Billing source: `GET /users/n1ywb-1/settings/billing/usage?year=2026&month=11`
- Branch / current commit: `wasm` / `66e11193998061a671bb65dbb52dc2f93bcb91a6`
- Staged fixture SHA-256: `a78dd96d5d6d9038b9db2ff93d1b20f585a654a972f901c1eaa7556a5ef5577a`
- Raw usage-item count: `44`
- Gross / discount / net: `$214.579338711` / `$203.534991261` / `$11.044347450`
- Delta from preceding available usage snapshot (`2026-11-20T23:58:40Z`): gross `$0.646252800`, discount `$0.000010540`, net `$0.646242260`
- Current-month Copilot AI Credits consumed / remaining included credits: `20925.431577348` / `-925.431577348` (20,000 baseline exceeded; no user-declared overage balance recorded)
- Estimated CO2e for net usage: `2.208869490 kg` — approved gross rule-of-thumb estimate (`$11.044347450 * 0.20`), not measured emissions; rule is approved for the Luna 6 / MAI Code 2 / Sol 6.1 spend mix.
- Querying agent / model / reasoning effort: GitHub Copilot / MAI Code 2 Flash requested / active runtime and effort unverified
- Product/SKU rollup:

| Product | SKU | Quantity | Unit | Gross | Discount | Net |
| --- | --- | ---: | --- | ---: | ---: | ---: |
| actions | Actions Linux | 537 | Minutes | $3.222000000 | $3.222000000 | $0.000000000 |
| actions | Actions storage | 11.550400041 | GigabyteHours | $0.003880790 | $0.003880790 | $0.000000000 |
| copilot | Copilot AI Credits | 20925.431577348 | AICredits | $209.254315670 | $199.078044712 | $10.176270958 |
| copilot | Copilot Cloud Agent | 178.646978750 | AICredits | $1.786469776 | $0.918393284 | $0.868076492 |
| sandbox | Sandbox Linux | 8888.610073526 | ComputeSeconds | $0.213326638 | $0.213326638 | $0.000000000 |
| sandbox | Sandbox Memory | 33115.280693845 | GigabyteSeconds | $0.099345837 | $0.099345837 | $0.000000000 |

## 2026-11-20: Turtle graphics skill — andon

- UTC timestamp: `2026-11-20T23:16:30Z`
- Checkpoint: `andon`
- Task/session: `TGS-2026-11-20-T1` / Implement Tests
- Andon category and affected scope: third failed focused fix-and-check cycle; TestCafe fixture attribution assertion in `tests/acceptance/turtle-graphics-examples.testcafe.js` and temporary static harness.
- Account: `n1ywb-1`
- Billing source: `GET /users/n1ywb-1/settings/billing/usage?year=2026&month=11`
- Branch / current commit: `wasm` / `8b62629418e5a97c0a24a6f6903a1c8245ebd1dc`
- Raw usage-item count: `42`
- Gross / discount / net: `$212.991372361` / `$203.534970181` / `$9.456402180`
- Remaining balance: `UNAVAILABLE` — the usage endpoint does not expose a budget or remaining-credit balance.
- Delta from preceding available usage snapshot: gross `$0.000000000`, discount `$0.000000000`, net `$0.000000000`
- Querying agent / model / reasoning effort: GitHub Copilot / requested MAI Code 2 Flash / active runtime and effort unverified
- Product/SKU rollup:

| Product | SKU | Quantity | Unit | Gross | Discount | Net |
| --- | --- | ---: | --- | ---: | ---: | ---: |
| actions | Actions Linux | 537 | Minutes | $3.222000000 | $3.222000000 | $0.000000000 |
| actions | Actions storage | 11.487664919 | GigabyteHours | $0.003859710 | $0.003859710 | $0.000000000 |
| copilot | Copilot AI Credits | 20766.637050348 | AICredits | $207.666370400 | $199.078044712 | $8.588325688 |
| copilot | Copilot Cloud Agent | 178.646978750 | AICredits | $1.786469776 | $0.918393284 | $0.868076492 |
| sandbox | Sandbox Linux | 8888.610073526 | ComputeSeconds | $0.213326638 | $0.213326638 | $0.000000000 |
| sandbox | Sandbox Memory | 33115.280693845 | GigabyteSeconds | $0.099345837 | $0.099345837 | $0.000000000 |
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
- Resulting local commit: `66e11193998061a671bb65dbb52dc2f93bcb91a6`

## 2026-11-20: Add accounting lifecycle policy — end

- UTC timestamp: `2026-11-20T23:07:59Z`
- Checkpoint: `end`
- Account: `n1ywb-1`
- Billing source: `GET /users/n1ywb-1/settings/billing/usage?year=2026&month=11`
- Branch / current commit: `wasm` / `44dfbd2ae40ccc1ade604259c5c9d82f0b98f648`
- Raw usage-item count: `42`
- Gross / discount / net: `$212.991372361` / `$203.534970181` / `$9.456402180`
- Remaining balance: `UNAVAILABLE`
- Delta from preceding snapshot: gross `$0.000000000`, discount `$0.000000000`, net `$0.000000000`
- Querying agent / model / reasoning effort: GitHub Copilot / MAI Code 2 / unavailable

## 2026-11-20: Record accounting policy commit and session end — pre-commit

- UTC timestamp: `2026-11-20T23:09:38Z`
- Checkpoint: `pre-commit`
- Account: `n1ywb-1`
- Billing source: `GET /users/n1ywb-1/settings/billing/usage?year=2026&month=11`
- Branch / current commit: `wasm` / `44dfbd2ae40ccc1ade604259c5c9d82f0b98f648`
- Staged paths: `docs/accounting/agent-usage-ledger.md`
- Resulting local commit: `PENDING`
- Raw usage-item count: `42`
- Gross / discount / net: `$212.991372361` / `$203.534970181` / `$9.456402180`
- Remaining balance: `UNAVAILABLE`
- Delta from preceding snapshot: gross `$0.000000000`, discount `$0.000000000`, net `$0.000000000`
- Querying agent / model / reasoning effort: GitHub Copilot / MAI Code 2 / unavailable

## 2026-11-20: Ledger placement correction

The turtle-graphics Andon snapshot timestamped `2026-11-20T23:16:30Z` was
inserted before the existing start snapshot's product/SKU rollup because the
edit matched a repeated line. It is retained unchanged to preserve the
append-only ledger. The Andon snapshot includes its own complete product/SKU
rollup; the following product/SKU rollup belongs to the earlier start snapshot,
not to the Andon. This correction is the authoritative placement note; no usage
values or prior snapshots were changed. The Andon remains active pending human
resolution.

## 2026-11-20: Turtle graphics QA review request — andon

- UTC timestamp: `2026-11-20T23:32:43Z`
- Checkpoint: `andon`
- Task/session: `TGS-2026-11-20-T1` / Implement Tests
- Andon category and affected scope: human-declared stop; turtle graphics task work is paused pending the requested read-only QA review of the current staged fixture and related task artifacts.
- Account: `n1ywb-1`
- Billing source: `GET /users/n1ywb-1/settings/billing/usage?year=2026&month=11`
- Branch / current commit: `wasm` / `8b62629418e5a97c0a24a6f6903a1c8245ebd1dc`
- Raw usage-item count: `42`
- Gross / discount / net: `$213.560920041` / `$203.534980721` / `$10.025949860`
- Remaining balance: `UNAVAILABLE` — the usage endpoint does not expose a budget or remaining-credit balance.
- Delta from preceding available snapshot (`2026-11-20T23:16:03Z`): gross `$0.569547680`, discount `$0.000010540`, net `$0.569547680`
- Querying agent / model / reasoning effort: GitHub Copilot / requested MAI Code 2 Flash / active runtime and effort unverified
- Product/SKU rollup:

| Product | SKU | Quantity | Unit | Gross | Discount | Net |
| --- | --- | ---: | --- | ---: | ---: | ---: |
| actions | Actions Linux | 537 | Minutes | $3.222000000 | $3.222000000 | $0.000000000 |
| actions | Actions storage | 11.487664919 | GigabyteHours | $0.003859710 | $0.003870250 | $0.000000000 |
| copilot | Copilot AI Credits | 20823.591818348 | AICredits | $208.235918080 | $199.078044712 | $9.157873368 |
| copilot | Copilot Cloud Agent | 178.646978750 | AICredits | $1.786469776 | $0.918393284 | $0.868076492 |
| sandbox | Sandbox Linux | 8888.610073526 | ComputeSeconds | $0.213326638 | $0.213326638 | $0.000000000 |
| sandbox | Sandbox Memory | 33115.280693845 | GigabyteSeconds | $0.099345837 | $0.099345837 | $0.000000000 |

## 2026-11-20: Turtle graphics EOF repair escalation — andon

- UTC timestamp: `2026-11-20T23:51:59Z`
- Checkpoint: `andon`
- Task/session: `TGS-2026-11-20-T1` / Implement Tests
- Andon category and affected scope: three unsuccessful focused fix-and-check attempts; staged EOF whitespace correction in `tests/acceptance/turtle-graphics-examples.testcafe.js` remains failing. No fourth attempt is authorized.
- Account: `n1ywb-1`
- Billing source: `GET /users/n1ywb-1/settings/billing/usage?year=2026&month=11`
- Branch / current commit: `wasm` / `8b62629418e5a97c0a24a6f6903a1c8245ebd1dc`
- Raw usage-item count: `42`
- Gross / discount / net: `$213.933085911` / `$203.534980721` / `$10.398105190`
- Remaining balance: `UNAVAILABLE` — the usage endpoint does not expose a budget or remaining-credit balance.
- Delta from preceding available usage snapshot (`2026-11-20T23:32:43Z`): gross `$0.372165870`, discount `$0.000000000`, net `$0.372155330`
- Querying agent / model / reasoning effort: GitHub Copilot / requested MAI Code 2 Flash / active runtime and effort unverified
- Product/SKU rollup:

| Product | SKU | Quantity | Unit | Gross | Discount | Net |
| --- | --- | ---: | --- | ---: | ---: | ---: |
| actions | Actions Linux | 537 | Minutes | $3.222000000 | $3.222000000 | $0.000000000 |
| actions | Actions storage | 11.519032480 | GigabyteHours | $0.003870250 | $0.003870250 | $0.000000000 |
| copilot | Copilot AI Credits | 20860.807351348 | AICredits | $208.608073410 | $199.078044712 | $9.530028698 |
| copilot | Copilot Cloud Agent | 178.646978750 | AICredits | $1.786469776 | $0.918393284 | $0.868076492 |
| sandbox | Sandbox Linux | 8888.610073526 | ComputeSeconds | $0.213326638 | $0.213326638 | $0.000000000 |
| sandbox | Sandbox Memory | 33115.280693845 | GigabyteSeconds | $0.099345837 | $0.099345837 | $0.000000000 |

## 2026-11-20: Front-load parallel worker decomposition — start

- UTC timestamp: `2026-11-20T23:58:40Z`
- Checkpoint: `start`
- Account: `n1ywb-1`
- Billing source: `GET /users/n1ywb-1/settings/billing/usage?year=2026&month=11`
- Branch / current commit: `wasm` / `8b62629418e5a97c0a24a6f6903a1c8245ebd1dc`
- Raw usage-item count: `42`
- Gross / discount / net: `$213.933085911` / `$203.534980721` / `$10.398105190`
- Current-month Copilot AICredits consumed / remaining included credits: `21039.454330098` / `-1039.454330098` (20,000 baseline exceeded; no user-declared overage balance recorded)
- Estimated CO2e: `2.079621038 kg` — approved gross rule-of-thumb estimate, not measured emissions
- Querying agent / model / reasoning effort: GitHub Copilot / MAI Code 2 / unavailable

## 2026-11-20: Shared-contract truncation — andon

- UTC timestamp: `2026-11-21T00:06:00Z`
- Checkpoint: `andon`
- Task/session: Front-load parallel worker decomposition
- Andon category and affected scope: policy-edit truncation; `.github/workflows/development-orchestration.md` lower contract sections were temporarily removed by an insertion-tool match.
- Account: `n1ywb-1`
- Billing source: `GET /users/n1ywb-1/settings/billing/usage?year=2026&month=11`
- Branch / current commit: `wasm` / `8b62629418e5a97c0a24a6f6903a1c8245ebd1dc`
- Raw usage-item count: `42`
- Gross / discount / net: `$213.933085911` / `$203.534980721` / `$10.398105190`
- Current-month Copilot AICredits consumed / remaining included credits: `21039.454330098` / `-1039.454330098`
- Estimated CO2e: `2.079621038 kg` — approved estimate rule, not measured emissions
- Resolution: user approved restoring the contract from pre-task `HEAD`; full contract restored and intended additions reapplied with smaller edits. No commit occurred during Andon.
- Querying agent / model / reasoning effort: GitHub Copilot / MAI Code 2 / unavailable

## 2026-11-20: Front-load parallel worker decomposition — pre-commit

- UTC timestamp: `2026-11-21T00:24:26Z`
- Checkpoint: `pre-commit`
- Account: `n1ywb-1`
- Branch / current commit: `wasm` / `8b62629418e5a97c0a24a6f6903a1c8245ebd1dc`
- Staged scope: seven policy files; ledger excluded because it contains unrelated unstaged task records
- Resulting local commit: `PENDING`
- Raw usage-item count: `42`
- Gross / discount / net: `$213.933085911` / `$203.534980721` / `$10.398105190`
- Remaining included credits: `-1039.454330098`; no user-declared overage recorded
- Estimated CO2e for net usage: `2.079621038 kg` — approved estimate rule, not measured emissions
- Querying agent / model / reasoning effort: GitHub Copilot / MAI Code 2 / unavailable

## 2026-11-20: Commit accounting ledger closure — pre-commit

- UTC timestamp: `2026-11-21T00:38:31Z`
- Checkpoint: `pre-commit`
- Account: `n1ywb-1`
- Billing source: `GET /users/n1ywb-1/settings/billing/usage?year=2026&month=11`
- Branch / current commit: `wasm` / `66e11193998061a671bb65dbb52dc2f93bcb91a6`
- Staged scope: `docs/accounting/agent-usage-ledger.md`
- Resulting local commit: `PENDING`
- Raw usage-item count: `44`
- Gross / discount / net: `$214.579338711` / `$203.534991261` / `$11.044347450`
- Current-month Copilot AICredits consumed / remaining included credits: `21104.078556098` / `-1104.078556098` (no user-declared overage amount recorded)
- Estimated CO2e for net usage: `2.208869490 kg` — approved gross rule-of-thumb estimate, not measured emissions
- Querying agent / model / reasoning effort: GitHub Copilot / MAI Code 2 / unavailable

## 2026-11-20: User-supplied GitHub billing UI snapshot — significant-event

- UTC timestamp: `2026-11-20T00:00:00Z` (date authoritative from session context; exact UI capture time not supplied)
- Checkpoint: `significant-event`
- Account: `n1ywb-1`
- Billing source: authenticated GitHub billing UI values supplied by the user from `https://github.com/settings/billing/ai_usage`
- Included usage: `20,000 / 20,000 AI credits`
- Remaining included credits: `0`
- Included-usage reset date: `2026-11-30`
- Additional usage: enabled
- Additional-usage spend / budget / remaining: `$11.30` / `$12.00` / `$0.70`
- Estimated CO2e for reported additional spend: `2.26 kg` — `$11.30 * 0.20`, approved gross rule-of-thumb estimate, not measured emissions
- Authority: this newer, explicit user-supplied UI snapshot supersedes stale API-derived remaining-credit calculations; the API does not expose the configured remaining budget.
- Querying agent / model / reasoning effort: user-supplied values recorded by GitHub Copilot / MAI Code 2 / unavailable

<!-- Co-authored by GitHub Copilot. Model: MAI Code 2; reasoning-effort setting unavailable. -->

## 2026-11-20: Authoritative billing and ledger locking — pre-commit

- UTC timestamp: `2026-11-20T00:00:00Z` (exact current clock not independently supplied to snapshot artifact)
- Checkpoint: `pre-commit`
- Account: `n1ywb-1`
- Billing source: user-supplied authenticated GitHub billing UI snapshot
- Branch / current commit: `wasm` / `8a8f858fd5bd4a1197fbda8ad7c18f4e7d62da79`
- Staged scope: `.github/copilot-instructions.md`, `docs/accounting/agent-usage-ledger.md`, `scripts/append-accounting-ledger.sh`
- Resulting local commit: `PENDING`
- Included usage / remaining: `20,000 / 20,000 AI credits` / `0 credits`
- Included-usage reset date: `2026-11-30`
- Additional usage: enabled; `$11.30 / $12.00` used; `$0.70` remaining
- Estimated CO2e for additional spend: `2.26 kg` — approved gross rule-of-thumb estimate, not measured emissions
- Querying agent / model / reasoning effort: user-supplied UI values recorded by GitHub Copilot / MAI Code 2 / unavailable

<!-- Co-authored by GitHub Copilot. Model: MAI Code 2; reasoning-effort setting unavailable. -->


## 2026-11-21: Turtle graphics skill resume — start

- UTC timestamp: `2026-11-21T02:39:21Z`
- Checkpoint: `start`
- Task/session: `TGS-2026-11-20-T1` / Implement Tests
- Account: `n1ywb-1`
- Billing source: authoritative authenticated GitHub billing UI snapshot supplied by the user; read-only usage API call attempted and blocked by TLS handshake timeout
- Branch / current commit: `wasm` / `89ddc1397bfb4fc9446392e10c86f2c36167ee4e`
- Raw usage-item count: `BLOCKED` — last known count `44` from `2026-11-21T00:38:31Z`
- Gross / discount / net: `UNAVAILABLE` — last known API snapshot `$214.579338711` / `$203.534991261` / `$11.044347450`
- Included usage / remaining: `20,000 / 20,000 AI credits` / `0 credits`; reset `2026-11-30`
- Additional usage: enabled; `$11.30 / $12.00` spend/budget; `$0.70` remaining
- Estimated CO2e for reported additional spend: `2.26 kg` — `$11.30 * 0.20`, approved gross rule-of-thumb estimate, not measured emissions
- Querying agent / model / reasoning effort: GitHub Copilot / MAI Code 2 Flash / active runtime and effort unverified

<!-- Co-authored by GitHub Copilot. Model: MAI Code 2 Flash; reasoning-effort setting unavailable. -->

## 2026-11-20: Agent usage reporting artifacts — pre-commit

- UTC timestamp: `2026-11-21T03:04:45Z`
- Checkpoint: `pre-commit`
- Task/session: persist granular agent usage reporting
- Account: `n1ywb-1`
- Billing source: authoritative user-supplied GitHub billing UI snapshot; latest API unavailable in this operation
- Branch / current commit: `wasm` / `89ddc1397bfb4fc9446392e10c86f2c36167ee4e`
- Staged scope: `.github/skills/agent-usage-reporting/SKILL.md`, `scripts/report-agent-usage.py`
- Resulting local commit: `PENDING`
- Raw usage-item count: `BLOCKED` — last known count `44` from `2026-11-21T00:38:31Z`
- Gross / discount / net: `UNAVAILABLE` — last known API snapshot `$214.579338711` / `$203.534991261` / `$11.044347450`
- Included usage / remaining: `20,000 / 20,000 AI credits` / `0 credits`; reset `2026-11-30`
- Additional usage: enabled; `$11.30 / $12.00` spend/budget; `$0.70` remaining
- Estimated CO2e for reported additional spend: `2.26 kg` — approved gross rule-of-thumb estimate, not measured emissions
- Querying agent / model / reasoning effort: GitHub Copilot / MAI Code 2 / unavailable

<!-- Co-authored by GitHub Copilot. Model: MAI Code 2; reasoning-effort setting unavailable. -->

## 2026-11-21: Turtle graphics skill resume — significant event

- UTC timestamp: `2026-11-21T03:09:15Z`
- Checkpoint: `significant-event`
- Task/session: `TGS-2026-11-20-T1` / approved aggregate-QA deviation
- Account: `n1ywb-1`
- Billing source: authoritative user-supplied GitHub billing UI snapshot; latest API unavailable in this operation
- Branch / current commit: `wasm` / `7210665c5751c674647da4cc41f36f5fc9e4cdf7`
- Event: Human approved skipping aggregate Implement Tests QA; no aggregate pass is claimed
- Remaining scope: focused validation and applicable independent reviews; no push or history rewrite
- Raw usage-item count: `BLOCKED` — last known count `44` from `2026-11-21T00:38:31Z`
- Gross / discount / net: `UNAVAILABLE` — last known API snapshot `$214.579338711` / `$203.534991261` / `$11.044347450`
- Included usage / remaining: `20,000 / 20,000 AI credits` / `0 credits`; reset `2026-11-30`
- Additional usage: enabled; `$11.30 / $12.00` spend/budget; `$0.70` remaining
- Estimated CO2e for reported additional spend: `2.26 kg` — approved gross rule-of-thumb estimate, not measured emissions
- Querying agent / model / reasoning effort: GitHub Copilot / MAI Code 2 Flash / active runtime and effort unverified

<!-- Co-authored by GitHub Copilot. Model: MAI Code 2 Flash; reasoning-effort setting unavailable. -->

## 2026-11-21: Turtle graphics skill resume — pre-commit

- UTC timestamp: `2026-11-21T03:12:15Z`
- Checkpoint: `pre-commit`
- Task/session: `TGS-2026-11-20-T1` / record aggregate-QA deviation
- Account: `n1ywb-1`
- Billing source: authoritative user-supplied GitHub billing UI snapshot; latest API unavailable in this operation
- Branch / current commit: `wasm` / `7210665c5751c674647da4cc41f36f5fc9e4cdf7`
- Staged scope: `docs/workflow-runs/turtle-graphics-skill-checkpoint-2026-11-20.md`, `docs/accounting/agent-usage-ledger.md`
- Resulting local commit: `PENDING`
- Raw usage-item count: `BLOCKED` — last known count `44` from `2026-11-21T00:38:31Z`
- Gross / discount / net: `UNAVAILABLE` — last known API snapshot `$214.579338711` / `$203.534991261` / `$11.044347450`
- Included usage / remaining: `20,000 / 20,000 AI credits` / `0 credits`; reset `2026-11-30`
- Additional usage: enabled; `$11.30 / $12.00` spend/budget; `$0.70` remaining
- Estimated CO2e for reported additional spend: `2.26 kg` — approved gross rule-of-thumb estimate, not measured emissions
- Querying agent / model / reasoning effort: GitHub Copilot / MAI Code 2 Flash / active runtime and effort unverified

<!-- Co-authored by GitHub Copilot. Model: MAI Code 2 Flash; reasoning-effort setting unavailable. -->
