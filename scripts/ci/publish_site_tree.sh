#!/usr/bin/env bash
# SPDX-License-Identifier: LGPL-3.0-or-later
#
# publish_site_tree.sh <build-out> <pages-root>
#
# Publishes an assembled website (scripts/build_site.py output: site at the
# root, docs under docs/<version>/) into a Pages checkout whose root is the
# site root. Used by .github/workflows/deploy-next-doc.yml for the pre-go-live
# test environment (next-doc.jupedsim.org); git is left to the caller.
#
# - The site and the docs are replaced; stale pages and docs versions are
#   deleted.
# - CNAME, .nojekyll and docs/v1x/ (the 1.x archive, copied there by hand)
#   are never touched.
# - robots.txt keeps the test copy out of search engines.
# - docs/versions.json and docs/stable are rewritten by write_docs_versions.py
#   (with the "1.x (archive)" entry when docs/v1x holds a version), then the
#   Pagefind index is rebuilt over the published tree (never over docs/v1x).
#
# Exit codes: 0 published, 2 unusable build output (nothing touched).
set -euo pipefail

if [ "$#" -ne 2 ]; then
    echo "usage: $0 <build-out> <pages-root>" >&2
    exit 2
fi
build="$1"
pages="$2"
here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

if [ ! -f "$build/index.html" ] || [ ! -d "$build/docs" ]; then
    echo "publish_site_tree.sh: $build is not a site build (needs index.html and docs/); nothing published" >&2
    exit 2
fi
if [ ! -d "$pages" ]; then
    echo "publish_site_tree.sh: $pages does not exist" >&2
    exit 2
fi

rsync -a --delete \
    --exclude=/.git \
    --exclude=/CNAME \
    --exclude=/.nojekyll \
    --exclude=/robots.txt \
    --exclude=/docs/v1x \
    "$build/" "$pages/"

printf 'User-agent: *\nDisallow: /\n' >"$pages/robots.txt"

if ! compgen -G "$pages/docs/v1x/v[0-9]*" >/dev/null; then
    echo "::warning::1.x archive not copied yet ($pages/docs/v1x has no version directory): [DOCS V1] links will 404 and the version switcher has no archive entry"
fi

python3 "$here/write_docs_versions.py" "$pages/docs"
bash "$here/build_pagefind.sh" "$pages"
