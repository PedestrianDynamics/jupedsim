#!/usr/bin/env python3
# SPDX-License-Identifier: LGPL-3.0-or-later
"""Move a built website under a URL prefix, for PR previews.

The site and docs link to each other with root-absolute URLs ("/docs/stable/",
"/articles/", "/pagefind/pagefind.js", "/docs/versions.json"), which is right
for jupedsim.org and next-doc.jupedsim.org, both served from a domain root.
PR previews live under /pull-requests/<N>/ instead, so deploy-previews.yml
runs this script on the preview tree before publishing it:

- In every *.html file, root-absolute values of href, src, action,
  data-pagefind and data-json attributes get the prefix. Protocol-relative
  ("//host") and absolute ("https://") URLs, relative links and fragments are
  left alone, and values that already carry the prefix are not changed again,
  so the script is idempotent.
- docs/versions.json: root-absolute "url" values get the prefix.
- --stable NAME: if docs/NAME exists and docs/stable does not, docs/stable
  becomes a symlink to NAME (PR builds name their docs docs/dev/; the site
  links to docs/stable/).

Usage: rebase_preview_links.py <tree> <prefix> [--stable NAME]
The prefix must start and end with "/", e.g. /pull-requests/42/.
"""

import argparse
import json
import re
import sys
from pathlib import Path

ATTRS = ("href", "src", "action", "data-pagefind", "data-json")


def attr_pattern(prefix: str) -> re.Pattern:
    rest = re.escape(prefix[1:])
    names = "|".join(re.escape(a) for a in ATTRS)
    # An attribute (preceded by whitespace) whose quoted value starts with a
    # single "/" that is not already followed by the prefix.
    return re.compile(rf"(\s(?:{names})=)([\"'])/(?!/|{rest})")


def rebase_html(path: Path, pattern: re.Pattern, prefix: str) -> int:
    text = path.read_text(encoding="utf-8")
    new, n = pattern.subn(lambda m: f"{m.group(1)}{m.group(2)}{prefix}", text)
    if n:
        path.write_text(new, encoding="utf-8")
    return n


def rebase_versions(path: Path, prefix: str) -> int:
    entries = json.loads(path.read_text(encoding="utf-8"))
    n = 0
    for entry in entries:
        url = entry.get("url") if isinstance(entry, dict) else None
        if (
            isinstance(url, str)
            and url.startswith("/")
            and not url.startswith("//")
            and not url.startswith(prefix)
        ):
            entry["url"] = prefix + url[1:]
            n += 1
    if n:
        path.write_text(json.dumps(entries, indent=2) + "\n", encoding="utf-8")
    return n


def main() -> int:
    parser = argparse.ArgumentParser(
        description=__doc__.splitlines()[0],
        epilog="See the module docstring for the rules.",
    )
    parser.add_argument("tree", type=Path, help="built site (site root)")
    parser.add_argument("prefix", help="URL prefix, e.g. /pull-requests/42/")
    parser.add_argument(
        "--stable",
        metavar="NAME",
        help="link docs/stable to docs/NAME when docs/stable is missing",
    )
    args = parser.parse_args()
    prefix = args.prefix
    if not (
        prefix.startswith("/") and prefix.endswith("/") and len(prefix) > 1
    ):
        sys.exit(f"prefix must start and end with '/': {prefix!r}")
    if not (args.tree / "index.html").is_file():
        sys.exit(f"{args.tree} is not a site build (no index.html)")

    pattern = attr_pattern(prefix)
    files = links = 0
    for path in sorted(args.tree.rglob("*.html")):
        if path.is_symlink():
            continue
        n = rebase_html(path, pattern, prefix)
        files += bool(n)
        links += n
    versions = args.tree / "docs" / "versions.json"
    urls = rebase_versions(versions, prefix) if versions.is_file() else 0

    stable = args.tree / "docs" / "stable"
    linked = False
    if (
        args.stable
        and (args.tree / "docs" / args.stable).is_dir()
        and not stable.exists()
        and not stable.is_symlink()
    ):
        stable.symlink_to(args.stable)
        linked = True

    print(
        f"rebase_preview_links: {links} links in {files} HTML files, "
        f"{urls} versions.json urls -> {prefix}"
        + (f"; docs/stable -> {args.stable}" if linked else "")
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
