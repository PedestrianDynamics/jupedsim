# SPDX-License-Identifier: LGPL-3.0-or-later
"""``docs/versions.json`` and the ``docs/stable`` symlink.

Takes the ``docs/`` directory of a Pages checkout or of a local website build.
Every directory directly inside it named ``vX.Y.Z`` is a published
documentation version; other entries (``v1x``, ``stable``,
``versions.json``) are not versions.

The highest version (numeric order, ``v2.10.0`` > ``v2.9.0``) becomes
``stable``: ``<docs>/stable`` is re-pointed at it with a relative symlink
(a plain directory or file there, e.g. the pre-2.0 redirect stub, is removed
first), and ``<docs>/versions.json`` lists a ``stable`` entry marked
preferred, followed by all versions newest first. The theme's version
switcher reads this file (``switcher_json_url``).

``<docs>/v1x/`` is the hand-curated 1.x documentation, a self-contained
drop of the old website; the tooling never reads or writes anything beneath it
and it never appears in ``versions.json``.

A build whose version is not a release name (``build-website --version
stable``, the PR previews) lists just itself and gets no ``stable`` link;
``build-website`` does that through ``write_dev_versions``.

Nothing outside ``<docs>`` is touched, and running again without changes
rewrites nothing.
"""

from __future__ import annotations

import json
import re
import shutil
from dataclasses import dataclass
from pathlib import Path
from typing import NotRequired, TypedDict

from packaging.version import Version

from jps_website_tooling.errors import ToolError, UsageError

PREFIX = "https://www.jupedsim.org/docs/"
VERSION_DIR = re.compile(r"^v\d+\.\d+\.\d+$")


class VersionEntry(TypedDict):
    """One entry of ``versions.json``, as the theme's switcher reads it.

    Attributes:
        name: Label shown in the switcher.
        version: Directory name under ``docs/`` the entry stands for.
        url: Absolute URL (``https://www.jupedsim.org/docs/<dir>/``) or, for
            development builds, the root-relative path.
        preferred: Marks the entry selected by default; only ``stable`` or
            a development build carry it.
    """

    name: str
    version: str
    url: str
    preferred: NotRequired[bool]


@dataclass(frozen=True)
class VersionsResult:
    """What a writer did to ``<docs>``.

    Attributes:
        stable: The version ``stable`` points at; None for a development
            build, which has no ``stable`` link.
        json_changed: Whether ``versions.json`` was (re)written.
        link_changed: Whether the ``stable`` link was created or re-pointed.
    """

    stable: str | None
    json_changed: bool
    link_changed: bool

    @property
    def changed(self) -> bool:
        """Whether anything in ``<docs>`` was written."""
        return self.json_changed or self.link_changed


def collect_versions(root: Path) -> list[str]:
    """List the release directories directly inside a directory.

    Args:
        root: Directory to scan (``<docs>``).

    Returns:
        Names matching ``vX.Y.Z`` that are directories, newest first by
        numeric version order.
    """
    versions = [
        item.name
        for item in root.iterdir()
        if VERSION_DIR.match(item.name) and item.is_dir()
    ]
    versions.sort(key=lambda name: Version(name[1:]), reverse=True)
    return versions


def entries(docs: Path) -> list[VersionEntry]:
    """Compute the ``versions.json`` content for a release layout.

    Args:
        docs: The ``docs/`` directory holding the version directories.

    Returns:
        The ``stable`` entry (preferred), then one entry per version,
        newest first.

    Raises:
        UsageError: If ``docs`` is not a directory or holds no version.
    """
    if not docs.is_dir():
        raise UsageError(f"{docs} is not a directory")
    versions = collect_versions(docs)
    if not versions:
        raise UsageError(f"no version directories (vX.Y.Z) in {docs}")
    stable = versions[0]
    result: list[VersionEntry] = [
        VersionEntry(
            name="stable",
            version=stable,
            url=f"{PREFIX}stable/",
            preferred=True,
        )
    ]
    result.extend(
        VersionEntry(name=v, version=v, url=f"{PREFIX}{v}/") for v in versions
    )
    return result


def dev_entries(version: str) -> list[VersionEntry]:
    """Compute the ``versions.json`` content of a development build.

    Args:
        version: The directory name of the build, e.g. ``dev``.

    Returns:
        A single preferred entry pointing at ``/docs/<version>/``.
    """
    return [
        VersionEntry(
            name=version,
            version=version,
            url=f"/docs/{version}/",
            preferred=True,
        )
    ]


def _write_json(docs: Path, content: list[VersionEntry]) -> bool:
    """Write ``<docs>/versions.json`` if its content differs.

    Args:
        docs: The ``docs/`` directory.
        content: The entries to serialise (indent 2, trailing newline).

    Returns:
        True if the file was written, False if it was already current.

    Raises:
        ToolError: If the path exists but is not a regular file.
    """
    path = docs / "versions.json"
    text = json.dumps(content, indent=2) + "\n"
    if path.is_symlink() or (path.exists() and not path.is_file()):
        raise ToolError(f"{path} exists but is not a regular file")
    if path.is_file() and path.read_text(encoding="utf-8") == text:
        return False
    path.write_text(text, encoding="utf-8")
    return True


def _update_stable(docs: Path, target: str) -> bool:
    """Point ``<docs>/stable`` at ``target`` with a relative symlink.

    A plain directory or file at ``stable`` is removed first; an existing
    symlink with the same target is left alone.

    Args:
        docs: The ``docs/`` directory.
        target: The version directory name, e.g. ``v2.1.0``.

    Returns:
        True if the link was created or re-pointed, False if unchanged.
    """
    link = docs / "stable"
    if link.is_symlink():
        if str(link.readlink()) == target:
            return False
        link.unlink()
    elif link.is_dir():
        shutil.rmtree(link)
    elif link.exists():
        link.unlink()
    link.symlink_to(target, target_is_directory=True)
    return True


def write_versions(docs: Path) -> VersionsResult:
    """Rewrite ``versions.json`` and re-point ``stable`` for a release tree.

    Nothing is written when the content is already current.

    Args:
        docs: The ``docs/`` directory holding the version directories.

    Returns:
        What was changed and which version is ``stable``.

    Raises:
        UsageError: If ``docs`` is not a directory or holds no version.
        ToolError: If ``docs/versions.json`` exists but is not a regular
            file; nothing is written then.
    """
    content = entries(docs)
    stable = str(content[0]["version"])
    json_changed = _write_json(docs, content)
    link_changed = _update_stable(docs, stable)
    return VersionsResult(stable, json_changed, link_changed)


def write_dev_versions(docs: Path, version: str) -> VersionsResult:
    """Write the ``versions.json`` of a development build.

    No ``stable`` link is created or touched.

    Args:
        docs: The ``docs/`` directory of the build.
        version: The directory name of the build, e.g. ``dev``.

    Returns:
        What was changed; ``stable`` is None.

    Raises:
        UsageError: If ``docs`` is not a directory.
        ToolError: If ``docs/versions.json`` exists but is not a regular
            file.
    """
    if not docs.is_dir():
        raise UsageError(f"{docs} is not a directory")
    return VersionsResult(None, _write_json(docs, dev_entries(version)), False)
