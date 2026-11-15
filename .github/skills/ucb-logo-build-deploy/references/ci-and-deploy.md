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

## Related Plans

- [PWA installability and offline hardening](../../../../docs/plans/pwa-installability.md)
- [Browser and Node CI](../../../../docs/plans/browser-node-ci.md)
- [Emscripten upgrade](../../../../docs/plans/emscripten-upgrade.md)
- [TestCafe consolidation](../../../../docs/plans/testcafe-consolidation.md)
- [Native build restoration](../../../../docs/plans/native-build-restoration.md)
