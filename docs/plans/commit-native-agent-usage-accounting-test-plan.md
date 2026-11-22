# Test Plan: Commit-Native Agent Usage Accounting

## Authority and stage boundary

- Approved Plan: `docs/plans/commit-native-agent-usage-accounting.md`
- Approved Plan revision: `0b763a9241a0d8a2a48dd8952ca9896df008ca03`
- Controlling-plan approval: the human explicitly reconfirmed this committed revision after QA and directed that the uncommitted reopen artifacts do not control this execution.
- Branch: `wasm`
- Execution mode: serial because the human reported degraded network conditions and explicitly requested no parallel worker execution.
- This document specifies tests only. It does not authorize implementation, dependency installation, ledger deletion, pushes, or edits to `.github/copilot-instructions.md`.

## Objectives

Verify that every future Git commit can carry exactly one canonical, privacy-safe accounting block; that malformed or unsafe blocks are rejected; that unavailable billing remains explicit rather than becoming zero; and that commit, PR, and issue reports agree on the same machine-readable source.

The implementation must preserve the append-only ledger unchanged until migration validation passes. Application builds and browser tests are out of scope.

## Acceptance criteria

1. A canonical block is delimited by exact `COPILOT-ACCOUNTING-BEGIN v1` and `COPILOT-ACCOUNTING-END` lines and contains exactly one JSON object between them.
2. Serialization is deterministic: fixed field order, UTF-8, no comments, no trailing JSON text, exactly two fractional digits for monetary strings, and stable timestamps.
3. Parsing rejects duplicate blocks, malformed markers, truncated JSON, trailing JSON, unknown fields, missing fields, wrong types, invalid decimal strings, and ambiguous marker text in the subject or body.
4. Collection records measured versus user-supplied provenance and emits `null` plus a nonempty reason for unavailable values. It never invents zero, prompt, response, token, credential, or payment data.
5. Normal commits and amend/merge/cherry-pick-like flows produce one valid block. Unavailable billing does not block an otherwise valid commit.
6. The commit hook never interprets shell syntax. Dollar signs, backticks, command substitutions, newlines, control characters, Unicode, and marker-like strings remain literal or are rejected safely.
7. A migration audit maps every legacy ledger entry to reconciled evidence or `UNRESOLVED`, preserves a checksum, and never normalizes the historical ledger in place.
8. Git-range and linked-commit aggregations deduplicate accounting IDs, distinguish account-level temporal attribution from causation, and produce separate PR and issue formats.
9. Expected-exclusion paths do not invoke the prose formatter and produce no misleading skipped-formatting warning; formatter defenses still handle outside-workspace, deleted, unsupported, binary, generated, and protected paths.
10. Ledger retirement occurs only after representative parser, hook, migration, and report checks pass in a temporary repository.

## Test environment and isolation

- Run all focused tests from the repository root.
- Tests create temporary repositories and temporary data beneath the test runner's temporary directory, preferably `/tmp/copilot/<task-id>/` when invoked by orchestration.
- Each hook test initializes a fresh repository and configures a test-only identity. No test may modify the user's global Git configuration.
- Tests use synthetic fixture data and never call live billing endpoints or include secrets.
- File-backed handoffs are copied into a temporary repository. Tests assert cleanup of candidates and handoff files after success and failure.
- All concurrent-hook checks use a bounded count of two workers and isolated candidate files; no shared repository commit is expected to race without serialization.

## Compact fixture set

### `tests/fixtures/copilot-accounting-v1/`

| Fixture | Purpose |
|---|---|
| `valid-measured.json` | Complete measured record with product/SKU/model rollups and two-decimal money strings. |
| `valid-user-supplied.json` | Complete record whose totals are authoritative user-supplied UI values. |
| `valid-unavailable.json` | Complete schema with unavailable totals represented by `null` plus reasons. |
| `valid-human-summary.txt` | Expected human-readable summary corresponding to `valid-measured.json`. |
| `duplicate-block.txt` | Message containing two complete accounting blocks; must be rejected. |
| `malformed-matrix/` | Small files for missing markers, reordered markers, truncated JSON, trailing JSON, unknown field, missing field, wrong type, bad money, and bad timestamp. |
| `hostile-values.json` | Dollar signs, backticks, `$(...)`, backslash sequences, Unicode, newline, tabs, control-character escapes, and marker-like text encoded as JSON strings. |

Fixture values are synthetic. No fixture may contain a real token, credential, card data, billing URL, full prompt, response, or unnecessary personal data.

### `tests/fixtures/copilot-accounting-collector/`

- one measured API snapshot;
- one user-supplied snapshot;
- one partial API snapshot with missing rollup details;
- one unavailable snapshot;
- one privacy-negative payload containing prompt/response/token-like fields that must be removed or rejected according to the approved policy.

### `tests/fixtures/ledger-migration/`

- one well-formed legacy snapshot;
- one corrupt snapshot with a resolvable Git hash;
- one corrupt snapshot with no safe resolution;
- one duplicate snapshot;
- one snapshot with missing rollups;
- expected reconciliation classes, reasons, and deterministic migration checksum.

### `tests/fixtures/cnaa-reports/`

A three-commit range with:

- commit A measured usage;
- commit B authoritative user-supplied usage;
- commit C unavailable usage;
- one duplicate accounting ID on a cherry-pick-shaped fixture commit;
- expected PR totals, provenance caveats, and concise issue subset.

## Test cases

### A. Contract, schema, serializer, and parser

| ID | Input | Expected result |
|---|---|---|
| A1 | `valid-measured.json` | Canonical serialization succeeds; repeated serialization is byte-identical. |
| A2 | Valid record wrapped in exact markers | Parser returns the body before accounting separately and one typed accounting record. |
| A3 | Human summary plus valid block | Human totals are compared to JSON; matching values pass. Drift fails. |
| A4 | Each `malformed-matrix/` case | Each case fails with a stable category; no partial record is returned. |
| A5 | `duplicate-block.txt` | Failure names duplicate blocks, regardless of marker position. |
| A6 | Marker-like text inside JSON string | Parser accepts it as data when encoded safely; it does not misidentify block boundaries. |
| A7 | Marker-like text in subject/body outside an exact whole-line accounting suffix | Parser treats ordinary substrings such as `prefix COPILOT-ACCOUNTING-BEGIN v1 suffix` as body text. An exact begin/end marker line outside one terminal suffix is rejected as ambiguous. |
| A8 | `hostile-values.json` | JSON remains literal; no shell execution or value expansion occurs. |
| A9 | Reordered object keys with equivalent values | Canonical serializer emits the approved field order, while parser accepts only the declared schema semantics and not undeclared fields. |
| A10 | Money values `1`, `1.0`, `1.000`, negative values, exponent notation, NaN, and Infinity | Only nonnegative decimal strings with exactly two fractional digits are accepted. |
| A11 | Unknown, missing, nullable, and wrong-type fields | Unknown or missing fields fail; nullable fields require both a `null` value and an explicit reason. |
| A12 | Invalid UTF-8 and NUL bytes in candidate data | Candidate is rejected before commit-message modification. |
| A13 | Complete field fixture | Includes and validates parent revision, reporting relationship fields, and absence of a current commit hash; object order, UTF-8 bytes, and exactly two-decimal CO2e/method/provenance are asserted. |
| A14 | Marker lines with CRLF, BOM, trailing whitespace, or embedded exact marker line | Non-exact delimiters fail; marker text encoded inside JSON remains data. |

Required focused command: `node --test scripts/copilot-accounting-contract.test.js`.

Expected-red stage evidence: before `scripts/copilot-accounting-contract.js` and the schema exist, the suite uses a test-controlled skeletal module and fails at the first intended missing-behavior assertion, not at module loading, fixture setup, or syntax.

### B. Accounting collector and privacy

| ID | Input | Expected result |
|---|---|---|
| B1 | Measured snapshot fixture | Product/SKU/model rollups sum consistently to gross, discount, and net; provenance is `measured`. |
| B2 | User-supplied snapshot fixture | Values remain authoritative and are labeled `user-supplied`; stale API values do not overwrite them when the supplied snapshot is newer. |
| B3 | Partial or unavailable snapshot | Missing values remain `null`; each has a nonempty, bounded reason. |
| B4 | Privacy-negative payload | Forbidden fields are absent from the candidate record and do not appear in temporary files or diagnostics. |
| B5 | Nondeterministic ambient fields | Task ID and output are deterministic for equivalent task input; uncontrolled host details are excluded. |
| B6 | Included credit and budget values | Remaining values are derived from an explicitly supplied baseline/limit and never from an inferred recurring allowance. |
| B7 | CO2e input | Applies the approved monetary rule only to an available monetary amount; method and not-measured classification are included. Elapsed time alone does not produce an estimate. |
| B8 | Collector failure or unavailable billing endpoint | Emits a valid unavailable record when possible; logs only privacy-safe error categories. |

Required focused command: `node --test scripts/copilot-accounting-collector.test.js`.

### C. Hook lifecycle and commit safety

| ID | Scenario | Expected result |
|---|---|---|
| C1 | Normal commit with candidate file | `pre-commit` captures staged task context, `commit-msg` appends one block, and post-validation parses exactly one block. |
| C2 | Empty message or subject-only message | Normal Git validation continues; accounting validation neither creates an unintended message nor corrupts the subject. |
| C3 | `git commit --amend` with valid existing block | The block is preserved or refreshed only under the explicit refresh condition; no duplicate appears. |
| C4 | Merge-shaped commit | Merge messages remain readable and end in one valid block. |
| C5 | Cherry-pick/rebase-shaped existing metadata | Valid existing accounting identity is preserved; a new identity is not silently fabricated. |
| C6 | Billing unavailable | Commit succeeds with explicit unavailable fields and reasons. |
| C7 | Malformed candidate block | Commit is blocked before commit creation, with a concise actionable reason. |
| C8 | Privacy-unsafe candidate | Commit is blocked; forbidden values are not echoed in stderr. |
| C9 | Duplicate hook invocation | Idempotent result: still exactly one accounting block. |
| C10 | Two candidate writes targeting one message | File-backed locking or atomic replacement prevents interleaving and leaves either one valid candidate or a safe failure. |
| C11 | Candidate read/write failure | Commit fails safely; original message is preserved or restorable and temporary state is cleaned. |
| C12 | Dollar/backtick/control/Unicode payload | Hook code treats data as files, APIs, and literal content; no shell interpretation occurs. |
| C13 | `commit-msg` validation failure | The original commit message is not silently replaced by a partial block. |
| C14 | Stale candidate from an earlier commit | Hook rejects or removes stale context rather than attaching it to unrelated staged paths. |
| C15 | Managed hook installer run twice | Installation is idempotent, preserves compatible existing hooks, and fails safely for incompatible unmanaged hooks. |
| C16 | Dry-run enablement and uninstallation | Dry run performs no commit-message mutation; uninstallation restores the pre-test hook state. |
| C17 | Amend, rebase, and cherry-pick refresh decisions | Existing valid metadata is preserved unless staged/task scope materially changed or an explicit verified refresh was requested. |

Required focused command: `node --test .github/hooks/copilot-accounting.test.js`.

Prerequisite: A and B focused suites must pass their implementation contract before C implementation tests can be green.

### D. Legacy migration and audit

| ID | Input | Expected result |
|---|---|---|
| D1 | Well-formed snapshot | Classified as reconciled with evidence class and no value reinterpretation. |
| D2 | Malformed but Git-resolvable snapshot | Evidence and resolution are recorded; original historical text is unchanged. |
| D3 | Malformed and unresolvable snapshot | Classification is `UNRESOLVED` with a reason; migration does not invent totals. |
| D4 | Duplicate snapshot | Audit records duplication explicitly and avoids double-counting in the migration summary. |
| D5 | Missing rollup | Migration preserves unavailable detail and does not fabricate SKU/model rows. |
| D6 | Entire fixture ledger | Report checksum and entry count are stable across repeated audits. |
| D7 | Migration dry run against the actual ledger copy | It produces a report without changing source bytes; the pre/post SHA-256 values match. |
| D8 | Representative commit conversion fixtures | Converted accounting records parse through the v1 contract and retain provenance. |

The real-ledger copy used by D7 must establish the exact number of snapshots/entries considered, classify every entry as reconciled or `UNRESOLVED`, verify the expected report hash/checksum, and assert that no entry is omitted or silently normalized.

Required focused command: `python3 scripts/migrate-agent-usage-ledger.test.py`.

### E. Commit, PR, and issue reporting

| ID | Scenario | Expected result |
|---|---|---|
| E1 | Last-N valid commits | Exact range is selected; each accounting ID appears once. |
| E2 | PR base-to-head range | Output reports commit count, window, totals, remaining budget, provenance caveats, product/SKU/model details, and estimated CO2e. |
| E3 | Linked issue subset | Concise output includes selected commit count, net value, budget remainder when available, estimated CO2e, and unavailable caveat. |
| E4 | Duplicate accounting ID | The record contributes once; duplicate evidence is reported rather than hidden. |
| E5 | Missing accounting block | Fallback names the affected commit and marks its cost unavailable; report still completes for valid peers. |
| E6 | Malformed block | PR and issue modes both fail closed with exit status 1, identify the exact commit hash, and emit no totals containing that malformed commit. |
| E7 | Measured plus user-supplied records | Keep measured and user-supplied amounts in separate provenance totals. The PR report may display a combined total only as an explicitly labeled display subtotal when all records use the same currency and window; tests assert the separate rows and provenance labels, not causal ownership. |
| E8 | All unavailable records | No zero-cost claim is made; unavailable totals remain unavailable. |
| E9 | Cumulative account-level values | Output labels temporal attribution and explicitly denies causal proof. |
| E10 | PR description versus PR update templates | Both full-detail outputs contain the same accounting IDs and totals. The update template additionally labels its range and added/removed totals relative to the named prior PR state; no silent reformatting of issue output occurs. |

Required focused command: `python3 scripts/report-agent-usage.test.py`.

### F. PostToolUse whitespace eligibility regression

| ID | Scenario | Expected result |
|---|---|---|
| F1 | Outside-workspace file path such as `/memories/session/plan.md` | It is filtered before formatter invocation and produces no warning. |
| F2 | Deleted Markdown path | It is filtered or returned as an expected missing exclusion without a warning. |
| F3 | Unsupported source/binary/generated/protected path | Expected ineligible path is not sent to the formatter. |
| F4 | Eligible Markdown edit | Formatter is invoked once and normalizes according to existing behavior. |
| F5 | Multi-tool read-only batch with an unrelated path | No formatter invocation occurs. |
| F6 | Mixed eligible/ineligible batch | Only eligible existing workspace paths are formatted. |
| F7 | Dollar signs, backticks, and marker-like strings in eligible prose | Formatter preserves content other than approved whitespace normalization. |
| F8 | Ledger path during migration | Formatter preserves ledger bytes or excludes it according to the migration-protection rule. |
| F9 | Outside-workspace defense-in-depth test | Direct formatter boundary still returns `outside-workspace`; hook-level expected exclusions remain warning-free. |

Required focused command:

```sh
node --test scripts/format-whitespace.test.js .github/hooks/format-whitespace.test.js
```

### G. End-to-end temporary-repository migration

| ID | Scenario | Expected result |
|---|---|---|
| G1 | Temporary repository receives normal, amend, merge, and unavailable commits | Every inspected commit has one valid block. |
| G2 | Parser and human checker inspect generated commits | Human summary and canonical JSON agree exactly. |
| G3 | PR and issue commands inspect the generated range | Their results agree with the common accounting IDs and differ only by requested output scope. |
| G4 | Migration report runs before and after sample commits | Historical source bytes remain unchanged; new reports use Git commit messages. |
| G5 | Simulated ledger retirement | Retirement is gated until A–E pass; then assert removal of `docs/accounting/agent-usage-ledger.md`, `scripts/append-accounting-ledger.sh`, ledger-lock logic, and all ledger-specific consumers, with no replacement ledger file. |
| G6 | Failure-injection sweep | Malformed, duplicate, unsafe, unavailable, stale, and concurrent candidate cases leave the Git repository inspectable and remove task-owned temporary handoffs. |

Required aggregate command set:

```sh
node --test scripts/copilot-accounting-contract.test.js \
  scripts/copilot-accounting-collector.test.js \
  .github/hooks/copilot-accounting.test.js \
  scripts/format-whitespace.test.js \
  .github/hooks/format-whitespace.test.js
python3 scripts/migrate-agent-usage-ledger.test.py
python3 scripts/report-agent-usage.test.py
git diff --check
```

## Evidence and stage-exit rules

### Implement Tests stage

- Record each focused suite's actual baseline result.
- For A–E and G, record whether the baseline is expected-red for missing production behavior or green for behavior-preserving migration/report compatibility; absent modules, syntax errors, and fixture failures are invalid red evidence. C and F must discriminate the intended hook defects from setup failures.
- A behavior-addition test may be expected-red only for missing production behavior. Setup, syntax, fixture, or module-resolution mistakes are defects and do not count as valid red evidence.
- A behavior-addition suite must load a test-controlled stub or skeletal production module and execute through its assertions. It is valid expected-red only when the assertion observes the intended missing production behavior; missing modules, import failures, setup failures, syntax errors, and fixture mistakes are defects and do not count.
- The formatter regression suite is behavior-changing and should demonstrate the expected hook-invocation defect before the fix.
- Submit the cumulative test diff, outputs, and exact revision for aggregate Development QA.

### Implement Changes stage

- Focused suites for each semantic block must pass.
- No unrelated commits may be made green merely because their tests are unchanged.
- Hook tests must run in temporary repositories, and end-to-end reporting must inspect the exact generated commit messages.
- Migration and ledger bytes must be checked before and after implementation.
- Submit independent review evidence, focused validation, cumulative diff, and exact final commit range/hash for aggregate Development QA.

### Rollout and retirement gate

Ledger deletion remains blocked until all of the following are evidenced on the intended revision:

1. canonical parser and human-summary consistency tests;
2. normal, amend, merge, unavailable, duplicate, and hostile-input hook tests;
3. read-only migration checksum/report;
4. PR-range and linked-issue reporting agreement;
5. privacy checks and whitespace regressions;
6. explicit approval for the exact migration/retirement commit.

## Test priorities

1. Privacy and shell-safety failures are release-blocking.
2. Duplicate or ambiguous accounting blocks are release-blocking.
3. Silent conversion of unavailable values to zero is release-blocking.
4. Ledger-byte mutation before retirement approval is release-blocking.
5. Reporting mismatches are release-blocking for the migration commit.
6. Formatting warning polish is blocking for the hook regression scope but does not supersede privacy or parser failures.

## Non-goals

- Live billing endpoint mutation or OAuth scope expansion.
- Production push or deployment.
- Browser, Lighthouse, C/C++ interpreter, native, and WebAssembly build tests.
- Rewriting historical Git commits to attach accounting metadata.
- Adding Git notes.
- Creating a replacement accounting ledger file.

<!-- Co-authored by GitHub Copilot. Scope: commit-native agent usage accounting Test Plan. Model: MAI Code 2 Flash; reasoning-effort setting unavailable. Energy (kWh) and CO2: unavailable; no measured data or defensible estimation inputs. -->
