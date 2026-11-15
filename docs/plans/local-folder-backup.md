# Project Plan: Chrome Local Folder Backup And Sync (File System Access API)

Status: Proposed; implementation has not started.
Date: 2026-11-15

## Objective

Let users on Chromium browsers choose a folder on their computer that mirrors
the Web Edition virtual filesystem (`/d`, IDBFS). It needs no account and no
network, works offline, and protects against browser storage eviction. It also
gives desktop users other destinations without any OAuth: the chosen folder can
sit inside a Dropbox, OneDrive, Google Drive, or iCloud Drive desktop-client
folder, or be a git working tree, and that client or tool does the rest.

This is the second provider, after Dropbox ([dropbox-backup.md](dropbox-backup.md)).
It reuses that plan's sync core, manifest, triggers, and Backup dialog. Wider
provider options are in
[backup-provider-investigations.md](backup-provider-investigations.md).

## Support matrix

Verified (MDN and Chrome documentation, 2026-11-15):

- `showDirectoryPicker`: Chrome and Edge desktop 86+, Opera 72+, Chrome Android
  132+, Samsung Internet 29+. Not in Firefox, Safari, or iOS. Brave: behind a
  flag. The Chrome documentation lists Windows, macOS, ChromeOS, Linux, and
  Android.
- Directory handles are serializable to IndexedDB and can be `postMessage`d
  within the same top-level origin.
- `requestPermission()` needs transient user activation (a worker cannot
  consume it), so permission UI must run on the main thread. `queryPermission`
  and `requestPermission` exist on Chrome desktop and Android.
- Chrome may refuse core system folders (the Windows and macOS Library folders
  are given as examples) and asks the user to pick another folder; the exact
  blocklist is unverified.
- `createWritable()` changes are not committed until the stream is closed.
- A `File` returned by `getFile()` becomes unreadable if the file changes on
  disk; call `getFile()` again.
- `FileSystemHandle.move()` is not supported for directories and is behind a
  flag outside OPFS, so renames are copy plus delete.
- `FileSystemObserver` (change notifications): Chrome and Edge 133+ desktop only,
  experimental and non-standard; not on Android. It can only be an optional
  enhancement.

Unverified (settle in Phase 0): whether Chrome keeps directory permission across
browser restarts for a stored handle (the documentation says permissions are not
always persisted between sessions and links to a persistent-permissions
article that could not be fetched), behavior for installed PWA windows, the
exact restricted-folder list, swap-file names written during `createWritable`,
and Android folder restrictions.

## Constraints

- No backend, no network. Nothing leaves the user's machine through this
  feature, so it adds no CORS, CSP, service-worker, or deploy-staging changes.
- The Asyncify Logo runtime is non-reentrant: sync code touches `FS` from JS
  only and never calls C exports.
- Do not auto-activate anything that reloads a live session; do not navigate
  away from it.
- Generated outputs (`ucblogo.html`, `sw.js`, `web-config.js`) are fixed in
  their sources.
- This plan never pushes or releases. iPad and desktop Safari cannot use this
  feature; report that as an unsupported path, not as coverage.

## Threat model: a persisted directory handle is a capability

`Module.runjs` uses a direct `eval` in [pre.js](../../pre.js), and IndexedDB is
origin-wide, so a Logo program loaded by the user can obtain a stored,
readwrite directory handle and use it. Unlike a Dropbox token, the handle
cannot be hidden in a Worker because it must be persisted in IndexedDB to
survive reloads. The exposure is bounded to the chosen folder, but within it a
hostile program could read everything or plant files. Mitigations:

1. Guide users to choose a dedicated, new folder; warn not to choose a folder
   with other data. Tell them the folder is readable by programs they run.
2. On connect: an empty folder starts fresh; a folder with the sync marker
   resumes; a non-empty folder without the marker requires an explicit "merge"
   confirmation that lists what is there.
3. Never write dot-files from Logo, and on push skip executable and script
   extensions (initial list: `exe bat cmd com scr msi ps1 vbs vbe wsf jar lnk
   app command desktop appimage`), reporting each skipped file. This is a policy
   to confirm; Chrome may already block some writes (unverified).
4. Never recursively delete directories; remove only files, and empty
   directories that the sync created.
5. Allow explicit "Disconnect" that deletes the stored handle and manifest.

Keeping the handle in memory only would remove the exposure but force a folder
pick every session; this plan chooses persistence with the mitigations above.

## Provider design

The folder provider implements the shared provider interface from the Dropbox
plan. Differences from a cloud provider drive the design:

| Concern | Folder behavior | Handling |
| --- | --- | --- |
| Versioning | No revision or ETag; `lastModified` resolution varies by file system | `{size, lastModified}` is a hint; SHA-256 of the bytes is the truth |
| Compare-and-swap | None | Re-stat immediately before writing and treat a mismatch as a conflict (conflict copy) |
| Atomic writes | Commit on `close()` | Always `close()`; do not use `keepExistingData`; ignore swap files |
| Rename | Unsupported for directories, flagged elsewhere | Copy plus delete |
| Case sensitivity | Depends on the volume | Treat names as case-insensitive; report collisions |
| Unicode | NFC versus NFD differences across platforms | Compare names normalized to NFC |
| Windows names | Reserved names (CON, NUL, COM1...), `<>:"\|?*`, trailing dot or space, long paths | Apply the Windows rules on every OS so the folder is portable; skip and report |
| External edits | No notification on Android, Firefox, or older Chrome | Scan on `focus` and `visibilitychange`; optionally coalesce `FileSystemObserver` records into the same scan; never trust records alone |
| Hidden or tool files | `.git`, `.DS_Store`, `Thumbs.db`, `desktop.ini`, swap files | Skip dot-prefixed names at any depth and the known files |
| Cloud-client folders | Online-only placeholders download on read; clients may create "conflicted copy" files | Per-file timeout and status; recommend keeping the folder offline-available; conflicted copies sync like any file and are listed as a known issue |
| Scale | Each `getFile()` is a round trip | Bounded concurrency scan, file-count warning, per-file size cap |

Handle lifecycle: call `showDirectoryPicker({id: 'ucblogo-backup', mode:
'readwrite'})` synchronously in the click handler (no `await` before it), store
the handle in the shared sync IndexedDB, and on load run
`queryPermission({mode: 'readwrite'})`. States: unsupported, disconnected,
connected, needs-permission (show "Reconnect folder", whose click calls
`requestPermission`), stale-handle (`NotFoundError`, offer re-pick), denied,
error. A marker file `.ucblogo-folder.json` (random id, format version) detects
re-picked or foreign folders.

## Phase 0: Spikes (throwaway page, not shipped)

1. Chrome desktop (Linux available): picker, IndexedDB handle persistence across
   a full browser restart, `queryPermission` result afterwards, any
   "remember permission" prompt, installed-PWA window behavior. Record the Chrome
   version.
2. Which folders are restricted (home, Documents, Downloads, Desktop roots,
   system folders) and the error shape.
3. Write semantics: swap-file names and visibility during `createWritable`,
   mtime updates, a file changed between `getFile()` and read.
4. A Dropbox or OneDrive desktop-client folder: placeholder reads, interaction
   with the client syncing while we write, conflicted-copy creation.
5. Android Chrome 132+ if a device is available (picker restrictions, permission
   persistence); otherwise record it as a gap.
6. Timing: scan and hash 1,000 small files; choose scan limits.
7. `FileSystemObserver` reliability on the platforms available.

Exit criteria: decided reconnect UX, restricted-folder messaging, scan limits,
and the swap-file exclusion list.

## Phase 1: Shared prerequisites

From the Dropbox plan, Phase 1 steps 1, 2, 5, and 7: the IDBFS populate promise
and `IDBFS.queuePersist` dirty signal, the pure sync core and manifest, the
Backup dialog, and the trigger wiring. If this plan ships first, those steps are
built here and the Dropbox plan consumes them. Extend the shared pieces with:

- per-provider manifests in the same IndexedDB, keyed by provider id;
- a provider capability record (`caseSensitive`, `revisions`, `rename`,
  `observe`);
- one active provider at a time in v1 (switching requires disconnect), to avoid
  conflict copies multiplying between two mirrors of `/d`.

## Phase 2: Folder provider (`backup-folder.js`)

Pure module with injected `showDirectoryPicker`, handle store, and clock, so
Node can test it with a fake directory handle.

1. Feature detection, picker call, handle store, and the permission state machine
   above.
2. First-connect cases (empty, marker, non-empty without marker with merge
   confirmation).
3. Name policy: dot-prefix and known-file skips, Windows rules, case collisions,
   NFC, path length, dangerous-extension skip on push, per-file size cap and
   file-count warning. Each skip is recorded for the UI report.
4. Read: `getFile()` then read bytes immediately; one retry if the file changed.
5. Write: stat, compare with the expected state, `createWritable()`, write,
   `close()`; create nested directories as needed.
6. Remove: files only; prune empty directories the sync created.
7. Scan: bounded-concurrency listing returning `{path, size, lastModified}`;
   optional `FileSystemObserver` that only schedules a scan.
8. Error mapping: permission revoked mid-sync (stop and show Reconnect), folder
   removed (stale handle), quota or disk full, locked file (skip and retry
   later), name too long (skip and report).

## Phase 3: UI

A "Local folder" section in the Backup dialog, following the frontend skill
(native labels and buttons, `fieldset`/`legend` for the destination choice,
`role="status"` live region, focus restored after the picker closes, theme-aware
borders, touch-target sizes):

- Disabled with an explanation when `showDirectoryPicker` is absent, pointing to
  "Download session" (and the zip export if that ships, see the investigations
  plan).
- Choose folder, Reconnect, Sync now, Disconnect, last-sync time, and the folder
  name.
- The tip to use a new empty folder, optionally inside a cloud-drive or git
  folder, with the capability warning from the threat model.
- A skipped-files report (illegal names, dangerous extensions, too large,
  collisions) and the conflicts list.
- After first successful connect call `storage.persist()` (PWA plan Issue 8).

Externally edited files pulled into `/d` are not auto-loaded; the status line
says how many files changed and the user runs `load` as usual.

## Phase 4: Tests (failing tests first)

- **Source (`node --test`, added to `test:source`):** fake in-memory directory
  handle covering the name policy (Windows illegals, reserved names, case
  collisions, NFC), extension denylist, first-connect cases, the permission state
  machine (granted, prompt, denied, stale), expected-state write conflicts,
  external-edit detection, dot-file and swap-file skipping, bounded scan
  concurrency, error mapping.
- **Browser (TestCafe, Chrome only; other browsers assert the unsupported path):**
  stub `window.showDirectoryPicker` to return a subdirectory of the origin
  private file system (`navigator.storage.getDirectory()`), which is a real
  `FileSystemDirectoryHandle` with the same read, write, remove, and IndexedDB
  serialization behavior but no native dialog. Assert: connect then a Logo `save`
  appears in the folder; a file written into the folder externally appears in
  `/d` after a `focus` scan and can be `load`ed; a reload restores the handle
  and resumes; a wrapped handle reporting `prompt` shows Reconnect; a removed
  folder reports stale; Firefox shows the control disabled with the explanation;
  accessible names and focus handling; only metadata in `localStorage`;
  `persist()` called once. Update `tests/acceptance/COVERAGE.md`. The OPFS stand-in
  cannot reproduce OS behavior (case-insensitivity, Windows names, swap files,
  permission prompts), which stays manual.
- **Manual checklist (real OS):** Linux Chrome (maintainer's machine); Windows
  and macOS are not available locally, so report them as a gap unless run on
  another machine or CI runners; Edge; a Dropbox or OneDrive client folder; a git
  working tree (`.git` untouched); Android Chrome if available. Platform-matrix
  order: desktop Chrome, mobile Chrome emulation (UI only), Firefox
  (unsupported path), iPad Safari (unsupported path, human checkpoint).

## Phase 5: Docs

- Add a "Local folder backup (Chrome and Edge)" section to
  [webedition.md](../../webedition.md): how to choose a folder, using a cloud-drive
  or git folder, the threat model and limitations, and that nothing leaves the
  device. Reword the persistence claims per PWA plan Issue 8.

## Risks

- Capability exposure to loaded Logo programs (mitigated, not eliminated).
- Reconnect friction if Chrome does not persist permission (Phase 0 decides the
  UX).
- External editor and Logo conflicts: handled by conflict copies and the shared
  safeguards.
- Cloud clients creating "conflicted copy" files or serving placeholders.
- Chrome API drift (observer and `move` are experimental): feature-detect, and
  keep scan-on-focus as the baseline.
- Engine bugs causing data loss: the shared guards (populate-before-sync,
  mass-delete confirmation, eviction-recovery rule) apply here too.

## Assumptions to confirm

- One active provider at a time in v1.
- Persist the handle in IndexedDB with the mitigations above, rather than
  memory-only handles.
- The dangerous-extension skip list and its inability to be overridden in v1.
- Chrome and Edge desktop are the supported targets for v1; Android Chrome is
  best-effort pending a device check.

## Out of scope

- Firefox, Safari, and iOS support; zip export, share sheet, and folder import
  fallbacks (investigations plan B3).
- Using OPFS as a backup (same eviction bucket as IndexedDB).
- Mounting the folder as the primary store (would need a different Emscripten
  filesystem backend).
- Logo words for controlling sync.
