---
description: "Use when a task touches generated Web Edition build outputs such as ucblogo.html, ucblogo.js, ucblogo.wasm, sw.js, or the Workbox runtime."
applyTo: "ucblogo.html,ucblogo.js,ucblogo.wasm,sw.js,workbox-????????.js"
---
# Generated Web Edition Artifacts

These files are untracked build outputs. Do not hand-edit them to fix behavior.

- Edit the sources instead: `shell.html`, `pre.js`, `liblogo.js`, `workbox-config.js`, `Makefile.am`, or the C sources.
- Rebuild with `emmake make` (or `npm run build:web` for a clean build) and verify the new content is what the browser loaded.
- If behavior contradicts the source, suspect a stale build, HTTP cache, or service worker before editing again. See the [stale-artifacts checklist](../skills/ucb-logo-web-testing/references/stale-artifacts.md).
