# Research Findings Handoff

Research reports in this directory provide evidence for planning. They are not
implementation plans. Create one report per research pass at
`<topic-slug>-<UTC-date>.md`; preserve existing reports and add a numeric suffix
if the name is already taken.

Use this structure:

```markdown
# Research Findings: <topic>

**Status:** Complete or Partial
**Researched:** YYYY-MM-DD (UTC)
**Question:** The decision or uncertainty being researched
**Scope and assumptions:** Boundaries, constraints, and assumptions

## Key Findings

## Evidence Ledger

| ID | Finding | Type | Confidence and basis | Source and date | Planning relevance |
| --- | --- | --- | --- | --- | --- |

## Options and Trade-offs

## Conflicts and Counterevidence

## Constraints and Dependencies

## Unknowns and Unverified Items

## Search Coverage and Limitations

## Planning Handoff

## Sources
```

For evidence types, use `documented fact`, `direct observation`, `inference`,
`assumption`, or `proposal`. Confidence is `High`, `Medium`, or `Low`, with a
brief rationale. Cite claims with evidence IDs and link to source URLs or
workspace paths; identify source type and dates where available. For internal
code, cite the owning file and relevant symbol or line. Distinguish sources that
were inaccessible from topics that were not searched. Keep options neutral;
leave sequencing, implementation choices, and validation planning to the
planning agent.

The planning handoff should surface the facts, constraints, dependencies,
trade-offs, risks, and unresolved decisions that may shape a plan. State what is
not known rather than filling gaps with assumptions.

<!-- Co-authored by GitHub Copilot. Scope: research findings handoff format. -->