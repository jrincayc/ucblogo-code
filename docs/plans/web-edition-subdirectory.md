# Plan: Move Web Edition files under `web/`

> **Review needed:** Jeff to review this plan again before any implementation starts.

Scope: relocate files added by the Web Edition into a subdirectory. Files that exist on `master` (legacy) stay where they are, even where this branch modified them.

## Method

The file set comes from `git diff --name-status master...HEAD`: 118 added, 32 modified, 2 deleted.

- Added files are candidates to move.
- Modified files (for example `main.c`, `mem.c`, `parse.c`, `Makefile.am`, `configure.ac`, `README.md`) are legacy and stay in place.
- Deleted files (`win32trm.c`, `win32trm.h`) are not relevant.

## Proposed layout

| Destination | Contents (all currently at the repo root unless noted) |
| --- | --- |
| `web/src/` | `webmain.c`, `weblib.c`, `webterm.c`, `webgraphics.c`, `webgraphics.h`, `liblogo.js`, `pre.js` |
| `web/` | `shell.html`, `manifest.json`, `ucblogo-192.png`, `ucblogo-512.png`, `workbox-config.js`, `webserver.js`, `wasmer.toml`, `web.lgo`, `webstartup.lg`, `webedition.md` |
| `web/demos/` | `animationdemo.lg`, `animationdemo2.lg`, `htmldemo.lg`, `logoeyes.lg`, `stars`, `chromeflagsjspi.png` |
| `web/scripts/` | `scripts/build-help.js`, `scripts/build-help-docs.js`, `scripts/stage-web-deploy.js` |
| `web/tests/` | `tests/acceptance/**`, `tests/native/*` |

Stay at the root:
- Legacy files, including those modified on this branch.
- `package.json` and `package-lock.json`. npm and the Actions workflows expect them there. Revisit only if the move is done in a second phase.
- `.github/**`, which GitHub requires at the root.
- `docs/plans/**`, which is already a subdirectory.
- `contrib/**`, which is already a subdirectory.
- `tests/test.lg` and `tests/runjs-multiline.lg`. `ucblogo.html` embeds the whole `tests/` directory (`--embed-file tests`) and `make check` runs `tests/test.lg`. Moving them would change the virtual-filesystem paths that the Logo tests use. They sit beside the legacy `tests/UnitTests*.lg` files.
- `address-sanitizer.txt`, `gdb.rc`: not web-specific. Decide per file.
- `helpfiles/setdarkmode` belongs with the legacy help tree.

Delete, do not move:
- `ucblogo-code` is a committed symlink (mode 120000) to the absolute path `/home/jeff/ucblogo-code`. It is machine-specific and breaks on any other checkout. Remove it from the index and add it to `.gitignore`; recreate it locally if the `/ucblogo-code/` URL prefix is needed for serving.

## Build outputs

Generated outputs (`ucblogo.html`, `ucblogo.js`, `ucblogo.wasm`, `ucblogo-node.js`, `sw.js`, `workbox-*.js`, `web-config.js`) are produced at the root by the Makefile rules and are gitignored. They stay at the root in phase 1. The GitHub Pages URL layout and the service worker scope (`navigateFallback: '/ucblogo-code/ucblogo.html'`) depend on that.

Moving the outputs into `web/` or `dist/` is a separate decision. It changes the service worker scope and deploy URLs, and needs a live-site check.

## References to update

1. **`Makefile.am`**
   - Prefix the web sources in `ucblogo_SOURCES` and `EXTRA_DIST` (`webmain.c`, `weblib.c`, `webterm.c`, `webgraphics.c`, `webgraphics.h`) with `web/src/`. `main.c` stays legacy.
   - `--js-library liblogo.js` (lines 30, 200, 244), `--pre-js pre.js` (180, 217), `--shell-file shell.html` (230), and the dependency lists on `ucblogo-node.js` (176) and `ucblogo.html` (211) need the `web/` prefix.
   - `sw.js: ... workbox-config.js` and `npx workbox-cli generateSW workbox-config.js` (167-169) need the new config path.
   - `help/index.json` (line 207 area) depends on `scripts/build-help.js`, `scripts/build-help-docs.js`, and `webedition.md`; update all four and the `node scripts/build-help.js help` command.
   - `ucblogo.html` embeds `web.lgo` and `webstartup.lg` by name (lines 220-223). Remap them (`--embed-file web/web.lgo@/web.lgo`) so the virtual-filesystem paths do not change.
   - `--embed-file tests` (219) and `node ./ucblogo-node.js $(srcdir)/tests/test.lg` (173) stay unchanged because those files do not move.
   - `web-config.js` is generated at the root and listed in `CLEANFILES`; no change.
2. **`configure.ac`**: no change needed.
   - `AM_INIT_AUTOMAKE([subdir-objects foreign -Wall])` already has `subdir-objects`, so `web/src/*.c` objects build correctly.
   - `AC_CONFIG_SRCDIR([logodata.c])` is a legacy file.
   - `AC_CONFIG_FILES` lists only `Makefile` and the legacy subdirectory Makefiles. Do not add `web/Makefile`; the web files are built by the root `Makefile.am`.
   - No web file path appears in the script. `WEB_SERVICE_WORKER` and `WEB_LDFLAGS` are substituted values, not paths.
   - `tests/Makefile.am` lists only legacy `.lg` files, so moving `tests/acceptance` and `tests/native` does not affect it.
3. **`package.json`**: update the paths in the `test:*` and `stage:web` scripts (`tests/acceptance/...` becomes `web/tests/acceptance/...`, `scripts/stage-web-deploy.js` becomes `web/scripts/...`).
4. **`workbox-config.js`**: update `globDirectory` and `swDest` for the new config location. The glob patterns still resolve against the root, where outputs remain.
5. **Scripts and tests**: fix relative `require` and path strings in `scripts/*.js`, `tests/acceptance/*.js` (notably `source.test.js`, `stage-web-deploy.test.js`, `application-server.js`, `run-testcafe.js`), and `webserver.js` (server root).
6. **CI and deploy**: update `.github/workflows/ci.yaml`, `cd.yaml`, `deploy-web.yaml`, and `.github/hooks/guard-generated-files.*`.
7. **Customization files**: update path references in `.github/instructions/*.md` (including the `applyTo` pattern `tests/acceptance/**`), `.github/skills/**`, and `.github/prompts/**`.
8. **Docs**: `README.md`, `webedition.md`, `docs/plans/*.md`.
9. **`.gitignore`**: anchor patterns that currently match at the root (`reports/`, `screenshots/`, `/help/`) so they still match after moves.
10. **Other build files**: `makefile.msys`, `winbuild.sh`, `Info.plist`, and the `ucblogo.dsp` project should be unaffected because only web files move. Verify with grep.

## Execution order

Each step is one local commit.

1. Move files with `git mv` (history preserved), grouped by destination, with no content edits.
2. Fix the Autotools wiring (`Makefile.am`, `configure.ac`), then run a full WASM build.
3. Fix `package.json`, scripts, and test paths, then run `npm run test:source`.
4. Fix CI, deploy, and customization-file paths.
5. Update docs and `.gitignore`.
6. Run `npm run build:web`, `npm run test:built`, and a focused desktop Chrome smoke test (`npm run test:browser:smoke`).
7. Run `npm run stage:web`, then confirm the staged tree matches a pre-move staged tree (diff the two directories).

## Verification and risks

- Before building, run `make clean` while the current Makefile is still active, and use a fresh build tree to avoid stale objects.
- Native and wx builds must be unchanged. Build natively (`./configure && make`) and confirm that no web files are compiled in.
- The service worker and cache scope must be unchanged. Verify in a fresh Chrome profile with caching disabled.
- A pre-move staged deploy tree gives a baseline: the post-move tree should be byte-identical.
- Risk: `git mv` plus path edits across 100+ files makes rebases onto upstream harder. Do the move in one PR and avoid parallel web-edition work.
- Risk: root-level `package.json` with scripts under `web/` is a split layout. If that is unacceptable, move `package.json` and the Actions working directory together in a second phase.
