# Upstream Integration Roadmap

The eventual integration target is **`jrincayc/ucblogo-code`, base branch `master`**, not the maintainer's fork's `master`. Verify remote URLs and refreshed branch refs when integration is actually requested; local remote names are not universal.

The maintainer reports that wx is currently broken on the development branch. Preserve wx compatibility as a goal, but do not assume a passing native baseline. Upcoming work is:

1. Rebase the development changes onto upstream `master`, protecting local work and resolving conflicts without dropping intentional Web Edition changes.
2. Reproduce and repair the branch's Linux wx build with native, isolated build state. Verify compile/link, startup/input, relevant Logo regressions, and affected GUI behavior while retaining web/Chrome correctness; pursue macOS/Windows checks through available CI.
3. Clean up integration-related code, obsolete configuration, tests, docs, and artifact handling. Review the diff against upstream `master`, run the relevant broader platform checks, and prepare a PR to the upstream repository with explicit validation results and coverage gaps.

This roadmap is context, not standing permission to rebase, push, or create a PR during skill editing or unrelated development; local checkpoint commits follow the repo commit policy. Act on the task explicitly requested; implementation may be used to verify skill guidance when agreed, without automatically expanding into the whole roadmap. Before rewriting published history, agree on branch strategy and protection of uncommitted work. Never silently stash, discard changes, or force-push. Update known branch status and completed roadmap items as the work progresses.
