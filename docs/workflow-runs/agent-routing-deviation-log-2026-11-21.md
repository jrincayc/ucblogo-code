# Agent Direct-Assignment Routing Deviation Log

Status: **Open; correction deferred**
Date: 2026-11-21
Current task: `TURTLE-SKILL-RESUME-2026-11-21`
Turtle gate: **Plan reopened/pending**
Branch: `wasm`

## DEV-ROUTE-MODEL-LABEL-2026-11-21-01

The user reports that this direct-assignment model-label problem has occurred several times. In the latest observed occurrence, an attempted direct assignment was rejected because the supplied model label used the wrong vendor suffix. The repository-equivalent model was already available, and the intended assignment was retried with the exact available label without changing task scope.

This is a routing/preflight deviation, not a confirmed implementation defect. The repeated mismatch is recorded for later correction. The task and any dependent turtle work stop before any routing-policy correction or implementation until this record is saved and the turtle workflow is explicitly resumed.

### Affected Scope

- Direct-assignment model selection and model-label validation.
- Subagent preflight and host/model availability reporting.
- `.github/workflows/development-orchestration.md` and role prompt/model instructions, when a correction is later authorized.
- Future routing/assignment tests that must discriminate exact model identity from vendor-suffix formatting or repository aliases.

### Preservation

- No model fallback was used in the later retry.
- No routing-policy correction, Turtle Graphics implementation, Test Plan, build, browser, commit, or push action was authorized or performed as part of recording this deviation.
- The Turtle Graphics Plan remains reopened/pending with SHA-256 `c90b52dc4c7c58f01dddb7b9e32681e14bc1767df0479664a467ba688410f627`; this routing deviation does not replace the escaped screenshot-defect reopen record.
- Pre-existing worktree changes remain preserved.

### Deferred Correction

At a separately authorized correction stage, investigate and add one deterministic exact-model/vendor mapping and a preflight check for model availability before attempting assignment. Add a compact routing regression test that reproduces this vendor-suffix mismatch and verifies that no fallback or scope substitution occurs silently. Do not change shared orchestration policy or role prompts as part of recording this entry.

<!-- Co-authored by GitHub Copilot. Scope: repeated model-label direct-assignment routing deviation. Model: MAI Code 2 Flash; reasoning-effort setting unavailable. -->

## DEV-SHELL-LEDGER-EXPANSION-2026-11-21-01

An attempted parallel-pool significant-event command used an unquoted heredoc for an accounting snapshot. Shell expansion consumed missing or currency-prefixed variables, so the appended snapshot contained blank checkpoint fields and corrupted monetary text such as `1.30 / 2.00` and `/usr/bin/bash.70 remaining`. The command itself was the process deviation; the malformed output was not authoritative usage evidence.

The user explicitly called an Andon on the command, directed that the deviation be recorded, and authorized continuation. No implementation, browser, build, push, or stage advance occurred in the affected command. The turtle task remains at the reopened Plan gate.

### Required Recording

- Preserve every prior malformed append as immutable history; do not rewrite or reorder it.
- Append a quoted, non-expanding correction with the authoritative snapshot: `20,000 / 20,000` included credits used, `0` remaining, reset `2026-11-30`, and additional usage `$11.30 / $12.00`, `$0.70` remaining.
- Reiterate that monetary corruption invalidates only the malformed fields in the affected snapshots, not other unrelated accounting records.
- For later accounting commands, use single-quoted heredocs, literal dollar amounts, and a post-append grep/check for blank required fields and accidental shell expansion.

Correction authority: this is process/accounting correction only. The turtle Plan review verdict and revision identity are handled separately at the Plan gate.

<!-- Co-authored by GitHub Copilot. Scope: malformed shell expansion during accounting append. Model: MAI Code 2 Flash; reasoning-effort setting unavailable. -->

## Rapid Wrap Authorization — 2026-11-21

The user explicitly approved a deviation to wrap the turtle skill effort as quickly as possible because it is over budget. All remaining defects and corrections are deferred. No Plan repair, QA, Test Plan work, implementation, browser run, build, commit, or push is to occur in this session.

Current stopping gate: **Plan reopened/pending**. The exact candidate Plan remains at SHA-256 `c90b52dc4c7c58f01dddb7b9e32681e14bc1767df0479664a467ba688410f627`. Any future resume must begin by reconstructing current evidence, then obtain the required independent Plan QA and explicit human approval.

Deferred items include the repeated model/vendor-label routing deviation, malformed shell-accounting expansion correction verification, stale Plan revision/provenance issues, disabled screenshot/report contradictions, path/UUID negative coverage, parallel-map ownership gaps, and any future defect correction not explicitly resolved in this record.

<!-- Co-authored by GitHub Copilot. Scope: user-approved rapid task wrap and deferred defects. Model: MAI Code 2 Flash; reasoning-effort setting unavailable. -->
