# SPDX-License-Identifier: LGPL-3.0-or-later
"""Assemble the complete jupedsim.org website into one output directory.

The source is the ``jupedsim.org/`` folder of the checkout, which holds the
``www/`` and ``docs/`` Sphinx projects and their shared ``theme/``. The
output mirrors the Pages branch:

    <out>/                      www/ project (landing page, articles, notes)
    <out>/docs/<version>/       docs/ project
    <out>/docs/stable           symlink to the newest docs version (a
                                directory when <version> is "stable")
    <out>/docs/versions.json    version switcher data (only <version> itself
                                when it is not a release name like v2.0.0,
                                e.g. "stable" for PR previews)
    <out>/pagefind/             search index over www and docs/<version>
                                (served as docs/stable)

Build state (Sphinx doctrees, executed notebooks) is kept in ``<out>-cache``
next to it, so the output directory holds only what gets published.

Each Sphinx project is built in its own ``python -m sphinx`` process: the
two ``conf.py`` files mutate ``sys.path`` and carry myst-nb and autoapi
state, so they do not share an interpreter. The docs build imports
``jupedsim``; run this with a ``PYTHONPATH`` that points at the build's
bindings (the CMake ``website`` targets do that through the generated
``build-website`` wrapper).
"""

from __future__ import annotations

import importlib.util
import shutil
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Literal

from jps_website_tooling import pagefind, versions
from jps_website_tooling.errors import ToolError, UsageError
from jps_website_tooling.out import (
    running_in_github_action,
    write_info,
    write_warning,
)

Target = Literal["www", "docs", "all"]


@dataclass(frozen=True)
class BuildOptions:
    """What to build and where.

    Attributes:
        source: The ``jupedsim.org/`` folder (``www/``, ``docs/``, ``theme/``).
        out: Output directory, the local replica of the Pages tree.
        version: Docs version directory name, e.g. ``v2.0.0`` or ``dev``;
            unused (may be empty) when ``target`` is ``www``.
        notebooks: Execute the docs' notebooks (``False`` sets
            ``nb_execution_mode=off``); www never executes notebooks.
        strict: Turn Sphinx warnings into errors (``-W --keep-going``);
            the default, ``--lenient`` turns it off.
        target: What to build: the www project, the docs, or everything
            (``all``, which also builds the Pagefind index).
        clean: Remove ``out`` and ``<out>-cache`` first.
    """

    source: Path
    out: Path
    version: str = ""
    notebooks: bool = True
    strict: bool = True
    target: Target = "all"
    clean: bool = False


def sphinx_command(
    source: Path,
    out: Path,
    doctrees: Path,
    *,
    notebooks: bool,
    parallel: bool,
    strict: bool,
) -> list[str]:
    """Build the ``python -m sphinx`` command line for one project.

    Args:
        source: The project directory holding ``conf.py``.
        out: HTML output directory.
        doctrees: Where Sphinx keeps its doctrees (outside ``out``).
        notebooks: Execute notebooks; ``False`` adds ``nb_execution_mode=off``.
        parallel: Add ``-j auto``.
        strict: Add ``-W --keep-going`` (report every warning, then fail).

    Returns:
        The command.
    """
    cmd = [sys.executable, "-m", "sphinx", "-T"]
    if parallel:
        cmd += ["-j", "auto"]
    if strict:
        cmd += ["-W", "--keep-going"]
    cmd += ["-b", "html", "-d", str(doctrees), str(source), str(out)]
    if not notebooks:
        cmd += ["-D", "nb_execution_mode=off"]
    return cmd


def run_sphinx(
    source: Path,
    out: Path,
    doctrees: Path,
    *,
    notebooks: bool,
    parallel: bool,
    strict: bool,
    what: str,
) -> None:
    """Run one Sphinx build in its own process.

    Sphinx's output goes straight to this process's stdout and stderr.

    Args:
        source: The project directory holding ``conf.py``.
        out: HTML output directory.
        doctrees: Where Sphinx keeps its doctrees.
        notebooks: Execute notebooks.
        parallel: Build with ``-j auto``.
        strict: Warnings are errors.
        what: Name of the build for the error message.

    Raises:
        ToolError: If Sphinx exits non-zero.
    """
    cmd = sphinx_command(
        source,
        out,
        doctrees,
        notebooks=notebooks,
        parallel=parallel,
        strict=strict,
    )
    result = subprocess.run(cmd)
    if result.returncode != 0:
        raise ToolError(f"{what} failed with exit code {result.returncode}")


def pagefind_available() -> bool:
    """Whether the ``pagefind`` package and its binary are importable.

    Returns:
        True if both ``pagefind`` and ``pagefind_bin`` can be found.
    """
    return all(
        importlib.util.find_spec(name) is not None
        for name in ("pagefind", "pagefind_bin")
    )


def _validate(options: BuildOptions) -> tuple[Path, Path]:
    """Check the source folder and the version.

    Args:
        options: The build options.

    Returns:
        The ``www/`` and ``docs/`` project directories.

    Raises:
        UsageError: If the source is not the ``jupedsim.org`` folder or the
            version is empty while docs are built.
    """
    www_dir = options.source / "www"
    docs_dir = options.source / "docs"
    if (
        not (www_dir / "conf.py").is_file()
        or not (docs_dir / "conf.py").is_file()
    ):
        raise UsageError(
            f"{options.source} is not the jupedsim.org folder "
            "(needs www/conf.py and docs/conf.py)"
        )
    if options.target != "www" and not options.version:
        raise UsageError("the docs version must not be empty")
    return www_dir, docs_dir


def _build_docs(
    docs_dir: Path, out: Path, cache: Path, options: BuildOptions
) -> None:
    """Build the docs project into ``<out>/docs/<version>`` and its metadata.

    myst-nb always writes executed notebooks to ``<outdir>/../jupyter_execute``,
    i.e. into ``<out>/docs/``. That directory is parked in the cache between
    builds, so incremental builds still find the images the cached doctrees
    point to, and the served tree never keeps it, also when the build fails.

    Args:
        docs_dir: The ``docs/`` project.
        out: The output root.
        cache: The ``<out>-cache`` directory.
        options: The build options.
    """
    docs_out = out / "docs" / options.version
    shutil.rmtree(docs_out / ".doctrees", ignore_errors=True)
    nb_out = out / "docs" / "jupyter_execute"
    nb_cache = cache / "jupyter_execute-docs"
    if nb_cache.exists() and not nb_out.exists():
        nb_out.parent.mkdir(parents=True, exist_ok=True)
        nb_cache.rename(nb_out)
    try:
        run_sphinx(
            docs_dir,
            docs_out,
            cache / "doctrees-docs",
            notebooks=options.notebooks,
            parallel=True,
            strict=options.strict,
            what="docs build (docs/)",
        )
    finally:
        if nb_out.exists():
            shutil.rmtree(nb_cache, ignore_errors=True)
            nb_cache.parent.mkdir(parents=True, exist_ok=True)
            nb_out.rename(nb_cache)
    if versions.VERSION_DIR.match(options.version):
        versions.write_versions(out / "docs")
    else:
        # A development build (e.g. CI's "dev") lists just itself.
        versions.write_dev_versions(out / "docs", options.version)


def build_website(options: BuildOptions) -> Path:
    """Assemble the website.

    Args:
        options: What to build and where.

    Returns:
        The output directory.

    Raises:
        UsageError: If the source or the version is unusable.
        ToolError: If a Sphinx build or the index build fails.
    """
    www_dir, docs_dir = _validate(options)
    out = options.out.resolve()
    cache = out.parent / f"{out.name}-cache"
    if options.clean:
        for path in (out, cache):
            if path.exists():
                shutil.rmtree(path)
    out.mkdir(parents=True, exist_ok=True)

    if options.target in ("www", "all"):
        shutil.rmtree(out / ".doctrees", ignore_errors=True)
        # www is not tied to a jupedsim version: its notebooks keep the
        # outputs they were committed with and are never executed.
        run_sphinx(
            www_dir,
            out,
            cache / "doctrees-www",
            notebooks=False,
            parallel=False,
            strict=options.strict,
            what="www build (www/)",
        )
    if options.target in ("docs", "all"):
        _build_docs(docs_dir, out, cache, options)
    if options.target == "all":
        if pagefind_available():
            pagefind.build_index(out, docs=out / "docs" / options.version)
        else:
            write_warning(
                "pagefind is not installed (pip install 'pagefind[bin]~=1.5'); "
                "skipping the search index, the search box falls back to "
                "Sphinx search"
            )
    if not running_in_github_action():
        write_info(f"serve with: python -m http.server -b localhost -d {out}")
    return out
