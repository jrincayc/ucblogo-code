#!/usr/bin/env python3
"""Compatibility wrapper for commit-native GitHub agent usage reporting."""

# Co-authored by GitHub Copilot. Model: MAI Code 2; reasoning-effort setting unavailable.

from __future__ import annotations

import argparse
import subprocess
from pathlib import Path


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repo", type=Path, default=Path.cwd(), help="repository root")
    parser.add_argument("--ledger", type=Path, help="retired; ledger reporting is no longer supported")
    parser.add_argument("--range", help="exact Git revision range")
    parser.add_argument("--since", help="Git history lower bound")
    parser.add_argument("--until", help="Git history upper bound")
    parser.add_argument("--issue", action="store_true", help="emit concise issue-oriented output")
    args = parser.parse_args()

    if args.ledger is not None:
        parser.error("the append-only ledger is retired; use --repo and --range, --since, or --until")

    command = ["node", str(args.repo.resolve() / "scripts" / "report-agent-usage-v2.js"), "--repo", str(args.repo.resolve())]
    if args.range:
        command.extend(["--range", args.range])
    if args.since:
        command.extend(["--since", args.since])
    if args.until:
        command.extend(["--until", args.until])
    if args.issue:
        command.append("--issue")

    result = subprocess.run(command, check=False)
    return result.returncode


if __name__ == "__main__":
    raise SystemExit(main())
