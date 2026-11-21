#!/usr/bin/env python3
"""Report granular GitHub agent usage from the repository accounting ledger."""

# Co-authored by GitHub Copilot. Model: MAI Code 2; reasoning-effort setting unavailable.

from __future__ import annotations

import argparse
import re
import subprocess
from dataclasses import dataclass
from datetime import datetime, timezone
from decimal import Decimal, InvalidOperation
from pathlib import Path

MONEY_RE = re.compile(r"\$([0-9]+(?:\.[0-9]+)?)")
SHORT_HASH_RE = re.compile(r"^[0-9a-f]{7,40}$")


@dataclass
class Snapshot:
    heading: str
    timestamp: datetime | None
    checkpoint: str
    commit: str | None
    gross: Decimal | None
    discount: Decimal | None
    net: Decimal | None
    rollup: list[tuple[str, str, str, str, Decimal, Decimal, Decimal]]


@dataclass
class Commit:
    full_hash: str
    timestamp: datetime
    subject: str


def parse_money(value: str) -> Decimal | None:
    match = MONEY_RE.search(value)
    if not match:
        return None
    try:
        return Decimal(match.group(1))
    except InvalidOperation:
        return None


def parse_timestamp(value: str) -> datetime | None:
    raw = value.strip().split()[0]
    try:
        parsed = datetime.fromisoformat(raw.replace("Z", "+00:00"))
        return parsed if parsed.tzinfo else parsed.replace(tzinfo=timezone.utc)
    except ValueError:
        return None


def parse_rollup(block: str) -> list[tuple[str, str, str, str, Decimal, Decimal, Decimal]]:
    rows: list[tuple[str, str, str, str, Decimal, Decimal, Decimal]] = []
    for line in block.splitlines():
        if not line.startswith("|") or "---" in line:
            continue
        cells = [cell.strip() for cell in line.strip().strip("|").split("|")]
        if len(cells) != 7 or cells[0] in {"Product", "Usage", "Task"}:
            continue
        gross, discount, net = map(parse_money, cells[4:7])
        if gross is None or discount is None or net is None:
            continue
        rows.append((cells[0], cells[1], cells[2], cells[3], gross, discount, net))
    return rows


def parse_snapshots(text: str) -> list[Snapshot]:
    headings = list(re.finditer(r"^## (.+)$", text, re.MULTILINE))
    snapshots: list[Snapshot] = []
    for index, match in enumerate(headings):
        end = headings[index + 1].start() if index + 1 < len(headings) else len(text)
        block = text[match.end():end]
        timestamp_match = re.search(r"^- UTC timestamp: `([^`]+)`", block, re.MULTILINE)
        checkpoint_match = re.search(r"^- Checkpoint: `([^`]+)`", block, re.MULTILINE)
        commit_match = re.search(r"^- Branch / current commit: `[^`]+` / `([^`]+)`", block, re.MULTILINE)
        totals_match = re.search(r"Gross / discount / net: `([^`]+)` / `([^`]+)` / `([^`]+)`", block)
        gross = discount = net = None
        if totals_match:
            gross, discount, net = map(parse_money, totals_match.groups())
        snapshots.append(Snapshot(
            heading=match.group(1),
            timestamp=parse_timestamp(timestamp_match.group(1)) if timestamp_match else None,
            checkpoint=checkpoint_match.group(1) if checkpoint_match else "note",
            commit=commit_match.group(1) if commit_match else None,
            gross=gross,
            discount=discount,
            net=net,
            rollup=parse_rollup(block),
        ))
    return snapshots


def git_commits(repo: Path, since: str, until: str) -> list[Commit]:
    command = [
        "git", "-C", str(repo), "log", "--all", f"--since={since}", f"--until={until}",
        "--format=%H%x00%aI%x00%s",
    ]
    result = subprocess.run(command, check=True, capture_output=True, text=True)
    commits: list[Commit] = []
    for line in result.stdout.splitlines():
        fields = line.split("\0")
        if len(fields) != 3:
            continue
        timestamp = parse_timestamp(fields[1])
        if timestamp:
            commits.append(Commit(fields[0], timestamp, fields[2]))
    return commits


def matching_commit(commit: str | None, commits: list[Commit]) -> Commit | None:
    if not commit or not SHORT_HASH_RE.fullmatch(commit):
        return None
    return next((candidate for candidate in commits if candidate.full_hash.startswith(commit)), None)


def print_money(value: Decimal | None) -> str:
    return "UNAVAILABLE" if value is None else f"${value:.9f}"


def report(snapshots: list[Snapshot], commits: list[Commit]) -> None:
    measured = sorted(
        (snapshot for snapshot in snapshots if snapshot.timestamp and snapshot.gross is not None and snapshot.net is not None),
        key=lambda snapshot: snapshot.timestamp,
    )
    latest = measured[-1] if measured else None
    print(f"Measured snapshots: {len(measured)}")
    if not latest:
        print("No measured usage snapshots found.")
        return
    latest_rollup = next((snapshot.rollup for snapshot in reversed(measured) if snapshot.rollup), [])

    print("\nLatest cumulative total")
    print(f"  Time: {latest.timestamp.isoformat()}")
    print(f"  Gross / discount / net: {print_money(latest.gross)} / {print_money(latest.discount)} / {print_money(latest.net)}")
    if latest_rollup:
        print("\nLatest product / SKU breakdown")
        for product, sku, quantity, unit, gross, discount, net in latest_rollup:
            print(f"  {product} | {sku} | {quantity} {unit} | gross {print_money(gross)} | discount {print_money(discount)} | net {print_money(net)}")

    print("\nSnapshot progression")
    previous_net: Decimal | None = None
    for snapshot in measured:
        delta = None if previous_net is None or snapshot.net is None else snapshot.net - previous_net
        commit_label = snapshot.commit[:7] if snapshot.commit else "-"
        delta_label = "baseline" if delta is None else print_money(delta)
        print(f"  {snapshot.timestamp.isoformat()} | {commit_label} | {snapshot.checkpoint} | net {print_money(snapshot.net)} | delta {delta_label}")
        previous_net = snapshot.net

    print("\nAttribution by recorded Git commit")
    grouped: dict[str, Decimal] = {}
    previous: Snapshot | None = None
    for snapshot in measured:
        if previous is not None and previous.net is not None and snapshot.net is not None:
            grouped[previous.commit or "UNRECORDED"] = grouped.get(previous.commit or "UNRECORDED", Decimal(0)) + snapshot.net - previous.net
        previous = snapshot

    for commit_hash, delta in grouped.items():
        commit = matching_commit(commit_hash, commits)
        if commit:
            label = f"{commit.full_hash[:7]} | {commit.timestamp.isoformat()} | {commit.subject}"
        else:
            label = f"{commit_hash} | commit not in selected Git history"
        print(f"  {label} | net delta {print_money(delta)}")

    total_delta = sum(grouped.values(), Decimal(0))
    print(f"\nMeasured interval total: {print_money(total_delta)}")
    print(f"Estimated CO2e: {total_delta * Decimal('0.20'):.9f} kg (approved gross rule-of-thumb estimate, not measured emissions)")
    print("\nCaution: snapshots are cumulative account-level totals. Deltas are temporal attribution to the previously recorded HEAD, not proof that a commit caused the usage.")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repo", type=Path, default=Path.cwd(), help="repository root")
    parser.add_argument("--ledger", type=Path, help="ledger path (default: REPO/docs/accounting/agent-usage-ledger.md)")
    parser.add_argument("--since", default="today", help="Git history lower bound (git date syntax)")
    parser.add_argument("--until", default="tomorrow", help="Git history upper bound (git date syntax)")
    args = parser.parse_args()

    repo = args.repo.resolve()
    ledger = args.ledger.resolve() if args.ledger else repo / "docs/accounting/agent-usage-ledger.md"
    snapshots = parse_snapshots(ledger.read_text())
    commits = git_commits(repo, args.since, args.until)
    report(snapshots, commits)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
