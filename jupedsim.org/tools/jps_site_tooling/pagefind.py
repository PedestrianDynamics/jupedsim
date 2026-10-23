# SPDX-License-Identifier: LGPL-3.0-or-later
"""The Pagefind search index of a Pages-shaped tree.

Indexes the site at ``<root>`` plus ``<root>/docs/stable`` when that is a
symlink to a docs build. Everything else under ``docs/`` (other versions,
``docs/v1x``, a pre-2.0 ``stable`` stub directory) and the previous index
are never read. Pages without ``data-pagefind-body`` are skipped by Pagefind
itself.

The pages to index are staged into a temporary directory (so Pagefind sees
exactly that set and ``docs/stable`` as a real directory), the index is built
next to it and swapped in as ``<root>/pagefind/`` only on success; a failed
run keeps the previous index. The index carries ``build-stamp.txt`` (this
package's version and the Pagefind version) so a publisher can tell which
inputs built it.

Pagefind runs through the ``pagefind`` package's API (``PagefindIndex``),
which drives the bundled binary in service mode; no command line is
assembled here. The ``pagefind`` imports are local so that the other
subcommands work in environments without it.
"""

from __future__ import annotations

import asyncio
import importlib.metadata
import shutil
import tempfile
from pathlib import Path
from typing import TYPE_CHECKING

from jps_site_tooling import __version__
from jps_site_tooling.errors import ToolError, UsageError
from jps_site_tooling.out import write_info

if TYPE_CHECKING:
    from pagefind.index import IndexConfig

ROOT_EXCLUDES = (".git", "docs", "pagefind")
# Pagefind configuration (IndexConfig keys): one English index; heading
# permalinks and [source] links are left out of the excerpts.
INDEX_CONFIG: IndexConfig = {
    "force_language": "en",
    "exclude_selectors": ["a.headerlink", ".viewcode-link"],
}


def stamp() -> str:
    """Describe what builds the index.

    Returns:
        Two lines, ``jps_site_tooling <version>`` and ``pagefind <version>``,
        each newline-terminated; the content of ``build-stamp.txt``, which
        records what built the live index.

    Raises:
        ToolError: If the ``pagefind_bin`` distribution is not installed.
    """
    try:
        pagefind_version = importlib.metadata.version("pagefind_bin")
    except importlib.metadata.PackageNotFoundError as exc:
        raise ToolError(
            "pagefind_bin is not installed (pip install 'pagefind[bin]~=1.5')"
        ) from exc
    return f"jps_site_tooling {__version__}\npagefind {pagefind_version}\n"


def stage(root: Path, into: Path) -> None:
    """Copy the pages to index into a fresh directory.

    Args:
        root: The Pages-shaped tree.
        into: The staging directory to create.

    Raises:
        ToolError: If ``docs/stable`` is a dangling symlink (the tree is
            broken; indexing without the docs would hide that).
    """

    def skip(directory: str, names: list[str]) -> set[str]:
        """Drop the excluded entries, at the root only (not nested ones)."""
        if Path(directory) != root:
            return set()
        return {name for name in names if name in ROOT_EXCLUDES}

    shutil.copytree(root, into, symlinks=True, ignore=skip)
    stable = root / "docs" / "stable"
    if stable.is_symlink():
        if not stable.is_dir():
            raise ToolError(
                f"{stable} is a dangling symlink (-> {stable.readlink()})"
            )
        shutil.copytree(stable, into / "docs" / "stable", symlinks=False)


def _index_directory(site: Path, output: Path, config: IndexConfig) -> int:
    """Index one directory with Pagefind's API and write the index.

    Args:
        site: The staged pages.
        output: Where the index files go.
        config: ``IndexConfig`` entries.

    Returns:
        The number of pages Pagefind indexed.
    """
    # Imported here, not at module level: the other subcommands must work
    # in environments without Pagefind.
    from pagefind.index import PagefindIndex

    # The context manager writes the index once on exit, to the configured
    # output path; an explicit write_files() would add a second copy under
    # the default ./pagefind of the current directory.
    run_config: IndexConfig = {**config, "output_path": str(output)}

    async def run() -> int:
        """Drive the service: add the directory; exit writes the files."""
        async with PagefindIndex(config=run_config) as index:
            indexed = await index.add_directory(str(site))
        return int(indexed["page_count"])

    return asyncio.run(run())


def run_pagefind(site: Path, output: Path) -> int:
    """Run Pagefind over a staged tree.

    Args:
        site: The staged pages.
        output: Where the index is written.

    Returns:
        The number of pages indexed.

    Raises:
        ToolError: If Pagefind is not installed, fails, or writes no
            ``pagefind.js``.
    """
    try:
        count = _index_directory(site, output, INDEX_CONFIG)
    except ImportError as exc:
        raise ToolError(
            "pagefind is not installed (pip install 'pagefind[bin]~=1.5')"
        ) from exc
    except Exception as exc:  # the service surfaces assorted error types
        raise ToolError(f"pagefind failed: {exc}") from exc
    if not (output / "pagefind.js").is_file():
        raise ToolError(f"pagefind wrote no pagefind.js into {output}")
    return count


def _replace_index(index: Path, built: Path) -> None:
    """Put the freshly built index in place of the previous one.

    Args:
        index: ``<root>/pagefind``; a directory, a symlink or absent.
        built: The new index directory to move there.

    Raises:
        ToolError: If the previous index cannot be removed; nothing is moved
            then, so the previous index is never nested or overwritten.
    """
    try:
        if index.is_symlink() or index.is_file():
            index.unlink()
        elif index.is_dir():
            shutil.rmtree(index)
        if index.exists() or index.is_symlink():
            raise ToolError(f"could not remove the previous index {index}")
        shutil.move(str(built), str(index))
    except OSError as exc:
        raise ToolError(
            f"could not replace the previous index {index}: {exc}"
        ) from exc


def build_index(root: Path) -> Path:
    """Rebuild ``<root>/pagefind`` over the site and ``docs/stable``.

    Args:
        root: The Pages-shaped tree (a Pages checkout or a site build).

    Returns:
        The index directory, ``<root>/pagefind``.

    Raises:
        UsageError: If ``root`` is not a directory.
        ToolError: If staging, the Pagefind run or the swap fails; the
            previous index is kept.
    """
    if not root.is_dir():
        raise UsageError(f"{root} is not a directory")
    index = root / "pagefind"
    with tempfile.TemporaryDirectory(prefix="jps-pagefind-") as tmp:
        staged = Path(tmp) / "site"
        built = Path(tmp) / "pagefind"
        try:
            stage(root, staged)
        except (OSError, shutil.Error) as exc:
            raise ToolError(
                f"could not stage {root} for indexing: {exc}"
            ) from exc
        count = run_pagefind(staged, built)
        (built / "build-stamp.txt").write_text(stamp(), encoding="utf-8")
        _replace_index(index, built)
    write_info(f"pagefind: indexed {count} pages into {index}")
    return index
