# Plan: TPS-Inspired Software Workflow Improvements

Status: Deferred. Do not implement this plan until the current round of orchestration and workflow implementation is finished. This document records future ideas only; it does not change current acceptance criteria or authorize implementation.
Date: 2026-11-18

## Purpose

Explore how Toyota Production System ideas can improve this project's software-development workflow. Apply the underlying principles to the flow of code, information, artifacts, tests, and decisions rather than copying manufacturing practices literally. Preserve the existing quality gates and user approval requirements unless a future, explicitly approved plan changes them.

## Ideas to Evaluate

1. **Map the actual value stream.** After the current implementation round, observe a representative change from its initial request through implementation, validation, review, and acceptance. For Web Edition work, include the source change, Emscripten build outputs, served page and cache, browser checks, and test results. Record what moves between steps, queue and wait time, repeated work, and artifact/revision identity.

2. **Treat information and artifact I/O as movement.** Include filesystem, memory, network, build-system, generated-artifact, test-data/report, and human or agent context transfers. Treat unreviewed changes, tasks waiting at gates, unconsumed build outputs, and results awaiting interpretation as work-in-process inventory. Do not count useful I/O as waste; investigate transfers that are duplicated, premature, misrouted, slow, or leave stale state behind.

3. **Distinguish learning iterations from churn.** A loop is useful when it delivers value or reduces uncertainty. A failed experiment can be progress when it rules out a cause, tests an assumption, or produces a regression test. Capture the hypothesis, discriminating check, observed result, and how the next action changes. Repeating the same hypothesis and check without new evidence is waste. Treat the current three-attempt escalation limit as a ceiling, not a retry quota; turn recurring rework into a test, check, or clearer standard procedure.

4. **Pull work to available capacity and level the load.** Use observed queues to decide whether to limit new work awaiting QA, review, or human approval. Keep concurrency below the configured maximum when reviewer, maintainer, workstation, or network capacity is the bottleneck. Avoid large batches and overlapping I/O-heavy builds when smaller or staggered work would improve flow.

5. **Strengthen quality at the source.** Fail fast when checks reveal a wrong revision, stale artifact, or validation defect, and keep evidence tied to the exact commit and runtime artifacts. Consider automating deterministic, repeatable checks while retaining human judgment for scope, risk, design, and approval. Evaluate Andon pause scope using evidence; any change to the current stop-the-line policy requires explicit user approval.

6. **Make improvement collaborative.** Review observed friction with the people doing and checking the work. Choose one demonstrated source of delay or rework, try a bounded change, and compare the result. Do not use iteration count or individual utilization as a proxy for productivity.

## Candidate First Experiment

Once the current orchestration and workflow implementation round is complete, use one representative, human-selected task to map the value stream. Baseline end-to-end time, time waiting at each gate, work-in-process by gate, repeated I/O or handoffs, rebuild/retest/review loops, and stale-artifact incidents. Use the evidence to select one improvement; do not add a tracking system or change workflow defaults before a specific bottleneck is demonstrated.

## Scope and Deferral

- No implementation, workflow-policy edits, or changes to current task acceptance are included in this plan's present status.
- Begin follow-up only after the current orchestration and workflow implementation round is finished.
- Any later implementation must have its own agreed scope, acceptance criteria, and validation, following the workflow then in force.

<!-- Co-authored by GitHub Copilot. Scope: deferred TPS-inspired software workflow plan. Model identity and selected reasoning-effort setting are unavailable. Partial measured interval through initial validation: 2m10s (2026-11-18 12:45:50-12:48:00 EST); final metadata update and verification excluded. Energy (kWh) and CO2: unavailable; no measured data or defensible estimation inputs. -->
