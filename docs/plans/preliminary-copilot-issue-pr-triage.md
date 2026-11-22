# Plan: GitHub Issue and PR Preliminary Copilot Checks

## Decisions

- Automation surface: GitHub Actions or GitHub Models.
- Pull requests trigger on opened and synchronized events.
- Output is a public advisory comment.
- Unknown or forked arrivals are analyzed only through supplied text and diff metadata; PR code is not checked out or executed.
- Model use is low effort and restricted to MAI Code 2 Flash or GPT-6 Luna.
- Draft PR behavior is implementation-decision detail, not a planning blocker.

## Goals

Add two bounded read-only GitHub event workflows:

1. New issue preliminary triage.
2. Opened or synchronized PR preliminary check.

The workflows must stop after an advisory report. They must not implement fixes, edit repository state, approve or merge pull requests, or start the full development orchestration workflow. Humans decide whether further Copilot assistance is necessary.

## Issue Triage

Trigger only for newly opened issues and ignore bot-authored or synthetic repeats.

The issue report should be bounded to approximately 10-15 lines and cover:

- bug, question, enhancement, documentation, security, or unclear classification;
- affected subsystem when evidence exists;
- preliminary severity or urgency;
- reproducibility and missing environment or detail;
- likely relevant files or components only when source-grounded;
- whether to keep the issue as-is, request clarification, or escalate to deeper assistance; and
- whether a quick Logo Language CTO triage is requested.

The report must distinguish observed facts, uncertainty, and unsupported guesses. It must not claim that a security vulnerability exists.

## PR Precheck

Trigger for opened and synchronized pull requests. Use a concurrency group keyed by PR number and cancel superseded synchronization runs.

Supply only the PR title, description, changed-file summary, bounded diff hunks, base/head metadata, and existing CI context if already available. Do not checkout or execute the PR tree.

The report should be bounded to approximately 10-20 lines and cover:

- apparent scope;
- obvious correctness, security, compatibility, and test concerns;
- missing or weak test and documentation coverage;
- files of concern with line or range references;
- likely CI or build implications; and
- whether a human-maintained QA or deeper code-review pass seems worthwhile.

Post or update one sticky comment with a stable marker instead of creating a comment for every synchronization.

## Core Logo Language Escalation

If preliminary QA finds that an issue or PR proposes a core UCB Logo language change, the report must request a quick Logo Language CTO triage.

This applies to proposed changes involving:

- syntax or parsing;
- primitive names, defaults, or arity;
- evaluation semantics;
- core library behavior;
- idioms or dialect compatibility; or
- language philosophy.

The CTO triage is an advisory human-decision handoff. It does not automatically invoke the full development workflow, implementation, or a second model pass.

## Guardrails

- Use least-privilege permissions: read repository contents and write only issue or PR comments.
- Do not use `pull_request_target`, repository write secrets, repository dispatches, or merge/approval permissions.
- Use a bounded timeout and one bounded retry for transient model failure.
- Limit input context and output tokens.
- Ignore bot-authored arrivals and preview-comment self-loops.
- Use GitHub Actions or GitHub Models with a repository-owned prompt and no third-party model action unless separately approved.
- If provider, model, credentials, or low-effort support are unavailable, record a visible capability-check failure and make no model call.
- Never silently substitute any model outside MAI Code 2 Flash or GPT-6 Luna.

## Validation

Test the following scenarios:

- issue opened;
- PR opened;
- PR synchronized;
- bot-authored arrival;
- forked or unknown author;
- draft PR;
- oversized diff;
- core-language syntax, primitive, idiom, dialect, or philosophy proposal;
- unavailable provider;
- unavailable model;
- unavailable low-effort setting;
- permission failure;
- transient model failure;
- sticky-comment update and deduplication.

Verify YAML syntax and event/job permissions, bounded timeout and concurrency, comment markers, model allowlisting, low-effort configuration, read-only permissions, input/output limits, and absence of PR checkout or code execution.

Dry-run comments against sanitized representative issue and PR fixtures before live enablement.

## Scope Boundaries

Included:

- issue triage workflow;
- PR precheck workflow;
- bounded prompts;
- event guards;
- cost and permission controls;
- runtime capability checks;
- fixture-based validation.

Excluded:

- changing issue labels or assignees;
- approving or merging PRs;
- source edits or automated fixes;
- executing or building PR code;
- CI interpretation beyond supplied metadata;
- automatic invocation of the full development orchestration workflow; and
- deployment or release actions.

## Implementation Files

- `.github/workflows/ci.yaml` — existing PR build context to preserve.
- `.github/workflows/development-orchestration.md` — deeper workflow contract that preliminary checks must not replace or start automatically.
- `.github/agents/development-qa.agent.md` — existing deeper QA role.
- `.github/agents/development-reviewer.agent.md` — existing deeper code-review role.
- `.github/agents/logo-language-cto-reviewer.agent.md` — existing quick/official core-language reviewer role when assigned.
- `.github/prompts/` — candidate location for prompt assets.
- `.github/workflows/preliminary-issue-triage.yaml` — issue-opened workflow.
- `.github/workflows/preliminary-pr-check.yaml` — PR-opened/synchronized workflow.

<!-- Co-authored by GitHub Copilot (MAI Code 2 Flash, reasoning effort unavailable). -->
