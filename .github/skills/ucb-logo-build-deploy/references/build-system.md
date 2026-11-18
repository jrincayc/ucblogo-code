# Build-System Ownership

Improving the build system is an explicit responsibility of this skill. Make builds reproducible, easier to debug, fast to iterate, and maintainable while preserving web and wx builds.

## Web Build Profiles

Profiles are configure options (`emconfigure ./configure --enable-wasm ...`). See `webedition.md` for the option table.

| Profile | Configuration |
| --- | --- |
| `prod` | Default: `-O3`; service worker generated and registered. |
| `dev` | `--with-debug-info=dwarf\|separate-dwarf\|source-map`; `-Og` unless `CFLAGS` is set; no service worker. |
| `debug` | dev plus `--enable-asan` (and/or `--enable-ubsan`), `--enable-assertions`, `--enable-stack-overflow-check`; no service worker. |

- Configure rejects incompatible selections (e.g. ASAN with `SAFE_HEAP`; service worker in any non-production build; Emscripten-only options without `--enable-wasm`; wx/X11 with wasm). Add new exclusions there rather than in scripts. Do not silently disable ASAN to make a debug build pass. Do not change `EXIT_RUNTIME` for ASAN builds.
- Non-production builds write `Module.serviceWorker = false` in `web-config.js`; `shell.html` then unregisters any earlier worker. Skipping generation alone would not disable an already-registered worker.
- Regenerate Autotools only when `configure.ac`/`Makefile.am` change; reconfigure to switch profiles, then `emmake make clean` to avoid cross-profile objects.

## Build Engineering Rules

- Read `configure.ac`, `Makefile.am`, npm scripts, and relevant CI before changing recipes. Treat generated configuration, Makefiles, loaders/Wasm, and service-worker outputs according to actual repo policy; fix source recipes rather than generated symptoms.
- Prefer explicit prerequisites, accurate dependency tracking, clear options, shared source/object/export definitions, and target-specific settings over duplicated link lists and implicit shell state. Add abstractions only when they remove real complexity.
- Isolate native/wx and Emscripten configurations/objects. Check the active configuration before `make`; do not reuse Wasm objects in native links or overwrite working builds casually. Verify out-of-tree support before relying on it, particularly for root-relative custom web/embed recipes.
- Separate configuration, compilation, linking, asset staging, tests, and packaging where it improves iteration. Preserve required library/help/CSLS files, startup overrides, exports/debug assets, and browser/Node differences.
- Validate clean/incremental builds, dependency-triggered rebuilding, failure exit status, missing-tool diagnostics, and feasible parallel execution. Measure performance claims.
- Remove obsolete flags/comments in the relevant recipe without silently changing Asyncify. Treat optimization, sanitizers, debug symbols, memory, and exception settings as deliberate validated configurations.
- Check Workbox/PWA cache generation, registration, manifest/assets, deployment paths, and artifact consistency. Never mix old loaders and new Wasm in a cache.
- For shared interpreter/build changes, compile/link wx when available and smoke-test startup, input, basic evaluation, and affected behavior. Node/headless success is not wx GUI validation. Report missing wx dependencies/display honestly.

## Build State

- `configure.ac` owns Autoconf platform/options checks; `Makefile.am` owns source selection and custom Node/browser links. Generated outputs are not primary edit targets.
- `emconfigure ./configure --enable-wasm` selects the Web Edition (default target remains wx): wx/X11/docs off, gitid on, prefix `./dist`, `-O3`, Asyncify link settings in `WEB_LDFLAGS`. `npm run build:web` runs autoreconf, configure, clean, `make`, `make check`. It changes the root configuration and is not a narrow harmless test command.
- Browser links run `make install-data` first and embed `$(pkgdatadir)` as `/share/ucblogo`, plus tests, `web.lgo`, an icon, and `webstartup.lg` as `/share/ucblogo/logolib/startup.lg`. Library-file edits alone do not trigger a relink; delete `ucblogo.html`. `makehelp` uses `CC_FOR_BUILD`; a fresh tree needs `make regenerate-help-index` once.
- Node uses shared Wasm objects but different environment/filesystem flags. Check both links for shared exports/dependencies.
- `ucblogo.html`, loaders, Wasm/debug files, generated workers/Workbox assets are build outputs. Check artifact/configuration/cache consistency when observed behavior disagrees with source.
- Native build instructions: `README.md`; wx checks: `configure.ac`. Inspect `wx-config`, Autotools macros, docs tools, compiler and active configuration. Verify custom recipe out-of-tree support before relying on it.
- Read `.github/workflows/`, `winbuild.sh`, and platform packaging files when their pipeline/platform is affected.
- Native/wx build: `autoreconf --install`, `./configure --enable-wx`, `make`. Preserve isolated build state; optionally `--disable-docs` when intentionally skipping unavailable docs tools. Confirm wx was selected.
