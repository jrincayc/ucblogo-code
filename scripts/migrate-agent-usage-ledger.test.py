#!/usr/bin/env python3
"""Focused read-only migration audit fixtures for commit-native accounting."""

from __future__ import annotations

import hashlib
import json
import tempfile
import unittest
from pathlib import Path


class LedgerMigrationAudit(unittest.TestCase):
    def audit(self, entries):
        reconciled = []
        unresolved = []
        seen = set()
        for entry in entries:
            digest = hashlib.sha256(json.dumps(entry, sort_keys=True).encode("utf-8")).hexdigest()
            if digest in seen:
                unresolved.append({"classification": "UNRESOLVED", "reason": "duplicate snapshot"})
                continue
            seen.add(digest)
            if entry.get("valid") is True:
                reconciled.append({"classification": "reconciled", "sha256": digest})
            else:
                unresolved.append({"classification": "UNRESOLVED", "reason": entry.get("reason", "insufficient evidence")})
        report = {"reconciled": reconciled, "unresolved": unresolved, "entries": len(entries)}
        checksum = hashlib.sha256(json.dumps(report, sort_keys=True).encode("utf-8")).hexdigest()
        return report, checksum

    def test_well_formed_and_corrupt_entries(self):
        entries = [
            {"valid": True, "timestamp": "2026-11-20T22:01:50Z", "net": "9.456402180"},
            {"valid": False, "reason": "malformed but no verified Git or API resolution"},
            {"valid": True, "timestamp": "2026-11-21T17:06:05Z", "net": "15.356469620"},
        ]
        report, checksum = self.audit(entries)
        self.assertEqual(report["entries"], 3)
        self.assertEqual(len(report["reconciled"]), 2)
        self.assertEqual(len(report["unresolved"]), 1)
        self.assertEqual(checksum, hashlib.sha256(json.dumps(report, sort_keys=True).encode("utf-8")).hexdigest())

    def test_duplicate_does_not_double_count(self):
        duplicate = {"valid": True, "timestamp": "2026-11-20T22:01:50Z", "net": "9.456402180"}
        report, _ = self.audit([duplicate, duplicate])
        self.assertEqual(len(report["reconciled"]), 1)
        self.assertEqual(len(report["unresolved"]), 1)

    def test_source_bytes_are_not_modified(self):
        with tempfile.TemporaryDirectory(prefix="ledger-migration-") as directory:
            ledger = Path(directory) / "legacy-ledger.md"
            before = b"# Legacy\n\nmalformed-but-preserved\n"
            ledger.write_bytes(before)
            checksum_before = hashlib.sha256(ledger.read_bytes()).hexdigest()
            self.audit([{"valid": True}, {"valid": False, "reason": "unresolvable"}])
            checksum_after = hashlib.sha256(ledger.read_bytes()).hexdigest()
            self.assertEqual(before, ledger.read_bytes())
            self.assertEqual(checksum_before, checksum_after)


if __name__ == "__main__":
    unittest.main()
