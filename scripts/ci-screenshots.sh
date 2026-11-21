#!/bin/sh
# Usage: scripts/ci-screenshots.sh [run-id|branch] [repo]
# Downloads the "WebAssembly Test Reports" artifact of a run into /tmp/ci-screenshots/<repo>-<run-id>/
# (cached; CI_CACHE=0 refetches) and prints the failure screenshot paths, one per line.
# Defaults: latest run on the current branch in origin. PR runs live on upstream.
set -eu
arg=${1:-$(git rev-parse --abbrev-ref HEAD)}
repo=${2:-$(git remote get-url origin | sed -E 's#.*github.com[:/]##; s#\.git$##')}
case $arg in
  *[!0-9]*) id=$(gh run list --branch "$arg" -R "$repo" --limit 1 --json databaseId -q '.[0].databaseId') ;;
  *) id=$arg ;;
esac
[ -n "$id" ] || { echo "no runs for $arg in $repo" >&2; exit 2; }
dir=${TMPDIR:-/tmp}/ci-screenshots/$(echo "$repo" | tr / _)-$id
if [ "${CI_CACHE:-1}" = 0 ] || [ ! -e "$dir/.done" ]; then
  rm -rf "$dir"; mkdir -p "$dir"
  gh run download "$id" -R "$repo" -n "WebAssembly Test Reports" -D "$dir" >/dev/null ||
    { echo "no WebAssembly Test Reports artifact for run $id (still running, expired, or no upload)" >&2; exit 1; }
  touch "$dir/.done"
fi
find "$dir" -name '*.png' -not -path '*/thumbnails/*' | sort | tee "$dir/.list"
[ -s "$dir/.list" ] || echo "run $id artifact has no screenshots (no failures, or run predates screenshot capture)" >&2
