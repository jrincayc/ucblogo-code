#!/usr/bin/env sh
# Ledger appends are retired; use commit-native COPILOT-ACCOUNTING blocks.
set -eu

cat >&2 <<'EOF'
The append-only agent usage ledger is retired. Create or amend no ledger entry.
Use scripts/report-agent-usage-v2.js for commit-native reporting.
EOF
exit 2
