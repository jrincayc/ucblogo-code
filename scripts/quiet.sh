#!/bin/sh
# Usage: scripts/quiet.sh [-n LINES] command [args...]
# Runs the command with all output saved to a log. Silent on success; on failure
# prints the last LINES (default 30) lines, the log path, and keeps the exit code.
n=30
if [ "${1:-}" = "-n" ]; then n=$2; shift 2; fi
[ $# -gt 0 ] || { echo "usage: $0 [-n LINES] command [args...]" >&2; exit 2; }
log=$(mktemp "${TMPDIR:-/tmp}/quiet.XXXXXX.log")
if "$@" >"$log" 2>&1; then
  rm -f "$log"
else
  rc=$?
  tail -n "$n" "$log" | cut -c1-260
  echo "exit $rc; full log: $log" >&2
  exit "$rc"
fi
