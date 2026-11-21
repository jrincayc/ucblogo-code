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
