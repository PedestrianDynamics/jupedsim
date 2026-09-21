#!/usr/bin/env python3
# SPDX-License-Identifier: LGPL-3.0-or-later
"""Regenerate versions.json and the ``stable`` symlink for the documentation.

Expects to run at root of a checkout of the gh-pages branch.
Every top-level directory named ``v<major>.<minor>.<patch>`` is treated as a
published documentation version.

The highest such version is makred as ``stable``: the ``stable`` symlink is
re-pointed at it, and versions.json lists a ``stable`` entry first, followed
by all versions in descending order, matching the ``switcher.json_url``
consumed by docs/source/conf.py.
"""

import argparse
import json
import re
import sys
from pathlib import Path

from packaging.version import Version

PREFIX = "https://www.jupedsim.org/"
VERSION_DIR = re.compile(r"^v\d+\.\d+\.\d+$")


def collect_versions(root: Path) -> list[str]:
    versions = [
        item.name[1:]
        for item in root.iterdir()
        if item.is_dir() and VERSION_DIR.match(item.name)
    ]
    versions.sort(key=Version, reverse=True)
    return ["v" + v for v in versions]


def update_stable_symlink(root: Path, target: str) -> None:
    link = root / "stable"
    if link.is_symlink() or link.exists():
        link.unlink()
    link.symlink_to(target)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "root",
        nargs="?",
        default=".",
        type=Path,
        help="gh-pages checkout (default: current directory)",
    )
    args = parser.parse_args()

    versions = collect_versions(args.root)
    if len(versions) == 0:
        sys.exit("No version folders found! Aborting!")
    stable = versions[0]

    entries = [
        dict(
            name="stable",
            version=stable,
            url=f"{PREFIX}stable/",
            preferred=True,
        )
    ] + [
        dict(name=version, version=version, url=f"{PREFIX}{version}/")
        for version in versions
    ]
    with (args.root / "versions.json").open("w") as f:
        json.dump(entries, f)
    update_stable_symlink(args.root, stable)


if __name__ == "__main__":
    main()
