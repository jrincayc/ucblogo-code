# C Buffer Safety Follow-up Plan

Status: Planned; implementation has not started.
Date: 2026-11-15

## Objective

Resolve the fixed-buffer overrun risks and data-loss behaviors found in the
source audit. Keep Logo behavior and the existing picture-file format
compatible where possible, and make any necessary limits explicit rather than
silently corrupting memory or data.

This plan follows the source review of the C files; it is not a claim that every
buffer in the repository has been exhaustively proven safe. Each candidate is
rechecked against the current code and its callers before implementation.

## Findings To Address

### User-controlled or malformed-input writes

- [lists.c](../../lists.c): `lform` writes to `result[100]` with `sprintf`;
  precision is uncapped, and the negative-width path uses the Logo input as a
  C format string.
- [wrksp.c](../../wrksp.c): `fixhelp` copies arbitrary word lengths into
  `result[32]`; `addsep` copies arbitrary paths into `result[256]`; `leditfile`
  copies an arbitrary Logo word into `tmp_filename[500]`; `cpdf_newname` builds
  a title in `buf[2000]` without bounding the combined parts.
- [files.c](../../files.c): `silent_load` combines a prefix with a name capped
  at 150 bytes in `load_path[200]`, without bounding the prefix or total.
- [init.c](../../init.c): `SUGAR_BUNDLE_PATH` is copied and extended in three
  100-byte arrays without length checks.
- [paren.c](../../paren.c): `missing_space` splits a name into two 100-byte
  arrays without checking either part's length.
- [print.c](../../print.c): array-origin formatting writes a `FIXNUM` into
  the few bytes remaining after `@` in a string-literal-sized local array;
  accepted origins can require more decimal digits than fit.
- [term.c](../../term.c): termcap output is written through an unbounded
  callback into fixed capability arrays; review the corresponding wx/web
  adapters and the termcap library contract as part of the fix.
- [graphics.c](../../graphics.c): `lloadpict` accepts a final record index
  relative to `GR_SIZE`, then reads from after the pointer-sized header. Check
  the actual payload extent and validate records before playback.
- [graphics.c](../../graphics.c): `safe_to_save` reserves a fixed 300-byte
  tail, while `save_string` writes a variable-length label and a terminator.
  The label length is also stored in one byte, so lengths above 255 corrupt
  playback alignment even when they fit in the array.

### Truncation and format-contract risks

- [init.c](../../init.c): message lines are read in 98-byte chunks and have
  their last byte removed unconditionally; long lines split and can shift the
  message table. Bounded `snprintf` path construction can also silently use an
  incomplete `Messages` path.
- [coms.c](../../coms.c): `lshell` caps command rendering at 299 bytes, then
  executes the truncated command.
- [obj.c](../../obj.c): `newplate` formats a monotonically increasing `FIXNUM`
  in 20 bytes; assess the platform's `long` width and counter lifetime.
- Audit shared string-copy helpers and all fixed-buffer callers for capacity
  assumptions and expansion (including terminators), rather than changing a
  helper based on its name alone.

## Constraints And Decisions

- Preserve documented Logo semantics and the wxWidgets, terminal, and Web
  editions. Keep platform-specific fixes behind the existing build guards.
- Check lengths before addition and multiplication; include terminator space.
  Prefer dynamically sized storage for naturally unbounded paths/text, or
  reject an oversized input with a Logo error where the API needs a fixed
  format. Do not silently truncate executable commands or filenames.
- Do not pass Logo-controlled format strings to C `printf` functions. Preserve
  only the documented `form` syntax; parse/validate it or replace it with
  bounded formatting and explicit limits.
- Retain compatibility with existing picture files. If the one-byte label
  length must change, choose and test an explicit backward-compatible encoding
  or reject unsupported labels before writing a picture; never wrap the
  length.
- Preserve the caller's data and current worktree. Build-profile changes must
  follow the clean-before-reconfigure rule in
  [copilot-instructions.md](../../.github/copilot-instructions.md#artifact-freshness).

## Milestones

### M0: Reconfirm and baseline

1. Recheck each finding at its current source and call sites; record the
   actual maximum input, destination size, and whether the effect is an
   overrun, truncation, or both.
2. Capture baseline behavior for valid boundary-sized Logo inputs and picture
   files. Identify the existing test location for each primitive.
3. Use a separate configured build tree for ASan if switching profiles would
   otherwise replace a configured Makefile before its current objects are
   cleaned.

### M1: Bound direct input-to-buffer writes

1. Fix `lform`, `fixhelp`, `addsep`, `leditfile`, and `cpdf_newname` with
   checked sizing or explicit validation. Add regression cases for inputs at
   and beyond the previous buffer boundary.
2. Fix `silent_load`, `SUGAR_BUNDLE_PATH`, and `missing_space` using checked
   combined lengths or dynamic storage. Exercise long prefixes, names, and
   digit/alphabetic splits.
3. Size array-origin formatting for the complete signed `FIXNUM` range and
  test ordinary, negative, and boundary origins with `fullprintp` enabled.
4. Bound terminal capability collection in each active backend; test a
   capability longer than the old destination size, plus the no-capability
   case.

### M2: Harden graphics recording and file loading

1. Make record reservation depend on the complete record size and alignment,
   not a fixed tail estimate. Check every variable-length record write before
   mutating the buffer.
2. Define safe behavior for labels beyond the serialized one-byte length and
   for labels at a buffer boundary. Preserve old picture playback and add
   round-trip coverage for boundary labels.
3. Validate `rec_idx`, chained-buffer payload lengths, record tags, and record
   field extents before accepting a picture. Add malformed/truncated picture
   fixtures that are rejected cleanly under ASan.

### M3: Make truncation explicit

1. Read message entries as complete lines or reject overlong entries; remove a
   trailing newline only when one was actually read. Test empty, maximum-size,
   overlong, and premature-EOF message files.
2. Detect incomplete `Messages` paths and shell-command rendering. For paths
   and commands, return a clear error rather than executing/opening a changed
   value. Preserve intentional display limits only where they are documented.
3. Check `newplate` counter formatting and the generic string-copy helpers;
   add boundary and expansion cases for each helper that still has callers.

### M4: Verification and documentation

1. Run the focused Logo regressions and `make check` in the native build.
2. Run the same shared-C Logo regressions in the Web build, then `npm run
   test:source` and the focused browser case if a UI-visible behavior changed.
3. Run the wx smoke/build gate where the wx toolchain is available. Run the
   focused overlong-input and malformed-picture cases under AddressSanitizer.
4. Update user documentation only for behavior or limits that change; record
   unsupported platform checks explicitly.

## Exit Criteria

- No identified write can exceed its allocated destination for accepted
  inputs; oversized inputs fail predictably or use correctly sized storage.
- Malformed picture files are rejected before playback reads outside validated
  record extents, and valid legacy pictures still load.
- No command, path, message entry, or serialized length is silently changed by
  a fixed-buffer boundary.
- Focused boundary tests, native `make check`, Web source tests, available wx
  checks, and ASan cases pass. Any unavailable platform validation is reported.

## Attribution

Co-authored with GitHub Copilot.
