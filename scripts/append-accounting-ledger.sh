#!/usr/bin/env sh
# Serialize accounting-ledger appends within a Git worktree.
# Co-authored-by: GitHub Copilot. Model: MAI Code 2; reasoning effort unavailable.
set -eu

usage() {
    echo "Usage: $0 SNAPSHOT_FILE" >&2
    exit 2
}

[ "$#" -eq 1 ] || usage
snapshot=$1
repo_root=$(git rev-parse --show-toplevel 2>/dev/null) || {
    echo "Must run inside a Git worktree." >&2
    exit 1
}
ledger="$repo_root/docs/accounting/agent-usage-ledger.md"
lock="$repo_root/.git/agent-usage-ledger.lock"

[ -f "$snapshot" ] || {
    echo "Snapshot file not found: $snapshot" >&2
    exit 1
}
[ -s "$snapshot" ] || {
    echo "Snapshot file is empty: $snapshot" >&2
    exit 1
}
[ -f "$ledger" ] || {
    echo "Accounting ledger not found: $ledger" >&2
    exit 1
}

command -v flock >/dev/null 2>&1 || {
    echo "flock is required to append accounting snapshots." >&2
    exit 1
}

# Keep the file descriptor open for the complete read/check/append operation.
exec 9>"$lock"
flock -x 9

# Add one separating newline, then append the nonempty snapshot unchanged.
printf '\n' >> "$ledger"
cat "$snapshot" >> "$ledger"

# Ensure exactly one final newline in the appended snapshot without rewriting
# or truncating existing ledger content.
last_byte=$(tail -c 1 "$ledger" | od -An -tu1 | tr -d ' ')
[ "$last_byte" = "10" ] || printf '\n' >> "$ledger"

# Detect accidental truncation/replacement by the append operation.
git diff --check -- "$ledger" >/dev/null
