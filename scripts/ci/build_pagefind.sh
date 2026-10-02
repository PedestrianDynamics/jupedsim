#!/usr/bin/env bash
# SPDX-License-Identifier: LGPL-3.0-or-later
#
# Build the Pagefind search index for a gh-pages-shaped tree.
#
# Usage: build_pagefind.sh <root>
#        build_pagefind.sh --stamp
#
# Indexes the site at <root> plus <root>/docs/stable when that is a symlink to
# a 2.x docs build. A docs/stable redirect-stub directory (before 2.0.0),
# docs/v1x and the old top-level 1.x dirs are never indexed. Pages without
# data-pagefind-body (stubs, search, genindex, _modules) are skipped by
# Pagefind itself. Writes <root>/pagefind/ (replacing any earlier index).
#
# The index carries pagefind/build-stamp.txt: the sha256 of this script and
# the Pagefind version. --stamp prints what a build would write there now, so
# a publisher can tell whether an existing index was built by other inputs.
set -euo pipefail

if [[ $# -ne 1 || ($1 != --stamp && ! -d "$1") ]]; then
    echo "usage: $0 <root> | --stamp  (root must be an existing directory)" >&2
    exit 2
fi

python=python
if ! "$python" -m pagefind --version >/dev/null 2>&1; then
    echo "error: '$python -m pagefind' is not available;" \
        "install it with: python -m pip install 'pagefind[bin]~=1.5'" >&2
    exit 1
fi

stamp() {
    printf 'build_pagefind.sh sha256:%s\n' "$("$python" -c '
import hashlib, sys
print(hashlib.sha256(open(sys.argv[1], "rb").read()).hexdigest())
' "${BASH_SOURCE[0]}")"
    "$python" -m pagefind --version
}
if [[ $1 == --stamp ]]; then
    stamp
    exit 0
fi
root="${1%/}"

tmp="$(mktemp -d "${TMPDIR:-/tmp}/build_pagefind.XXXXXX")"
trap 'rm -rf "$tmp"' EXIT

rsync -a \
    --exclude '/.git' \
    --exclude '/docs' \
    --exclude '/pagefind' \
    --exclude '/stable' \
    --exclude '/v[0-9]*' \
    "$root/" "$tmp/site/"

if [[ -L "$root/docs/stable" ]]; then
    mkdir -p "$tmp/site/docs"
    cp -RL "$root/docs/stable" "$tmp/site/docs/stable"
fi

# Build next to the copy and swap it in only on success, so a failed or
# interrupted run keeps the previous index. Heading permalinks and [source]
# links are left out of the excerpts.
"$python" -m pagefind --site "$tmp/site" --output-path "$tmp/pagefind" \
    --force-language en \
    --exclude-selectors "a.headerlink" \
    --exclude-selectors ".viewcode-link"
test -f "$tmp/pagefind/pagefind.js"
stamp >"$tmp/pagefind/build-stamp.txt"
rm -rf "$root/pagefind"
mv "$tmp/pagefind" "$root/pagefind"
