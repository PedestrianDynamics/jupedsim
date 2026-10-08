#!/usr/bin/env python3
# SPDX-License-Identifier: LGPL-3.0-or-later
"""Assemble the complete jupedsim.org website into one output directory.

Layout of the output directory (mirrors the gh-pages branch):

    <out>/                      site/ project (landing page, articles, notes)
    <out>/docs/<version>/       docs/source project
    <out>/docs/stable           symlink to the newest docs version
    <out>/docs/versions.json    version switcher data
                                (only <version> itself when it is not a
                                release name like v2.0.0, e.g. "dev")
    <out>/pagefind/             search index over site and docs/stable

Build state (Sphinx doctrees, executed notebooks) is kept in ``<out>-cache``
next to it, so the output directory holds only what gets published.

The docs build imports ``jupedsim``; run this script with a PYTHONPATH that
points at the build's bindings (the CMake ``site`` / ``site-fast`` targets do
that through the generated ``build-site`` wrapper).
"""

import argparse
import json
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

# Version directory names write_docs_versions.py publishes.
RELEASE_VERSION = re.compile(r"^v\d+\.\d+\.\d+$")


def fail(message: str) -> None:
    sys.exit(f"build_site.py: error: {message}")


def run(cmd: list[str], env: dict[str, str], what: str) -> None:
    print("+ " + " ".join(cmd), flush=True)
    result = subprocess.run(cmd, env=env)
    if result.returncode != 0:
        fail(f"{what} failed with exit code {result.returncode}")


def sphinx_cmd(
    source: Path,
    out: Path,
    doctrees: Path,
    no_notebooks: bool,
    parallel: bool,
    strict: bool,
) -> list[str]:
    cmd = [sys.executable, "-m", "sphinx", "-T"]
    if parallel:
        cmd += ["-j", "auto"]
    if strict:
        # Report every warning of the build, then fail.
        cmd += ["-W", "--keep-going"]
    cmd += ["-b", "html", "-d", str(doctrees), str(source), str(out)]
    if no_notebooks:
        cmd += ["-D", "nb_execution_mode=off"]
    return cmd


def has_pagefind(env: dict[str, str]) -> bool:
    result = subprocess.run(
        [sys.executable, "-m", "pagefind", "--help"],
        env=env,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )
    return result.returncode == 0


def main() -> None:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument(
        "--source",
        required=True,
        type=Path,
        help="jupedsim source checkout",
    )
    parser.add_argument(
        "--out",
        required=True,
        type=Path,
        help="output directory (the local gh-pages replica)",
    )
    parser.add_argument(
        "--version",
        required=True,
        help="docs version directory name, e.g. v2.0.0 or dev",
    )
    parser.add_argument(
        "--no-notebooks",
        action="store_true",
        help="do not execute notebooks (nb_execution_mode=off)",
    )
    parser.add_argument(
        "--strict",
        action="store_true",
        help="turn Sphinx warnings into errors in the site and the docs "
        "build (-W --keep-going: report all warnings, then fail); "
        "used by CI",
    )
    parser.add_argument(
        "--only",
        choices=["site", "docs", "all"],
        default="all",
        help="build only the site, only the docs, or everything "
        "(default: all; docs also writes versions.json, "
        "all also builds the pagefind index)",
    )
    parser.add_argument(
        "--clean",
        action="store_true",
        help="remove the output directory and <out>-cache before building",
    )
    args = parser.parse_args()

    source = args.source.resolve()
    out = args.out.resolve()
    build_site = args.only in ("site", "all")
    build_docs = args.only in ("docs", "all")

    site_dir = source / "site"
    docs_dir = source / "docs" / "source"
    versions_script = source / "scripts" / "ci" / "write_docs_versions.py"
    pagefind_script = source / "scripts" / "ci" / "build_pagefind.sh"

    missing = []
    if build_site and not (site_dir / "conf.py").is_file():
        missing.append(f"site/ does not exist yet ({site_dir}/conf.py)")
    if build_docs and not (docs_dir / "conf.py").is_file():
        missing.append(f"{docs_dir}/conf.py does not exist")
    if build_docs and not versions_script.is_file():
        missing.append(f"{versions_script} does not exist yet")
    if args.only == "all" and not pagefind_script.is_file():
        missing.append(f"{pagefind_script} does not exist yet")
    if missing:
        fail("missing inputs:\n  " + "\n  ".join(missing))

    # Subprocesses (sphinx, build_pagefind.sh calling `python`) must use the
    # same interpreter as this script.
    env = dict(os.environ)
    env["PATH"] = (
        str(Path(sys.executable).parent) + os.pathsep + env.get("PATH", "")
    )

    # Build state that must not be published (Sphinx doctrees, myst-nb's
    # executed notebooks) lives next to the output, not inside it.
    cache = out.parent / f"{out.name}-cache"
    if args.clean:
        for path in (out, cache):
            if path.exists():
                shutil.rmtree(path)
    out.mkdir(parents=True, exist_ok=True)

    if build_site:
        shutil.rmtree(out / ".doctrees", ignore_errors=True)
        run(
            sphinx_cmd(
                site_dir,
                out,
                cache / "doctrees-site",
                args.no_notebooks,
                parallel=False,
                strict=args.strict,
            ),
            env,
            "site build (site/)",
        )
    if build_docs:
        docs_out = out / "docs" / args.version
        shutil.rmtree(docs_out / ".doctrees", ignore_errors=True)
        # myst-nb always writes to <outdir>/../jupyter_execute, i.e. into
        # <out>/docs/. Park it in the cache between builds so incremental
        # builds still find the images the cached doctrees point to.
        nb_out = out / "docs" / "jupyter_execute"
        nb_cache = cache / "jupyter_execute-docs"
        if nb_cache.exists() and not nb_out.exists():
            nb_out.parent.mkdir(parents=True, exist_ok=True)
            nb_cache.rename(nb_out)
        try:
            run(
                sphinx_cmd(
                    docs_dir,
                    docs_out,
                    cache / "doctrees-docs",
                    args.no_notebooks,
                    parallel=True,
                    strict=args.strict,
                ),
                env,
                "docs build (docs/source/)",
            )
        finally:
            # Also on a failed build (run() exits), so the served tree
            # never keeps jupyter_execute.
            if nb_out.exists():
                shutil.rmtree(nb_cache, ignore_errors=True)
                nb_cache.parent.mkdir(parents=True, exist_ok=True)
                nb_out.rename(nb_cache)
        if RELEASE_VERSION.match(args.version):
            run(
                [sys.executable, str(versions_script), str(out / "docs")],
                env,
                "write_docs_versions.py",
            )
        else:
            # write_docs_versions.py only knows release directories; a
            # development build (e.g. CI's "dev") lists just itself.
            entries = [
                dict(
                    name=args.version,
                    version=args.version,
                    url=f"/docs/{args.version}/",
                    preferred=True,
                )
            ]
            (out / "docs" / "versions.json").write_text(
                json.dumps(entries, indent=2) + "\n", encoding="utf-8"
            )
    if args.only == "all":
        if has_pagefind(env):
            run(
                ["bash", str(pagefind_script), str(out)],
                env,
                "build_pagefind.sh",
            )
        else:
            print(
                "build_site.py: warning: pagefind is not installed "
                "(pip install 'pagefind[bin]~=1.5'); "
                "skipping the search index, the search box falls back to "
                "Sphinx search",
                file=sys.stderr,
            )

    print(f"serve with:\npython -m http.server -b localhost -d {out}")


if __name__ == "__main__":
    main()
