#!/usr/bin/env python3
"""
Scan project source files and CGAL headers to find the minimal set of Boost
modules and headers that are transitively needed.

Usage (run from the repo root):
    python3 scripts/scan_boost_deps.py \\
        --cgal    third-party/cgal \\
        --boost   third-party/boost-1.88.0/include \\
        --src     libsimulator libcommon

The scanner starts from the given source directories, follows all CGAL includes
it finds, and collects every boost/ header included along the way.

Output:
  - A sorted list of all boost/ headers found
  - The top-level module/file names (suitable for use with extract_boost.sh)
  - A grep summary of ALL boost/ includes across all CGAL headers (fast,
    conservative alternative — use this when the BFS is too slow)
"""

import argparse
import re
import subprocess
import sys
from pathlib import Path

_BOOST_RE = re.compile(r'#\s*include\s*[<"]boost/([^>"]+)[>"]')
_CGAL_RE = re.compile(r'#\s*include\s*[<"]CGAL/([^>"]+)[>"]')


def _read(path: Path) -> str:
    try:
        return path.read_text(errors="replace")
    except OSError:
        return ""


def bfs_scan(
    src_dirs: list[Path],
    cgal_dir: Path,
    boost_include_dir: Path,
) -> tuple[set[str], set[str]]:
    """BFS over source → CGAL → Boost includes.

    Returns (visited_boost_headers, visited_files) where visited_boost_headers
    is a set of paths relative to the boost/ directory (e.g. "config.hpp",
    "mpl/bool.hpp").
    """
    boost_dir = boost_include_dir / "boost"

    # Build a quick lookup: CGAL/<name> → list of absolute paths in cgal_dir
    cgal_index: dict[str, list[Path]] = {}
    for p in cgal_dir.rglob("*.h"):
        parts = p.parts
        for i, part in enumerate(parts):
            if part == "CGAL" and i + 1 < len(parts):
                key = "/".join(parts[i + 1 :])
                cgal_index.setdefault(key, []).append(p)
    for p in cgal_dir.rglob("*.hpp"):
        parts = p.parts
        for i, part in enumerate(parts):
            if part == "CGAL" and i + 1 < len(parts):
                key = "/".join(parts[i + 1 :])
                cgal_index.setdefault(key, []).append(p)

    visited_files: set[str] = set()
    visited_boost: set[str] = set()
    queue: list[Path] = []

    for src in src_dirs:
        for ext in ("*.cpp", "*.hpp", "*.h"):
            queue.extend(src.rglob(ext))

    def process(path: Path) -> None:
        key = str(path)
        if key in visited_files:
            return
        visited_files.add(key)

        content = _read(path)

        for m in _BOOST_RE.finditer(content):
            rel = m.group(1)
            if rel not in visited_boost:
                visited_boost.add(rel)
                full = boost_dir / rel
                if full.exists():
                    process(full)

        for m in _CGAL_RE.finditer(content):
            for cgal_path in cgal_index.get(m.group(1), []):
                process(cgal_path)

    while queue:
        process(queue.pop())

    return visited_boost, visited_files


def grep_scan(cgal_dir: Path) -> set[str]:
    """Fast alternative: grep all CGAL headers for boost/ includes.

    This is conservative (includes everything CGAL can ever use) rather than
    minimal (only what this project actually pulls in).  Use it as a sanity
    check or when the BFS is too slow.
    """
    result = subprocess.run(
        ["grep", "-rh", r"#include.*<boost/", str(cgal_dir)],
        capture_output=True,
        text=True,
    )
    modules: set[str] = set()
    for line in result.stdout.splitlines():
        m = re.search(r"<boost/([^/\">]+)", line)
        if m:
            modules.add(m.group(1))
    return modules


def top_level_modules(headers: set[str]) -> set[str]:
    modules: set[str] = set()
    for h in headers:
        modules.add(h.split("/")[0])
    return modules


def main() -> None:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument(
        "--cgal", required=True, type=Path, help="Path to third-party/cgal"
    )
    parser.add_argument(
        "--boost",
        required=True,
        type=Path,
        help="Path to boost include dir (contains boost/)",
    )
    parser.add_argument(
        "--src",
        nargs="+",
        required=True,
        type=Path,
        help="Source directories to scan",
    )
    parser.add_argument(
        "--grep-only",
        action="store_true",
        help="Skip BFS, only run the fast grep scan",
    )
    args = parser.parse_args()

    if not args.grep_only:
        print(
            "Running BFS scan (may be slow on large CGAL trees)…",
            file=sys.stderr,
        )
        boost_headers, _ = bfs_scan(args.src, args.cgal, args.boost)

        modules = top_level_modules(boost_headers)
        print(f"\n=== BFS: Boost headers needed ({len(boost_headers)}) ===")
        for h in sorted(boost_headers):
            print(f"  boost/{h}")

        print(f"\n=== BFS: Top-level modules/files ({len(modules)}) ===")
        for m in sorted(modules):
            print(f"  {m}")

    print("\n=== Grep: Top-level modules referenced anywhere in CGAL ===")
    grep_modules = grep_scan(args.cgal)
    for m in sorted(grep_modules):
        print(f"  {m}")


if __name__ == "__main__":
    main()
