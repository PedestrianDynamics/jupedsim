#!/usr/bin/env python3
# SPDX-License-Identifier: LGPL-3.0-or-later
"""Regenerate docs/versions.json and the docs/stable symlink.

Takes the ``docs/`` directory of a gh-pages checkout (or of a local site
build) as its only argument. Every directory directly inside it named
``v<major>.<minor>.<patch>`` is a published documentation version; other
entries (``v1x``, the frozen 1.x archive, ``stable``, ``versions.json``) are
not versions.

The highest version (numeric order, v2.10.0 > v2.9.0) becomes ``stable``:
``<docs>/stable`` is re-pointed at it with a relative symlink (a plain
``stable`` directory, i.e. the pre-2.0 redirect stub, is removed first), and
``<docs>/versions.json`` lists a ``stable`` entry marked preferred, followed
by all versions newest first. If ``<docs>/v1x`` (the frozen 1.x archive,
which has no ``stable`` of its own) holds at least one ``vX.Y.Z`` directory,
a last entry ``1.x (archive)`` pointing at the newest of them
(``<docs>/v1x/<newest>/``) follows; it is never ``stable`` and never
preferred.
This file is read by the theme's version switcher (``switcher_json_url``).

Nothing outside ``<docs>`` is touched; in particular the root
``versions.json`` of gh-pages (the frozen copy the 1.x builds read) is left
alone. Running the script again without new versions changes nothing.
"""

import argparse
import json
import re
import shutil
import sys
from pathlib import Path

from packaging.version import Version

PREFIX = "https://www.jupedsim.org/docs/"
VERSION_DIR = re.compile(r"^v\d+\.\d+\.\d+$")
ARCHIVE_DIR = "v1x"


def collect_versions(root: Path) -> list[str]:
    versions = [
        item.name
        for item in root.iterdir()
        if VERSION_DIR.match(item.name) and item.is_dir()
    ]
    versions.sort(key=lambda name: Version(name[1:]), reverse=True)
    return versions


def update_stable_symlink(root: Path, target: str) -> None:
    link = root / "stable"
    if link.is_symlink():
        if str(link.readlink()) == target:
            return
        link.unlink()
    elif link.is_dir():
        shutil.rmtree(link)
    elif link.exists():
        link.unlink()
    link.symlink_to(target, target_is_directory=True)


def write_if_changed(path: Path, content: str) -> None:
    if path.is_file() and not path.is_symlink():
        if path.read_text(encoding="utf-8") == content:
            return
    elif path.is_symlink() or path.exists():
        sys.exit(f"{path} exists but is not a regular file! Aborting!")
    path.write_text(content, encoding="utf-8")


def main() -> None:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument(
        "root",
        type=Path,
        help="the docs/ directory (e.g. docs in a gh-pages checkout)",
    )
    args = parser.parse_args()
    root: Path = args.root

    if not root.is_dir():
        sys.exit(f"{root} is not a directory! Aborting!")
    versions = collect_versions(root)
    if len(versions) == 0:
        sys.exit(f"No version folders found in {root}! Aborting!")
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
    archive = root / ARCHIVE_DIR
    archive_versions = collect_versions(archive) if archive.is_dir() else []
    if archive_versions:
        entries.append(
            dict(
                name="1.x (archive)",
                version=ARCHIVE_DIR,
                url=f"{PREFIX}{ARCHIVE_DIR}/{archive_versions[0]}/",
            )
        )
    write_if_changed(
        root / "versions.json", json.dumps(entries, indent=2) + "\n"
    )
    update_stable_symlink(root, stable)


if __name__ == "__main__":
    main()
