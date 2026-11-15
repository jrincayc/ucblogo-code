# Project Plan: Backup Provider Investigations

Status: Proposed; no investigation has started.
Date: 2026-11-15

## Objective

Decide which backup and sync options beyond Dropbox
([dropbox-backup.md](dropbox-backup.md)) are worth building, using time-boxed
spikes that each end in a go/no-go. Findings are appended to this plan (or a
follow-up) so the decision record stays in the repository.

## Decision criteria

Every option is scored against the same list:

1. No backend and no secret; works from a static origin.
2. Token scope confined to the app (folder or repo), so a leaked token has a
   small blast radius. `Module.runjs` is not a sandbox and tokens must live in a
   Worker (see the Dropbox plan).
3. CORS works from the github.io origin and localhost.
4. Works in iOS Home Screen (standalone) mode and plain Safari tabs.
5. Provider terms, age limits, and school-admin approval flows are acceptable
   for a tool used by children, without the project handling user data.
6. Implementation and maintenance cost.

## Evidence so far

- Local directory access: `showDirectoryPicker` is Chromium only (Chrome and
  Edge desktop, Chrome Android 132+); Firefox and Safari lack it. OPFS exists
  everywhere but shares the origin's eviction bucket.
- GitLab.com: OAuth with PKCE and no secret works and token endpoints allow
  CORS (access tokens last 2 hours, refresh tokens rotate). Writing through
  the REST API needs the `api` scope (full account); `write_repository` is
  git-over-HTTP only. Project access tokens on GitLab.com need Premium.
- Codeberg/Forgejo: PKCE works, but OAuth2 scopes are not implemented, so
  tokens carry full account power. Its terms target FLOSS projects and say
  private repositories are "explicitly not" a personal cloud storage, so
  automated per-user private repos are likely out of policy.
- Not yet verified: CORS on `/api/v4` and Codeberg's API, GitHub's current OAuth
  and CORS status, and everything under Microsoft, Google, and Apple.

## Tracks

Sequence: B0, B3, B1, B2, B4, B5, B6, with B7 running alongside from the start
and B8 after Dropbox ships.

### B0: Shared spikes (shared with Dropbox Phase 0)

- Question: does popup auth survive iPad standalone mode and COOP, and is a
  Worker-held token unreadable by a direct-eval `runjs`?
- Method: reuse the Dropbox spike pages; add a one-page proof that `runjs` can
  read pre.js top-level variables but not Worker state.
- Output: an auth-flow matrix per browser and mode.

### B3: No-account paths (cheapest, widest reach; consider promoting before Dropbox)

- Zip export and import: a stored-zip writer (no dependency) with test vectors;
  import through `<input webkitdirectory>` and zip.
- Share sheet: `navigator.canShare({files})` on iPad, Android, and desktop, to
  save to Files, iCloud Drive, or another provider's app.
- The Chromium File System Access sync provider has its own plan:
  [local-folder-backup.md](local-folder-backup.md).
- Export reminders based on last export versus last change, using standalone
  detection (PWA Issue 10).
- Output: ship or no-ship per path.

### B1: OneDrive (Microsoft Graph)

- Check SPA registration with PKCE, token endpoint CORS, 24-hour refresh tokens,
  and the `Files.ReadWrite.AppFolder` scope on personal accounts.
- Check school and work tenants: user-consent policies, and whether unverified
  publishers are blocked (publisher verification may be required).
- Check delta queries, ETag conflict handling, upload limits, and Microsoft
  account age and family-consent rules.
- Output: go/no-go and the verification cost.

### B2: Google Drive

- Check the Identity Services token model (about one hour, no refresh token),
  `drive.file` versus `appdata`, publishing-status limits, and branding
  verification.
- Check Workspace for Education admin app-access controls, folder-ID mapping
  (no real paths), conflict detection by revision or checksum, and under-13
  rules.
- Output: go/no-go.

### B4: Git hosting

- GitHub: fine-grained token limited to one repository; CORS, rate limits,
  contents API versus git-data API for atomic commits; token kept in the Worker
  with a re-paste UX.
- GitLab: probe `/api/v4` CORS from a github.io origin; viable only with the
  Worker isolation because the token needs `api`.
- Codeberg: ask the Codeberg team whether automated private repositories are
  acceptable and probe CORS; default is to drop it.
- Optional: user-supplied Forgejo/Gitea/GitLab URL with a pasted token.
- Output: keep a GitHub token tier; decide on GitLab; likely drop Codeberg.

### B5: iCloud via CloudKit JS

- Check Apple Developer Program cost, container setup, private-database asset
  limits, sign-in in standalone mode, children's Apple IDs, and whether the
  JavaScript library is maintained.
- Output: go/no-go; relevant because Safari users are the eviction risk.

### B6: Other options (about an hour each)

- WebDAV/Nextcloud (CORS configuration burden), remoteStorage, Solid pods, and
  hobbyist code hosts (confirm whether Glitch shut down; others own the account).
- Output: expected to be no-go; record why.

### B7: Legal and privacy checklist (maintainer and counsel)

- Privacy policy content, per-provider age and terms, school-admin approval
  flows, and a written statement that the client-only design sends no user data
  to the project. This plan does not make legal determinations.

### B8: Provider abstraction review (after Dropbox ships)

- List the capabilities the interface needed (paths, case sensitivity,
  revisions, delta, listing cost) and adjust before adding providers.

## Method notes

- Documentation reads can be done by fetching vendor docs; live CORS checks need
  a throwaway static page on localhost and the github.io origin, or an
  `OPTIONS` request with `Origin` and `Access-Control-Request-Headers`.
- Test real services only with throwaway accounts and profiles.
- Record each result, source link, and date; mark claims that could not be
  verified.

## Out of scope

- Building any provider before its track returns a go.
- A project-operated backend or proxy of any kind.
