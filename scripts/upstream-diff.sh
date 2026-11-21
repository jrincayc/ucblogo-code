#!/bin/sh
# Usage: scripts/upstream-diff.sh {fetch|files|legacy|log|find|diff} [args]
# Compares HEAD with upstream/master since their merge base. Silent when nothing differs.
#   fetch              refresh upstream/master
#   files [path...]    name-status of every change (A/M/D/R)
#   legacy [path...]   only upstream files the branch modified, deleted, or renamed
#   log [path...]      one line per branch commit touching the paths
#   find STR [path...] branch commits that added or removed STR (git log -S)
#   diff path...       minimal-context diff against the merge base
set -eu
cmd=${1:-files}
[ $# -gt 0 ] && shift
base=$(git merge-base upstream/master HEAD)

case "$cmd" in
  fetch)  git fetch -q upstream master ;;
  files)  git diff -M --name-status "$base" HEAD -- "$@" ;;
  legacy) git diff -M --name-status --diff-filter=MDR "$base" HEAD -- "$@" ;;
  log)    git log --oneline "$base"..HEAD -- "$@" ;;
  find)   s=$1; shift; git log --oneline -S"$s" "$base"..HEAD -- "$@" ;;
  diff)   git diff -U1 "$base" HEAD -- "$@" ;;
  *)      echo "unknown command: $cmd" >&2; exit 2 ;;
esac
