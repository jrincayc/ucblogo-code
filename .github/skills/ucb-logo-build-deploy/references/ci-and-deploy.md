# CI And Deploy

Verify against the current workflow files before relying on these details.

## Workflows

| Workflow | Trigger | Purpose |
| --- | --- | --- |
| [ci.yaml](../../../workflows/ci.yaml) | push and pull request on `master` and `wasm` | `wasm_build_and_test` runs only for the `wasm` branch (push, PR base, or PR head); the remaining jobs build and test native targets. |
| [cd.yaml](../../../workflows/cd.yaml) | push to `cd_*`, `master`, `release*` | Builds and caches wxWidgets, then builds Logo for Linux, macOS (per-architecture and universal), and Windows. |
| [deploy-web.yaml](../../../workflows/deploy-web.yaml) | push of a tag matching `p[0-9]*`, or manual dispatch | Builds, tests, and stages the Web Edition; publishes to the `gh_pages` branch only for a tag. |

## Web Deploy Flow

- Version is `git describe --tags --always --dirty`. For a tag run it must equal the tag name or the job fails.
- The job pins Emscripten (`mymindstorm/setup-emsdk`, check the workflow for the current version), uses the latest Node, runs `npm ci`, installs `autoconf automake autoconf-archive libtool`, then runs `npm test` (build plus source checks plus all functional browser tests, no Lighthouse).
- It checks out `gh_pages` into `site`, then runs `npm run stage:web -- site --expect-version "$VERSION"`.
- A manual run uploads the staged site as an artifact and does not publish. A tag run commits the staged site with the version as the message and pushes `HEAD:gh_pages`. Concurrent runs queue (`concurrency: deploy-web`, no cancel).

## Staging Script

`node scripts/stage-web-deploy.js <target-dir> [--source <dir>] [--expect-version <tag>]` copies exactly the files the Web Edition needs and verifies them; it also exports `stage` and `verify`.

- Required: `ucblogo.html`, `ucblogo.js`, `ucblogo.wasm`, `sw.js`, `manifest.json`. Optional: the `.map` files. The Workbox runtime `workbox-xxxxxxxx.js` named by `sw.js` is included.
- `verify` checks that `sw.js` precache entries and revisions match the files, that `<link>` hrefs in `ucblogo.html` and manifest icons are flat bare file names present in the directory, and, with `--expect-version`, that the version is embedded.
- The published tree is flat; nested or absolute references fail.

## Releasing

1. Confirm the branch is clean and CI or a local `npm test` passed.
2. Pick the next tag in the existing `pN` series (`git tag --list 'p*' --sort=-v:refname | head`).
3. Pushing the tag publishes the live site. Get explicit confirmation, then `git push origin <tag>`.
4. Watch the run (`gh run list --workflow deploy-web.yaml`, `gh run watch`).
5. Verify the live bytes, not the browser:
   - Download the published files with a cache-busting query string into a temporary directory: the five required files, the Workbox runtime named in the live `sw.js`, and every file referenced by `<link>` hrefs in `ucblogo.html` and manifest icons.
   - Confirm superseded `workbox-xxxxxxxx.js` runtimes now return 404.
   - Run `node -e "require('./scripts/stage-web-deploy.js').verify('<dir>', {expectVersion: '<tag>'})"`.
   - Remove the temporary directory.
6. A browser that already loaded the site may keep serving the old build from its service worker; use a fresh profile or the [stale-artifacts checklist](../../ucb-logo-web-testing/references/stale-artifacts.md) before concluding the deploy failed.

## Diagnosing A CI Failure

`origin` is the fork (`n1ywb-1/ucblogo-code`) and `upstream` is `jrincayc/ucblogo-code`, so plain `gh run list` fails with "No default remote repository"; always pass `-R`. Push runs live on the fork, `pull_request` runs on upstream; both build the same SHA.

Shortcut: `scripts/ci-run.sh {list|watch|failed|errors|log} [branch] [repo]` wraps the steps below for the latest run on the current branch (default repo is `origin`). Output is terse to save tokens: `watch` is silent on success and prints only failing "job: step" lines plus the first `N` (default 15) distinct errors on failure; `log` saves the full failed log to a temp file and prints its path. After a push, use `watch` and report the result; to push rewritten history, `git push --force-with-lease origin <branch>` only with explicit permission.

CI helper scripts cache completed runs (immutable) in `/tmp`. `ci-run.sh` and `ci-test-stats.py` share `/tmp/ci-cache` (`<repo>-<run>-jobs` JSON read by both; `-log-failed` for `ci-run.sh`; the full gzipped `-log.gz` for `ci-test-stats.py`, which parses every job's test lines and step results so parser changes never refetch). `ci-screenshots.sh` caches the "WebAssembly Test Reports" artifact in `/tmp/ci-screenshots`. Whenever you add or change one of these scripts, check whether it can reuse another's cache or share a layout instead of refetching from `gh`; the maintainer is on a slow connection, so downloads are the cost, not disk. Measured on one run: the full log is about 84 KB gzipped and the failed-only log 22 KB, so `ci-run.sh` could later derive its failed-step output from `-log.gz`. The native and wx jobs print only summaries in their logs; their per-test results are in separate artifacts ("Logo Unit Test Results ...", "Native Terminal PTY Results"), not yet parsed.

1. `date; gh run list --branch wasm --limit 5 -R n1ywb-1/ucblogo-code --json databaseId,conclusion,event,headSha,displayTitle | cat` (use `-R jrincayc/ucblogo-code` for PR runs). Compare `headSha` with `git log -1`.
2. `gh run view <id> -R <repo>` lists jobs and failed steps; `gh run view <id> -R <repo> --log-failed` prints their logs.
3. Logs are huge and wrapped at terminal width: `--log-failed | grep -n "error\|Error"` first, then read around the hits. The full `--log | grep -m3 "Image: "` shows the runner image (for example `ubuntu-26.04`), which matters because `ubuntu-latest` moves.
4. Compare against `upstream/master` (`scripts/upstream-diff.sh {fetch|files|legacy|log|find|diff}`; silent when nothing differs, `legacy` lists only upstream files the branch modified, deleted, or renamed, `find STR` finds the commit that added or removed STR) to tell a branch regression from an environment change. If the file is unchanged, suspect the runner image or toolchain.
5. Reproduce locally only after noting that the local glibc/toolchain may differ from the runner's.

`errors` matches compiler `error:`, `##[error]`, and TestCafe `✖`/`failed (` lines; `failed` prints the job table, whose failing step name says which suite broke. The log is prefixed with job, step, and timestamp, so `cut -c70-260` makes the body readable.

Intermittent: `HELP-FONT-24` in `tests/acceptance/help-font-size.testcafe.js` failed twice on one SHA (selector `#helpCatList h3 button` with text "Constructors" not found, 89/90 passing) after passing on the previous SHA, so a rerun does not prove a flake; the other help-font sizes passed.

Known failure: native terminal builds on `ubuntu-26.04` (glibc without `struct termio`) fail in `term.c` with "`tty_cooked` has an incomplete type `struct termio`" (uses `TCGETA`/`TCSETA`). Not caused by the branch; the fix is to use `struct termios` with `tcgetattr`/`tcsetattr`.

## Related Plans

- [PWA installability and offline hardening](../../../../docs/plans/pwa-installability.md)
- [Browser and Node CI](../../../../docs/plans/browser-node-ci.md)
- [Emscripten upgrade](../../../../docs/plans/emscripten-upgrade.md)
- [TestCafe consolidation](../../../../docs/plans/testcafe-consolidation.md)
- [Native build restoration](../../../../docs/plans/native-build-restoration.md)
