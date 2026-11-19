#!/usr/bin/env python3

# Co-authored by GitHub Copilot. Scope: repository-local UCB Logo help lookup CLI.
# Model identity and selected reasoning-effort setting are unavailable; elapsed
# time and environmental accounting are recorded in
# docs/workflow-runs/UCB-LOGO-HELP-LOOKUP-2026-11-18.md.

import argparse
from pathlib import Path
import sys


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Print the UCB Logo help file for a word."
    )
    parser.add_argument("logo_word", help="Logo word name")
    args = parser.parse_args()

    help_directory = Path(__file__).resolve().parents[4] / "helpfiles"
    resolved_help_directory = help_directory.resolve()
    logo_word = args.logo_word.casefold()
    if "/" in logo_word or "\\" in logo_word or logo_word in {".", ".."}:
        parser.error("Logo word must be a single name")

    help_path = (resolved_help_directory / logo_word).resolve()
    if help_path.parent != resolved_help_directory:
        parser.error("Logo word must name a file directly under helpfiles")
    if not help_path.is_file():
        print(f"No help file for Logo word: {args.logo_word}", file=sys.stderr)
        return 1

    try:
        sys.stdout.write(help_path.read_text(encoding="utf-8"))
    except (OSError, UnicodeError) as error:
        print(
            f"Could not read help file for Logo word {args.logo_word}: {error}",
            file=sys.stderr,
        )
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main())