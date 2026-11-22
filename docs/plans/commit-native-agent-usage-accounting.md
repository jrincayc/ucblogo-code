# Plan: Commit-Native Agent Usage Accounting

Replace the ledger with a canonical, validated accounting block appended to each Git commit message. Every commit becomes the source of truth; humans receive concise readable totals and automation receives one strictly delimited JSON object. PR descriptions include full cumulative breakdowns, while issue updates summarize only costs associated with linked commits.

## Canonical format

Commit messages end with two sections in a fixed order:

1. **Human-readable summary** with currency formatted to exactly two decimals.
2. **Machine-readable block** delimited by exact markers:
   - `COPILOT-ACCOUNTING-BEGIN v1`
   - exactly one UTF-8 JSON object, with no comments or trailing text
   - `COPILOT-ACCOUNTING-END`

The JSON contract should be versioned and contain stable, typed fields: schema version; checkpoint/editor/account attribution; UTC generation time; parent revision; usage reporting window; billing provenance and availability; gross, discount, net, included credits, and remaining budget; product/SKU totals; per-model totals when available; CO2e kilograms and methodology/provenance; commit or PR/issue relationship fields; and a deterministic privacy-safe task identifier. Monetary values use decimal strings with exactly two fractional digits. Unknown data is `null` with an explicit `reason`—never zero or an invented estimate. JSON field order is fixed and output uses canonical serialization.

Human formatting example semantics (not implementation code):

- `Copilot usage: gross $12.00; discount $1.25; net $10.75.`
- `Estimated CO2e: 2.15 kg (0.20 kg/$ approved estimate; not measured).`
- `Additional budget: $0.70 remaining.`

## Steps

### Phase 1 — Contract and migration design

1. Define the versioned JSON Schema, marker grammar, canonical field ordering, decimal-string rules, timestamp rules, null/reason rules, and supported checkpoint/editor/availability enums.
2. Build a strict parser that accepts only one accounting block, requires exact markers, rejects duplicate or malformed blocks, and separates commit subject/body from accounting metadata without substring ambiguity.
3. Define a commit-msg lifecycle: `pre-commit` gathers and validates staged/task context; `commit-msg` appends the canonical accounting block because the resulting commit hash still does not exist. A `post-commit` operation may maintain a local non-authoritative reconciliation cache, but must not rewrite commit messages.
4. Audit the current ledger and classify every known corruption as reconciled from Git/API/user-supplied evidence or explicitly unresolved. Preserve a read-only migration report/checksum during transition; do not silently reinterpret malformed historical values.
5. Delete `docs/accounting/agent-usage-ledger.md` only after the parser, migration audit, hook integration, and representative commit/PR reports pass. Remove ledger-specific locking and reporting code after all consumers migrate.

### Phase 2 — Commit hook and local source of truth (parallel worker packages)

6. **Worker A — Schema/parser:** own JSON Schema, canonical serializer, strict parser, marker constants, fixtures, and unit tests. Reject shell-expanded values and unknown fields.
7. **Worker B — Accounting collector:** own billing/user-supplied snapshot normalization, product/SKU/model rollups, editor/task identity, privacy filtering, and unavailable-state handling. Never store prompts, responses, tokens, credentials, or payment data.
8. **Worker C — Git hook integration:** own managed pre-commit/commit-msg installer, staged-path capture, literal-safe file-backed handoff, append/replace validation, amend behavior, merge behavior, and temporary-file cleanup.
9. **Worker D — Migration/audit:** own the ledger corruption inventory and read-only migration report. Map every historical incident to verified evidence or `UNRESOLVED`; do not edit or normalize the historical ledger in place.
10. **Worker E — PR/issue reporting:** own parsing related commit ranges, full PR totals/breakdowns, issue-update concise summaries, deduplication, and unavailable-cost handling.

Dependencies: B and A converge on a typed record interface; C consumes that record only after A’s serializer tests pass. E depends on A’s parser. D can run in parallel with A–C. Independent QA starts after A–E integration.

### Phase 3 — Hook implementation safeguards

11. Transfer large/multi-line data only through named files and APIs. Workers must never use heredocs, inline `printf`/`echo`, command substitution, or shell-argument interpolation to construct accounting content.
12. The hook generator writes a candidate accounting file with a file-edit/library API; the commit-msg hook reads and validates the file, then serializes it atomically into the message. Validate before and after commit-message modification.
13. Amend/rebase/cherry-pick behavior must be explicit: preserve valid existing accounting blocks, prevent duplicates, and require a verified refresh only when the user requests it or the staged/task scope materially changes.
14. Do not block unrelated commits merely because billing data is unavailable. Emit a valid block with `null` values and reasons. Block when the block is malformed, duplicated, privacy-unsafe, or dollar-shell-expanded.
15. Currency is always exactly two decimal places in human and JSON output. CO2e is also fixed to two decimals with methodology and “estimated/not measured” classification. No estimate is computed from elapsed time alone.

### Phase 4 — PR and issue reporting

16. PR generation aggregates all commits in the exact PR base-to-head range, deduplicating commit accounting IDs. Include commit count, reporting window, gross, discount, net, included-credit use, remaining budget, product/SKU and model breakdowns, data availability/caveats, and estimated CO2e for net monetary usage.
17. Issue updates parse linked/related commit IDs from the issue context and show a brief summary: commit count, net cost, budget remaining if available, estimated CO2e, and caveat that unknown values remain unavailable. Do not paste the full PR breakdown unless requested.
18. Every aggregation labels account-level totals as temporal attribution, not causal proof that a commit caused the usage, and distinguishes measured API data from authoritative user-supplied snapshots.
19. Add explicit output templates for PR descriptions, PR updates, issue comments, and concise fallback messages when related commits lack valid accounting blocks.

### Phase 5 — Whitespace and worker-process fixes

20. Fix PostToolUse eligibility filtering so ineligible paths such as `/memories/session/plan.md` are excluded before invoking the workspace formatter; no “skipped formatting” warning should be generated for expected exclusions.
21. Keep formatter boundary checks as defense-in-depth. Preserve the append-only ledger’s historical bytes during migration; the formatter must never rewrite them.
22. Add regression cases for outside-workspace files, unsupported files, deleted paths, generated/protected files, ledger migration, dollar signs, backticks, control characters, duplicate markers, and shell expansion such as `$11.30` becoming unintended text.

### Phase 6 — Rollout and ledger retirement

23. Enable the new hook in a dry-run mode, validate representative commits, and compare parser results against the audited ledger.
24. Switch reporting to commits and Git history; verify last-N-commit, PR-range, and linked-issue reports agree on totals.
25. Require exact migration approval, then delete the ledger and remove ledger-specific lock/appender/reporting instructions. Historical ledger evidence may remain only in Git history; no replacement ledger file is created.
26. Update `.github/skills/agent-usage-reporting/SKILL.md` and related worker/orchestrator guidance. Updating `.github/copilot-instructions.md` remains gated on an exact proposed diff and explicit approval.

## Relevant files

- `docs/accounting/agent-usage-ledger.md` — audit first, then delete; never used by new reports.
- `scripts/report-agent-usage.py` — replace ledger parsing with Git commit-message parsing.
- `scripts/append-accounting-ledger.sh` — remove after migration.
- `.github/hooks/format-whitespace.js` — fix pre-invocation eligibility selection and related tests.
- `.github/skills/agent-usage-reporting/SKILL.md` — define commit, PR, and issue accounting workflows.
- `.github/copilot-instructions.md` — approval-gated policy update.
- Managed Git hook installer plus pre-commit/commit-msg implementation — exact paths determined during implementation.

## Verification

1. Schema tests verify canonical JSON, exact markers, fixed field order, required types, decimal-string precision, deterministic serialization, duplicate-block rejection, and malformed/truncated block rejection.
2. Fuzz dollar signs, backticks, control characters, Unicode, newlines, command-substitution strings, and malicious marker text; confirm no shell interpretation and no ambiguous parser result.
3. Test human and machine views independently, then parse each generated commit and assert both views carry matching totals.
4. In a temporary repository, test normal commits, amend, merge commits, unavailable billing, duplicate invocation, and concurrent hooks; verify one valid accounting block per commit.
5. Validate PR range aggregation and issue commit subsets against fixtures with measured, user-supplied, blocked, and unavailable data.
6. Verify all reported currency is exactly two decimal places and CO2e is clearly estimated with method/provenance.
7. Run privacy tests, `git diff --check`, focused hook/reporting tests, and a migration dry run. Application builds/browser tests are out of scope.

## Decisions

- Git commit messages are the only ongoing accounting source; the ledger is retired and deleted after migration.
- Human prose and canonical JSON coexist in one versioned, marker-delimited block; prose is never parsed as the accounting source.
- The accounting block cannot contain a current commit hash, because `commit-msg` runs before the commit object exists.
- Currency uses two decimal places. CO2e is reported to two decimals and labeled as estimated/not measured when modeled.
- Unknown values are `null` with reasons; unavailable is not zero.
- PRs provide full cost breakdowns; issues provide brief linked-commit summaries.
- No implementation is authorized by this plan alone.

## Further considerations

1. Prefer plain canonical JSON over base64 so both humans and tooling can inspect it; exact markers remove fence/format ambiguity.
2. Keep the human summary intentionally redundant and validate it against JSON to catch drift, but never infer machine values from prose.
3. Before deleting the ledger, preserve a checksum and migration report in the implementation commit/commit accounting metadata so auditors can locate the prior history in Git.

## Parallel Implementation Map

### Scheduling controls

- Task-scoped deviation: medium effort and medium-effort language. The map is concise and complete, not rushed or under-decomposed.
- Host model and reasoning-effort metadata are unavailable and must not be claimed.
- Base: `0b763a9241a0d8a2a48dd8952ca9896df008ca03`, branch `wasm`. This map does not authorize implementation or bypass the required stage gates.
- Worktrees: `/tmp/copilot/CNAA-2026-11-22/wt-<packet>/`; integration: `/tmp/copilot/CNAA-2026-11-22/integration/`. The orchestrator owns integration, and no two workers edit the same file.
- Packet owners run focused validation and return patch evidence. Development Reviewer reviews exact packet or staged content. Development Lead owns integration and commits; Development QA gives one aggregate stage verdict; no worker advances a gate alone.

### Test Plan handoff and test packets

| Packet | Worktree / exact ownership | Inputs, dependencies, readiness | Focused validation and exact output | Reviewer / commit owner |
|---|---|---|---|---|
| `CNAA-TP-01` | `/tmp/copilot/CNAA-2026-11-22/wt-CNAA-TP-01/`; `docs/plans/commit-native-agent-usage-accounting-test-plan.md` | Revised Plan QA and explicit human Plan approval; serial. | Create only the approved Test Plan: cases, fixtures, setup/teardown, commands, and regression scope. No test or implementation code. | Development QA; Development Lead. |
| `CNAA-T-A1` | `/tmp/copilot/CNAA-2026-11-22/wt-CNAA-T-A1/`; `scripts/copilot-accounting-contract.test.js`, `tests/fixtures/copilot-accounting-v1/` | Approved TP; ready and parallel with B1/D1/E1/F1. | `node --test scripts/copilot-accounting-contract.test.js`: field order/types, decimal strings, null reasons, exact markers, malformed/duplicate blocks, shell/marker fuzz. | Development Reviewer; Development Lead. |
| `CNAA-T-B1` | `/tmp/copilot/CNAA-2026-11-22/wt-CNAA-T-B1/`; `scripts/copilot-accounting-collector.test.js`, `tests/fixtures/copilot-accounting-collector/` | Approved TP and frozen typed-record contract; ready and parallel with A1/D1/E1/F1. | `node --test scripts/copilot-accounting-collector.test.js`: provenance, rollups, unavailable state, privacy filtering, deterministic identity, no secret/prompt persistence. | Development Reviewer; Development Lead. |
| `CNAA-T-C1` | `/tmp/copilot/CNAA-2026-11-22/wt-CNAA-T-C1/`; `.github/hooks/copilot-accounting.test.js` | A1/B1 contracts pass; blocked until then, serial after both. | `node --test .github/hooks/copilot-accounting.test.js`: normal/amend/merge, unavailable, duplicate/concurrent invocation, invalid/privacy failures, exactly one block. | Development Reviewer; Development Lead. |
| `CNAA-T-D1` | `/tmp/copilot/CNAA-2026-11-22/wt-CNAA-T-D1/`; `scripts/migrate-agent-usage-ledger.test.py`, `tests/fixtures/ledger-migration/` | Approved TP; synthetic fixtures; ready and parallel. | `python3 scripts/migrate-agent-usage-ledger.test.py`: reconciled versus `UNRESOLVED`, checksum/report, malformed history. | Development Reviewer; Development Lead. |
| `CNAA-T-E1` | `/tmp/copilot/CNAA-2026-11-22/wt-CNAA-T-E1/`; `scripts/report-agent-usage.test.py`, `tests/fixtures/cnaa-reports/` | Approved TP plus A1 parser contract; ready and parallel. | `python3 scripts/report-agent-usage.test.py`: commit range, deduplication, provenance, unavailable fallback, PR totals, issue summary. | Development Reviewer; Development Lead. |
| `CNAA-T-F1` | `/tmp/copilot/CNAA-2026-11-22/wt-CNAA-T-F1/`; `.github/hooks/format-whitespace.test.js`, `scripts/format-whitespace.test.js` | Approved TP; ready and parallel. | `node --test scripts/format-whitespace.test.js .github/hooks/format-whitespace.test.js`: outside/deleted/generated/protected paths, ledger text, special-character strings, no expected-exclusion warning. | Development Reviewer; Development Lead. |

### Implementation packets

| Packet | Worktree / exact ownership | Inputs, dependencies, readiness | Focused validation and exact output | Reviewer / commit owner |
|---|---|---|---|---|
| `CNAA-I-A1` | `/tmp/copilot/CNAA-2026-11-22/wt-CNAA-I-A1/`; `docs/schemas/copilot-accounting-v1.schema.json`, `scripts/copilot-accounting-contract.js` | Red A1 test passes as red baseline; implementation Wave 1. | `node --test scripts/copilot-accounting-contract.test.js`: versioned schema, fixed canonical fields, serializer/parser, exact markers, decimal/null rules, special-character safety. | Development Reviewer; Development Lead. |
| `CNAA-I-B1` | `/tmp/copilot/CNAA-2026-11-22/wt-CNAA-I-B1/`; `scripts/copilot-accounting-collector.js` | Red B1 test passes as red baseline; frozen I-A1 typed-record interface; implementation Wave 1. | `node --test scripts/copilot-accounting-collector.test.js`: normalized record, API/UI provenance, rollups, privacy filter, deterministic identity, unavailable nulls/reasons. | Development Reviewer; Development Lead. |
