#!/usr/bin/env bash
# SPDX-License-Identifier: LGPL-3.0-or-later
#
# check.sh — build the jupedsim_book fixture (site and docs mode) plus the
# real site/ and docs/ projects, then assert the theme's markup contract.
# Prints one ok/FAIL line per assertion and ends with CHECK PASSED (exit 0)
# or CHECK FAILED (exit 1).
#
#   bash docs/_theme/check.sh [--build-dir <path>] [--keep <dir>]
#
# --build-dir  build directory whose `environment` is sourced
#              (default: <worktree>/../<worktree basename>-build)
# --keep <dir> write outputs to <dir> (wiped first) and keep them;
#              default is a temp dir removed on exit. <dir> must be new,
#              empty or an earlier --keep dir, and not inside the checkout.
# (For the main clone `jupedsim/` the build dir is `build/`: pass --build-dir.)
set -euo pipefail

theme_dir="$(cd "$(dirname "$0")" && pwd)"
repo="$(cd "$theme_dir/../.." && pwd)"
fixture="$theme_dir/tests/fixture"

build_dir="$(dirname "$repo")/$(basename "$repo")-build"
keep=""
while [ $# -gt 0 ]; do
    case "$1" in
    --build-dir)
        build_dir="${2:?--build-dir needs a path}"
        shift 2
        ;;
    --keep)
        keep="${2:?--keep needs a directory}"
        shift 2
        ;;
    -h | --help)
        sed -n '4,17p' "$0"
        exit 0
        ;;
    *)
        echo "unknown argument: $1" >&2
        exit 2
        ;;
    esac
done

if [ ! -f "$build_dir/environment" ]; then
    echo "error: $build_dir/environment not found; configure the build dir or pass --build-dir" >&2
    exit 2
fi
# shellcheck disable=SC1091
source "$build_dir/environment"

marker=".jupedsim-book-check"
if [ -n "$keep" ]; then
    # Guard the wipe: only an earlier --keep dir (marker file) or an empty dir,
    # never /, $HOME, the checkout or anything inside it.
    real() { python -c 'import os, sys; print(os.path.realpath(sys.argv[1]))' "$1"; }
    k="$(real "$keep")"
    r="$(real "$repo")"
    case "$k/" in
    / | "$(real "$HOME")/" | "$r"/*)
        echo "error: refusing --keep $keep (resolves to $k)" >&2
        exit 2
        ;;
    esac
    case "$r/" in "$k"/*)
        echo "error: refusing --keep $keep (contains the checkout)" >&2
        exit 2
        ;;
    esac
    if [ -e "$k" ] && [ ! -f "$k/$marker" ] && [ -n "$(ls -A "$k" 2>/dev/null)" ]; then
        echo "error: refusing to wipe non-empty $k (not an earlier --keep dir)" >&2
        exit 2
    fi
    rm -rf "$k"
    mkdir -p "$k"
    touch "$k/$marker"
    out="$k"
else
    out="$(mktemp -d "${TMPDIR:-/tmp}/jupedsim-book-check.XXXXXX")"
    trap 'rm -rf "$out"' EXIT
fi

fail=0
pass=0

# build NAME SPHINX-ARGS... — run sphinx-build into $out/NAME;
# a non-zero exit is a failure, warnings print the log.
build() {
    local name="$1" log="$out/$1.log"
    shift
    echo "== build $name"
    if ! python -m sphinx "$@" "$out/$name" >"$log" 2>&1; then
        echo "FAIL: build $name failed (log: $log)"
        cat "$log"
        fail=$((fail + 1))
        return 0
    fi
    if grep -qE ': (WARNING|ERROR|CRITICAL):' "$log"; then
        echo "note: build $name emitted warnings:"
        grep -E ': (WARNING|ERROR|CRITICAL):' "$log"
    fi
    echo "ok:   build $name"
    pass=$((pass + 1))
}

build fixture-site -T -W --keep-going -E -b html "$fixture"
FIXTURE_MODE=docs build fixture-docs -T -W --keep-going -E -b html "$fixture"
# nav_section=docs: the docs tree (and its 404 rule) is rendered only on docs pages.
FIXTURE_MODE=site-no-notfound build fixture-plain -T -W --keep-going -E -b html \
    -D html_theme_options.logo_dark= -D html_theme_options.nav_section=docs "$fixture"
# bad-template.md (template: nope) is only part of this build; without -W,
# its one warning is asserted below.
FIXTURE_MODE=bad-template build fixture-badtpl -T --keep-going -E -b html "$fixture"
if [ -f "$repo/site/conf.py" ]; then
    build site -T -W --keep-going -E -b html "$repo/site"
    have_site=1
else
    echo "SKIP: build site (site/conf.py does not exist yet)"
    have_site=0
fi
build docs -T -W --keep-going -E -b html -D nb_execution_mode=off "$repo/docs/source"

# Assertions never abort the script (set -e is off from here on).
set +e

ok() {
    echo "ok:   $1"
    pass=$((pass + 1))
}
bad() {
    echo "FAIL: $1"
    fail=$((fail + 1))
}
# count FILE PATTERN(ERE) MIN LABEL
count() {
    local file="$1" pat="$2" min="$3" label="$4" n rc
    if [ ! -f "$file" ]; then
        bad "$label (missing $file)"
        return
    fi
    n=$(grep -oE -- "$pat" "$file" | wc -l | tr -d ' ')
    rc=$?  # pipefail: grep's status (1 = no match, 2 = error)
    if [ "$rc" -gt 1 ]; then
        bad "$label (grep error)"
        return
    fi
    if [ "$n" -lt "$min" ]; then bad "$label (found $n, want >= $min)"; else ok "$label ($n)"; fi
}
# none FILE PATTERN(ERE) LABEL — must not match in that one file
none() {
    local file="$1" pat="$2" label="$3" n rc
    if [ ! -f "$file" ]; then
        bad "$label (missing $file)"
        return
    fi
    n=$(grep -oE -- "$pat" "$file" | wc -l | tr -d ' ')
    rc=$?  # pipefail: grep's status (1 = no match, 2 = error)
    if [ "$rc" -gt 1 ]; then
        bad "$label (grep error)"
        return
    fi
    if [ "$n" -ne 0 ]; then bad "$label (found $n, want 0)"; else ok "$label"; fi
}
# absent DIR PATTERN(ERE) LABEL — must match no file below DIR
absent() {
    local dir="$1" pat="$2" label="$3" hits rc
    if [ ! -d "$dir" ]; then
        bad "$label (missing $dir)"
        return
    fi
    hits=$(grep -rEl -- "$pat" "$dir")
    rc=$?
    if [ "$rc" -gt 1 ]; then
        bad "$label (grep error)"
    elif [ -n "$hits" ]; then
        bad "$label:"
        echo "$hits" | sed 's/^/      /'
    else ok "$label"; fi
}
# exactly FILE PATTERN(ERE) N LABEL
exactly() {
    local file="$1" pat="$2" want="$3" label="$4" n rc
    if [ ! -f "$file" ]; then
        bad "$label (missing $file)"
        return
    fi
    n=$(grep -oE -- "$pat" "$file" | wc -l | tr -d ' ')
    rc=$?  # pipefail: grep's status (1 = no match, 2 = error)
    if [ "$rc" -gt 1 ]; then
        bad "$label (grep error)"
        return
    fi
    if [ "$n" -ne "$want" ]; then bad "$label (found $n, want $want)"; else ok "$label"; fi
}
# exists FILE LABEL
exists() {
    if [ -f "$1" ]; then ok "$2"; else bad "$2 (missing $1)"; fi
}
# before FILE ANCHOR A B LABEL — after the first ANCHOR (fixed string), A
# occurs before B; fails when any of the three is missing.
before() {
    local file="$1" anchor="$2" a="$3" b="$4" label="$5" res
    if [ ! -f "$file" ]; then
        bad "$label (missing $file)"
        return
    fi
    if res=$(python -c '
import sys
t = open(sys.argv[1], encoding="utf-8").read()
i = t.find(sys.argv[2])
if i < 0:
    print("anchor not found"); sys.exit(1)
t = t[i:]
pa, pb = t.find(sys.argv[3]), t.find(sys.argv[4])
print(f"{pa} vs {pb}")
sys.exit(0 if 0 <= pa < pb else 1)
' "$file" "$anchor" "$a" "$b"); then ok "$label ($res)"; else bad "$label ($res)"; fi
}

# block FILE START END NAME — copy the first START…END (fixed strings) of FILE
# to $out/blocks/NAME.html for count/none/exactly; fails when START is missing
# (the file is then absent, so later checks on it fail too).
mkdir -p "$out/blocks"
block() {
    local file="$1" start="$2" end="$3" name="$4"
    rm -f "$out/blocks/$name.html"
    if [ ! -f "$file" ]; then
        bad "block $name (missing $file)"
        return
    fi
    python -c '
import sys
t = open(sys.argv[1], encoding="utf-8").read()
i = t.find(sys.argv[2])
if i < 0:
    sys.exit(1)
j = t.find(sys.argv[3], i)
open(sys.argv[4], "w", encoding="utf-8").write(t[i : j + len(sys.argv[3])] if j >= 0 else t[i:])
' "$file" "$start" "$end" "$out/blocks/$name.html" || bad "block $name ($start not in $file)"
}

fs="$out/fixture-site"
fd="$out/fixture-docs"
page="$fs/articles/kitchen.html"

echo "== article ($page)"
count "$page" '<article class="report"' 1 "article.report shell"
count "$page" 'data-pagefind-body' 1 "pagefind body"
count "$page" 'data-pagefind-filter="section:Article"' 1 "pagefind section filter"
count "$page" '<p class="report-kicker">Article</p>' 1 "kicker = Article"
count "$page" '<div class="report-body"' 1 "div.report-body"
count "$page" '[0-9]+ words</span>' 1 "word count"
count "$page" '[0-9]+ min read' 1 "reading time"
count "$page" 'data-lang="python"' 1 "data-lang=python"
count "$page" 'data-lang="cpp"' 1 "data-lang=cpp"
count "$page" 'class="(highlight-default|literal-block)[^"]*"' 1 "bare literal block"
none "$page" '(highlight-default|literal-block)[^>]*data-lang' "bare literal block has no data-lang"
count "$page" 'highlight-bash notranslate"[^>]*(id="named-block"[^>]*data-lang="bash"|data-lang="bash"[^>]*id="named-block")' 1 "named code block has data-lang"
none "$page" 'highlight-(text|none) [^>]*data-lang' "text/none blocks have no data-lang"
count "$page" '<span class="k">' 1 "pygments token classes"
count "$page" '<span id="kitchen"></span><h1' 1 "labelled page title (h1 not first child)"
count "$page" 'class="doctest highlight-default notranslate"' 1 "doctest block (class before highlight-*)"
count "$page" 'admonition warning' 1 "warning admonition"
count "$page" 'footnote-reference' 1 "footnote reference"
count "$page" 'class="math notranslate' 2 "math (inline + display)"
count "$page" '<figcaption>' 1 "figcaption"
count "$page" '<aside class="book-menu"' 1 "sidebar"
count "$page" '<nav class="book-pagenav"' 1 "prev/next nav"
count "$page" 'class="book-pagenav-next" href="[^"]+" rel="next"' 1 "pagenav rel=next link"
count "$page" 'id="TableOfContents"' 1 "toc rendered"
count "$page" "localStorage\.getItem\('theme'\)" 1 "no-flash head snippet"
count "$page" 'id="menu-control"' 1 "mobile menu checkbox"
count "$page" 'id="toc-control"' 1 "mobile toc checkbox"
count "$page" '<link rel="canonical" href="https://example\.org/articles/kitchen\.html"' 1 "canonical url"

echo "== notes"
notes="$fs/notes/index.html"
count "$notes" 'class="post-list"' 1 "notes post-list"
exactly "$notes" 'class="post-list-item' 2 "notes list has 2 entries"
before "$notes" 'class="post-list"' '2026-01-02' '2025-12-31' "notes list newest first"
exactly "$notes" '<p class="post-list-desc">' 2 "notes list description class = post-list-desc"
exactly "$notes" '^<a class="post-list-title" href="[^"]+">' 2 "notes list title <a> directly in <li>"
none "$notes" '<p><a [^>]*post-list-title' "notes list title not wrapped in <p>"
# :style: rows / rows-desc: each row is one link (date · title · →), the
# description only in rows-desc; the default style above is unchanged.
count "$notes" 'class="post-rows"' 1 "notes rows list (ul.post-rows)"
block "$notes" 'limit-probe' '</ul>' notes-limit
exactly "$out/blocks/notes-limit.html" 'class="post-row"' 1 "rows :limit: 1 has exactly 1 row"
exactly "$out/blocks/notes-limit.html" '<li class="post-row"><a class="post-row-link" href="2026-01-02-second.html">' 1 "row = one link to the newest note"
count "$out/blocks/notes-limit.html" '<time class="post-row-date" datetime="2026-01-02">2026-01-02</time>' 1 "row date as <time datetime>"
count "$out/blocks/notes-limit.html" '<span class="post-row-title">Second note</span>' 1 "row title"
count "$out/blocks/notes-limit.html" '<span class="post-row-arrow" aria-hidden="true">→</span></a></li>' 1 "row arrow aria-hidden, last in the link"
none "$out/blocks/notes-limit.html" 'post-row-desc' "rows style has no description"
block "$notes" 'desc-probe' '</ul>' notes-desc
exactly "$out/blocks/notes-desc.html" 'class="post-row"' 2 "rows-desc lists both notes"
before "$out/blocks/notes-desc.html" 'post-rows' '2026-01-02' '2025-12-31' "rows-desc newest first"
exactly "$out/blocks/notes-desc.html" '<span class="post-row-desc">[^<]+</span></a></li>' 2 "rows-desc: description last in the link"
count "$out/blocks/notes-desc.html" '<span class="post-row-title">First note: A &amp; B</span>' 1 "row title escaped"
count "$out/blocks/notes-desc.html" '<span class="post-row-desc">The first note, written on new year&#x27;s eve &amp; &lt;after&gt;.</span>' 1 "row description escaped"
note="$fs/notes/2026-01-02-second.html"
count "$note" '<p class="report-kicker">Note</p>' 1 "kicker = Note"
# Section listing pages (notes/index): no default kicker, the description as
# subtitle, and no prev/next (it would repeat the first row of the list).
lst="$fs/notes/index.html"
exactly "$lst" 'class="report-kicker"' 0 "listing page: no kicker"
count "$lst" '<p class="report-subtitle">News about releases and the project\.</p>' 1 "listing page: description subtitle"
exactly "$lst" 'book-pagenav' 0 "listing page: no prev/next"
exactly "$lst" 'data-pagefind-filter="section:"' 0 "listing page: no empty pagefind section filter"
none "$note" 'words</span>' "no word count on notes"
exactly "$note" '<time datetime="2026-01-02"' 1 "note header has <time datetime>"
none "$note" 'data-pagefind-meta="title:[^"]*, date:' "pagefind title does not swallow the date"
count "$note" '<time datetime="2026-01-02" data-pagefind-meta="date\[datetime\]"' 1 "pagefind date on <time>"
exactly "$fs/articles/htmlout.html" '>[0-9]{1,2} words</span>' 1 "raw HTML output not counted as words"
count "$fs/articles/kitchen-md.html" 'class="report-subtitle"' 1 "kitchen-md description as subtitle"
# Pagefind indexes only pages with data-pagefind-body (design: never search,
# genindex, 404, _modules or stubs).
none "$fs/search.html" 'data-pagefind-body' "search page not indexed"
none "$fs/genindex.html" 'data-pagefind-body' "genindex page not indexed"
none "$fs/404.html" 'data-pagefind-body' "404 page not indexed"
count "$fs/search.html" 'class="report-header"' 1 "search page has report header"
count "$fs/genindex.html" 'class="report-header"' 1 "genindex page has report header"
none "$fs/articles/tiny.html" 'id="toc-control"' "no toc checkbox on page without sections"
feed="$fs/notes/feed.xml"
if [ ! -f "$feed" ]; then
    bad "feed.xml parses (missing $feed)"
elif python -c 'import sys, xml.etree.ElementTree as E; E.parse(sys.argv[1])' "$feed" 2>/dev/null; then
    ok "feed.xml parses"
else
    bad "feed.xml parses"
fi
exactly "$feed" '<item>' 2 "feed has 2 items"
before "$feed" '<item>' '2026-01-02' '2025-12-31' "feed newest first"

echo "== notebook, api, search"
tiny="$fs/articles/tiny.html"
count "$tiny" 'cell_input' 1 "notebook cell_input"
count "$tiny" 'cell_output' 1 "notebook cell_output"
count "$tiny" 'highlight-ipython3 notranslate"[^>]*data-lang="python"' 1 "ipython3 -> data-lang=python"
count "$tiny" '<pre><span></span>hello' 1 "notebook output text"
api="$fs/api/dummymod/index.html"
count "$api" 'sig sig-object py' 1 "autoapi signature"
count "$api" 'headerlink' 1 "headerlink"
count "$fs/search.html" 'searchtools\.js' 1 "search page loads searchtools.js"
count "$fs/search.html" 'name="q" aria-labelledby="search-documentation"' 1 "search page query input"
count "$fs/_static/pygments.css" '\.highlight \.k \{ color: #007020' 1 "friendly pygments style"

echo "== docs mode ($fd/index.html)"
count "$fd/index.html" 'book-version-switcher' 1 "version switcher"
count "$fd/index.html" 'data-match="v9\.9\.9"' 1 "switcher version match"
count "$fd/index.html" '<p class="report-kicker">Docs</p>' 1 "kicker = Docs"
none "$fd/index.html" 'book-section-title' "no docs section title (the bar names the section)"
count "$fd/articles/kitchen.html" 'toctree-l1 current' 1 "docs tree toctree-l1 current"
count "$fd/articles/kitchen.html" 'toctree-l2 current' 1 "docs tree toctree-l2 current"

echo "== 404 sidebar"
# notfound.extension rewrites the tree's links to absolute ones, so the tree stays.
count "$fd/404.html" 'toctree-l1' 1 "docs 404 keeps the tree"
count "$fd/404.html" '<li class="toctree-l1"><a class="reference internal" href="/articles/' 1 "docs 404 tree links absolute"
none "$fd/404.html" 'href="(\.\./)*articles/' "docs 404 has no relative tree links"
count "$fs/404.html" '<a class="bracket-label" href="/articles/"' 1 "site 404 has the bar nav with absolute /articles/"
none "$fs/404.html" 'toctree-l1' "site 404 has no tree"
# Without the extension the tree is skipped (docs mode: fixture-plain sets
# nav_section) and no empty li.book-tree is left.
count "$out/fixture-plain/index.html" 'toctree-l1' 1 "fixture-plain renders the docs tree"
none "$out/fixture-plain/404.html" 'toctree-l1' "404 without notfound has no tree"
none "$out/fixture-plain/404.html" 'class="book-tree"' "404 without notfound has no empty li.book-tree"
count "$out/fixture-plain/404.html" 'href="/docs/stable/"' 1 "404 without notfound keeps nav links"
count "$fs/_static/sphinx.css" 'li\.toctree-l1\.current > ul' 1 "sidebar expansion css"
none "$fs/_static/book.css" 'display: block math' "book.css stays verbatim (math fix lives in sphinx.css)"
count "$fs/_static/sphinx.css" 'math\[display="block"\] \{ display: block math' 1 "block MathML layout css"
count "$fs/_static/sphinx.css" '@media screen and \(min-width: 72rem\) \{ :root \{ --measure: 53rem; \} \}' 1 "wide-screen measure only inside min-width 72rem"
count "$fs/_static/sphinx.css" '--measure:' 1 "sphinx.css sets --measure nowhere else"
count "$fs/_static/sphinx.css" 'grid-template-columns: var\(--book-menu-w\) minmax\(0, var\(--measure\)\) var\(--book-toc-w\);' 1 "centred three-column book on wide screens"

echo "== landing"
# template: landing -> landing.html: hero (kicker, title, lede, CTAs, figure)
# instead of the report header, body as sections, no TOC and no prev/next.
land="$fs/landing.html"
exactly "$land" 'class="hero"' 1 "landing: one section.hero"
exactly "$land" 'class="hero-figure"' 1 "landing: one figure.hero-figure"
exactly "$land" '<figcaption' 1 "landing: one figcaption"
count "$land" '<figcaption class="bracket-label">Fig\. 1 — Trajectories through a bottleneck</figcaption>' 1 "landing: figure caption text"
exactly "$land" 'class="hf-alert"' 1 "landing: one hf-alert agent"
count "$land" 'class="hf-traj"' 8 "landing: >= 8 hf-traj trajectories"
exactly "$land" '<svg [^>]*role="img"[^>]*aria-label="Simulated trajectories of pedestrians leaving a room through a bottleneck"' 1 "landing: figure svg role=img + aria-label"
exactly "$land" '<p class="bracket-label hero-kicker">\[Jülich Pedestrian Simulator\]</p>' 1 "landing: hero kicker"
exactly "$land" '<h1 class="hero-title">Fixture landing</h1>' 1 "landing: hero title = page title"
exactly "$land" '<p class="hero-lede">A landing page for the fixture &amp; its checks\.</p>' 1 "landing: lede from description (escaped)"
exactly "$land" 'class="btn btn-solid"' 1 "landing: one solid button"
exactly "$land" '<a class="btn btn-solid" href="/docs/stable/">Get started →</a>' 1 "landing: primary CTA -> /docs/stable/"
exactly "$land" '<a class="btn" href="https://github\.com/PedestrianDynamics/jupedsim">GitHub</a>' 1 "landing: secondary CTA (outlined)"
exactly "$land" 'id="toc-control"' 0 "landing: no toc checkbox"
exactly "$land" 'class="book-toc"' 0 "landing: no toc"
exactly "$land" 'book-pagenav' 0 "landing: no prev/next"
exactly "$land" 'class="report-header"' 0 "landing: no report header"
exactly "$land" '<main class="book book--no-tree book--landing">' 1 "landing: main.book--landing"
count "$land" 'class="post-rows"' 1 "landing: notes-list rows"
exactly "$land" '<div class="rows docutils container">' 1 "landing: rows container"
exactly "$land" '<div class="report-body landing-body" data-pagefind-body>' 1 "landing: body indexed by Pagefind"
exactly "$land" '<div class="hero-text" data-pagefind-body>' 1 "landing: hero text (lede) indexed by Pagefind"
# The fixture landing page has no toc key: template: landing alone drops the TOC.
none "$fixture/landing.md" '^toc:' "landing fixture has no toc key"
# basic.css pads <figure>; the hero figure fills its grid cell.
count "$fs/_static/site.css" '^\.hero-figure \{ margin: 0; padding: 0; \}' 1 "hero figure: no basic.css margin/padding"
exactly "$land" 'data-pagefind-meta="title:Fixture landing"' 1 "landing: Pagefind title"
count "$land" '<title>Fixture landing' 1 "landing: <title> = page title"
before "$land" '<section class="hero">' 'class="hero-text"' 'class="hero-figure"' "landing: hero text before the figure"
before "$land" '<article class="landing"' '</section>' 'class="report-body landing-body"' "landing: body after the hero"
exactly "$page" '<main class="book book--no-tree">' 1 "article: no book--landing"
none "$page" 'class="hero' "article: no hero"
# Unknown template value: one warning naming page and value, default page.
bt="$out/fixture-badtpl.log"
exactly "$bt" ': (WARNING|ERROR|CRITICAL):' 1 "bad template: exactly one warning"
exactly "$bt" 'bad-template[^ ]*: WARNING: .*nope' 1 "bad template: warning names page and value"
count "$out/fixture-badtpl/bad-template.html" '<article class="report"' 1 "bad template: default page template"
none "$out/fixture-badtpl/bad-template.html" 'class="hero|book--landing' "bad template: no landing markup"
# CTA parsing: "Label|url"; missing | / empty label or url -> skipped + warning.
if res=$(cd "$theme_dir" && python - 2>&1 <<'P'
import sys
import jupedsim_book as jb
ok = [{"label": "Go", "url": "/docs/stable/"}, {"label": "B", "url": "https://x.org/a|b"}]
cases = [
    ({"cta_primary": " Go | /docs/stable/ ", "cta_secondary": "B|https://x.org/a|b"}, ok),
    ({"cta_primary": "Go|/docs/stable/", "cta_secondary": "no pipe"}, ok[:1]),
    ({"cta_primary": "Go|", "cta_secondary": "|/x"}, []),
    ({"cta_secondary": "Go|/docs/stable/"}, ok[:1]),
    ({}, []),
]
bad = [(m, jb._page_ctas(m, "p")) for m, want in cases if jb._page_ctas(m, "p") != want]
print(bad or "all cases")
sys.exit(bool(bad))
P
); then ok "CTA parsing ($(echo "$res" | tail -1))"
else bad "CTA parsing ($res)"; fi
exists "$theme_dir/jupedsim_book/hero-figure.svg" "hero-figure.svg in the theme"
hf_size=$(wc -c <"$theme_dir/jupedsim_book/hero-figure.svg" 2>/dev/null | tr -d ' ')
if [ -n "$hf_size" ] && [ "$hf_size" -le 15360 ]; then ok "hero-figure.svg <= 15360 bytes ($hf_size)"
else bad "hero-figure.svg <= 15360 bytes (${hf_size:-missing})"; fi
none "$theme_dir/jupedsim_book/hero-figure.svg" '(fill|stroke)="#|style=' "hero-figure.svg: colours only via classes"
count "$fs/_static/site.css" '--alert: #c22f1f' 1 "--alert light token"
count "$fs/_static/site.css" '--alert: #e04b30' 2 "--alert dark token (data-theme + auto)"

echo "== top bar"
# One bar per page; brand, search and theme toggle only in the bar; the left
# track holds the version select + docs tree on docs pages and nothing but
# the drawer nav on site pages.
for p in "$fs/index.html" "$page" "$fs/notes/2026-01-02-second.html" "$fs/404.html" \
    "$fs/search.html" "$fd/index.html" "$fd/articles/kitchen.html" "$fd/404.html"; do
    l="${p#"$out"/}"
    exactly "$p" '<header class="site-bar">' 1 "one header.site-bar ($l)"
    exactly "$p" 'id="book-theme-toggle"' 1 "one theme toggle ($l)"
    exactly "$p" 'id="book-search-input"' 1 "one search input ($l)"
    exactly "$p" 'class="book-header' 0 "no old header.book-header ($l)"
    b="menu-$(echo "$l" | tr '/.' '__')"
    block "$p" '<aside class="book-menu"' '</aside>' "$b"
    none "$out/blocks/$b.html" 'book-brand|book-search|book-theme-toggle' "left track has no brand/search/toggle ($l)"
    count "$out/blocks/$b.html" '<nav class="site-nav-drawer" aria-label="Site \(menu\)">' 1 "drawer nav in the left track ($l)"
    case "$p" in
    "$fs"/*)
        exactly "$p" 'id="book-version-switcher"' 0 "no version select on site pages ($l)"
        none "$out/blocks/$b.html" 'toctree-l1' "no docs tree on site pages ($l)"
        exactly "$p" '<main class="book book--no-tree">' 1 "main.book--no-tree on site pages ($l)"
        ;;
    *)
        exactly "$p" 'id="book-version-switcher"' 1 "version select on docs pages ($l)"
        count "$out/blocks/$b.html" 'toctree-l1' 1 "docs tree on docs pages ($l)"
        exactly "$p" '<main class="book">' 1 "main.book with tree track on docs pages ($l)"
        ;;
    esac
done
# Four bracketed items; the page's section is aria-current (prefix on the
# site, nav_section on docs pages); the landing page has none.
nav_items='<a class="bracket-label" href="/articles/"( aria-current="page")?>\[Articles\]</a>|<a class="bracket-label" href="/notes/"( aria-current="page")?>\[Notes\]</a>|<a class="bracket-label" href="/docs/stable/"( aria-current="page")?>\[Docs\]</a>|<a class="bracket-label" href="/docs/v1x/v1\.4\.2/"( aria-current="page")?>\[Docs v1&nbsp;<span aria-hidden="true">↗</span>\]</a>'
check_nav() {  # check_nav FILE NAME ACTIVE-HREF-or-empty
    local p="$1" n="$2" act="$3" l="${1#"$out"/}"
    block "$p" '<nav class="site-nav" aria-label="Site">' '</nav>' "nav-$n"
    exactly "$out/blocks/nav-$n.html" "$nav_items" 4 "bar nav has the four items ($l)"
    if [ -n "$act" ]; then
        exactly "$out/blocks/nav-$n.html" 'aria-current="page"' 1 "bar nav: one active item ($l)"
        exactly "$out/blocks/nav-$n.html" "<a class=\"bracket-label\" href=\"$act\" aria-current=\"page\">" 1 "bar nav: $act active ($l)"
    else
        none "$out/blocks/nav-$n.html" 'aria-current' "bar nav: no active item ($l)"
    fi
}
check_nav "$fs/index.html" site-index ""
check_nav "$fs/landing.html" site-landing ""
check_nav "$page" site-kitchen /articles/
check_nav "$fs/notes/2026-01-02-second.html" site-note /notes/
check_nav "$fd/articles/kitchen.html" docs-kitchen /docs/stable/
check_nav "$fd/index.html" docs-index /docs/stable/
count "$page" '<a class="bracket-label" href="/docs/v1x/v1\.4\.2/">\[Docs v1&nbsp;<span aria-hidden="true">↗</span>\]</a>' 2 "Docs v1 -> /docs/v1x/v1.4.2/ with ↗ (bar + drawer)"
exists "$fs/_static/site.css" "site.css shipped"
count "$fs/_static/site.css" 'scroll-margin-top' 1 "site.css sets scroll-margin-top"
count "$fs/_static/site.css" '--bar-h' 1 "site.css defines --bar-h"
before "$page" '<head>' '_static/book.css' '_static/site.css' "site.css loads after book.css"
for f in "$theme_dir/jupedsim_book/theme.conf" "$fixture/conf.py" "$repo/site/conf.py" \
    "$repo/docs/source/conf.py"; do
    none "$f" 'nav_before|nav_after|nav_title' "no nav_before/nav_after/nav_title in ${f#"$repo"/}"
done

echo "== footer"
# One grid-aligned footer after </main> on every page (404, search and
# genindex included); the old footer.book-footer inside .book-page is gone.
footer_text='<p class="site-footer-text">© JuPedSim contributors · LGPL-3.0</p>'
footer_links='<ul class="site-footer-links"><li><a href="https://github\.com/PedestrianDynamics/jupedsim">GitHub</a></li><li><a href="/notes/feed\.xml">RSS</a></li><li><a href="/docs/v1x/v1\.4\.2/">1\.x docs</a></li></ul>'
for p in "$fs/index.html" "$page" "$fs/notes/index.html" "$fs/404.html" "$fs/search.html" \
    "$fs/genindex.html" "$fd/index.html" "$fd/articles/kitchen.html" "$fd/404.html" \
    "$fd/search.html" "$fd/genindex.html" "$out/fixture-plain/404.html"; do
    l="${p#"$out"/}"
    exactly "$p" '<footer class="site-footer">' 1 "one footer.site-footer ($l)"
    exactly "$p" 'class="book-footer"' 0 "no footer.book-footer ($l)"
    before "$p" '<main class="book' '</main>' '<footer class="site-footer">' "footer after </main> ($l)"
    b="footer-$(echo "$l" | tr '/.' '__')"
    block "$p" '<footer class="site-footer">' '</footer>' "$b"
    count "$out/blocks/$b.html" '<div class="site-footer-inner">' 1 "footer inner box ($l)"
    exactly "$out/blocks/$b.html" "$footer_text" 1 "footer text ($l)"
    exactly "$out/blocks/$b.html" "$footer_links" 1 "footer links GitHub · RSS · 1.x docs ($l)"
    count "$out/blocks/$b.html" 'href="/docs/v1x/v1\.4\.2/"' 1 "footer links the 1.x docs ($l)"
done
count "$fs/_static/site.css" '\.site-footer' 1 "site.css styles the footer"
# Short pages: body is a column and main takes the leftover height, so the
# footer ends the screen instead of starting below it (smoke: footer-in-view).
count "$fs/_static/site.css" '^body \{ display: flex; flex-direction: column; min-height: 100vh; \}' 1 "body is a full-height column"
count "$fs/_static/site.css" '^\.book \{ flex: 1 0 auto; \}' 1 "main takes the leftover height"
none "$fs/_static/site.css" 'book-(menu|toc) \{[^}]* height: calc\(100vh' "sticky columns cap their height (max-height), they do not set it"

echo "== colour restraint"
# The accent is kept for running-text links, focus, the bar's active item
# and the current tree item: no shipped rule for kickers, TOC, tree links,
# version select, post lists, rows, buttons, footer or page nav uses it
# (focus rings and a.current excepted).
if res=$(python - "$fs/_static" <<'P'
import pathlib, re, sys
targets = (".report-kicker", "#TableOfContents", ".book-tree a", ".book-version-switcher",
           ".post-list", ".post-row", ".rows ", ".btn", ".site-footer", ".book-pagenav")
seen, hits = set(), []
for css in sorted(pathlib.Path(sys.argv[1]).glob("*.css")):
    for sel, body in re.findall(r"([^{}]+)\{([^{}]*)\}", css.read_text(encoding="utf-8")):
        for s in (x.strip() for x in re.split(r",(?![^()]*\))", sel)):
            t = next((t for t in targets if t in s + " "), None)
            if t is None or "focus" in s or "a.current" in s:
                continue
            seen.add(t)
            if "accent" in body:
                hits.append(f"{css.name}: {s}")
missing = [t for t in targets if t not in seen]
print("; ".join(hits) or "none", "| no rule for:", missing or "-")
sys.exit(bool(hits or missing))
P
); then ok "no accent in kicker/TOC/tree/select/list/row/button/footer/pagenav rules ($res)"
else bad "no accent in kicker/TOC/tree/select/list/row/button/footer/pagenav rules ($res)"; fi
block "$fs/_static/jupedsim-style.css" '.report-kicker {' '}' kicker-rule
count "$out/blocks/kicker-rule.html" 'color: var\(--ink-soft\);' 1 ".report-kicker is ink-soft"
none "$out/blocks/kicker-rule.html" 'accent' ".report-kicker rule has no accent"

echo "== logo"
# Two theme-shipped <img> marks (light + dark), not an inlined brand <svg>;
# the link's aria-label names it in every theme (the light <img> is
# display: none in dark mode) and the dark copy is aria-hidden. width/height
# are each shipped file's viewBox size, rounded (layout box before load).
svg_size() {
    python -c '
import re, sys, xml.etree.ElementTree as ET
vb = ET.parse(sys.argv[1]).getroot().get("viewBox")
w, h = (float(v) for v in re.split(r"[\s,]+", vb.strip())[2:4])
print(f"width=\"{round(w)}\" height=\"{round(h)}\"")
' "$1" 2>/dev/null || echo "width=\"unreadable\" height=\"unreadable\""
}
size_light="$(svg_size "$fs/_static/jupedsim-mark-light.svg")"
size_dark="$(svg_size "$fs/_static/jupedsim-mark-dark.svg")"
echo "      mark sizes: light $size_light, dark $size_dark"
logo_light="<img class=\"book-logo book-logo-light\" src=\"(\\.\\./)*_static/jupedsim-mark-light\\.svg\" alt=\"JuPedSim\" $size_light>"
logo_dark="<img class=\"book-logo book-logo-dark\" src=\"(\\.\\./)*_static/jupedsim-mark-dark\\.svg\" alt=\"\" aria-hidden=\"true\" $size_dark>"
for p in "$page" "$fd/index.html"; do
    exactly "$p" "$logo_light" 1 "light mark <img> with viewBox size (${p#"$out"/})"
    exactly "$p" "$logo_dark" 1 "dark mark <img> with viewBox size, aria-hidden (${p#"$out"/})"
    none "$p" '<a class="book-brand"[^>]*>[[:space:]]*<svg|sodipodi|inkscape:' "no inline brand <svg> (${p#"$out"/})"
    exactly "$p" 'alt="JuPedSim"' 1 "brand named once (${p#"$out"/})"
    exactly "$p" '<a class="book-brand" href="[^"]*" aria-label="JuPedSim">' 1 "brand link aria-label (${p#"$out"/})"
done
# fixture-plain sets logo_dark empty: one logo without the -light class, so
# the dark-mode rules cannot hide it.
exactly "$out/fixture-plain/index.html" "<img class=\"book-logo\" src=\"(\\.\\./)*_static/jupedsim-mark-light\\.svg\" alt=\"JuPedSim\" $size_light>" 1 "single logo when logo_dark is empty"
none "$out/fixture-plain/index.html" 'book-logo-(light|dark)' "single logo has no light/dark class"
exists "$fs/_static/jupedsim-mark-light.svg" "light mark in _static"
exists "$fs/_static/jupedsim-mark-dark.svg" "dark mark in _static"
# vector-effect is not inherited: it must sit on each stroked element, so the
# four '+' registration marks keep their 1.6px stroke at any scale.
for v in light dark; do
    f="$fs/_static/jupedsim-mark-$v.svg"
    exactly "$f" '<path d="M[^"]*" vector-effect="non-scaling-stroke"/>' 4 "non-scaling + marks ($v mark)"
    none "$f" '<g [^>]*vector-effect' "no vector-effect on a <g> ($v mark)"
done
# The full lockups left the theme: no file or reference anywhere.
for d in "$fs" "$fd" "$out/fixture-plain" "$theme_dir/jupedsim_book" "$fixture"; do
    label="${d#"$out"/}"
    absent "$d" 'jupedsim-labs-' "no jupedsim-labs- in ${label#"$repo"/}"
done
for f in "$repo/site/conf.py" "$repo/docs/source/conf.py"; do
    [ -f "$f" ] && none "$f" 'jupedsim-labs-' "no jupedsim-labs- in ${f#"$repo"/}"
done
# Theme marks are byte-identical to the logo sources (local checkout only).
logo_src="$HOME/workspaces/jps/jupedsim-labs-logos"
if [ -d "$logo_src" ]; then
    for v in light dark; do
        if cmp -s "$theme_dir/jupedsim_book/static/jupedsim-mark-$v.svg" "$logo_src/jupedsim-mark-$v.svg"; then
            ok "jupedsim-mark-$v.svg equals its source"
        else
            bad "jupedsim-mark-$v.svg differs from $logo_src/jupedsim-mark-$v.svg"
        fi
    done
else
    echo "SKIP: mark sources ($logo_src does not exist)"
fi
exists "$fs/_static/jupedsim-style.css" "base stylesheet shipped"
count "$fs/_static/sphinx.css" '^\.book-brand \.book-logo-dark \{ display: none; \}' 1 "dark logo hidden by default (print, light)"
count "$fs/_static/sphinx.css" ':root\[data-theme="dark"\] \.book-brand \.book-logo-dark \{ display: block; \}' 1 "dark logo shown for data-theme=dark"
count "$fs/_static/sphinx.css" ':root:not\(\[data-theme\]\) \.book-brand \.book-logo-dark \{ display: block; \}' 1 "dark logo shown for prefers-color-scheme without JS"
count "$fs/_static/sphinx.css" ':root\[data-theme="dark"\] \.book-brand \.book-logo-light \{ display: none; \}' 1 "light logo hidden for data-theme=dark"
count "$fs/_static/sphinx.css" ':root:not\(\[data-theme\]\) \.book-brand \.book-logo-light \{ display: none; \}' 1 "light logo hidden for prefers-color-scheme without JS"
# Theme-switching logo rules sit only inside `@media screen` blocks (print
# keeps the light logo).
if res=$(python -c '
import re, sys
media, bad, n = None, [], 0
for i, line in enumerate(open(sys.argv[1], encoding="utf-8"), 1):
    if re.match(r"@media", line):
        media = line
    elif re.match(r"\S", line):
        media = None
    if "data-theme" in line and ".book-logo" in line:
        n += 1
        if not (media or "").startswith("@media screen"):
            bad.append(str(i))
if n != 4 or bad:
    sys.exit(f"{n} rules, outside @media screen: lines {bad}")
' "$fs/_static/sphinx.css" 2>&1); then
    ok "logo theme rules screen-only"
else
    bad "logo theme rules screen-only ($res)"
fi
for f in "$theme_dir/jupedsim_book/theme.conf" "$theme_dir/jupedsim_book/__init__.py" \
    "$theme_dir/jupedsim_book/sidebar.html" "$theme_dir/jupedsim_book/topbar.html" \
    "$fixture/conf.py" "$repo/site/conf.py" \
    "$repo/docs/source/conf.py"; do
    none "$f" 'brand_svg' "no brand_svg in ${f#"$repo"/}"
done

# Sphinx sorts attributes (<script defer="defer" src=…>, <link rel=… type=… href=…>),
# so match any attribute order; rel="canonical" may point off-site.
ext_script='<script[^>]*src="(https?:)?//'
ext_link='<link[^>]*rel="(stylesheet|preload|modulepreload)"[^>]*href="(https?:)?//'
ext_css="@import|url\\([\"']?(https?:)?//"
no_external() {
    local d="$1" n
    n="$(basename "$d")"
    absent "$d" "$ext_script" "no external script ($n)"
    absent "$d" "$ext_link" "no external stylesheet ($n)"
    absent "$d/_static" "$ext_css" "no external css import/url ($n)"
}
echo "== no external requests"
no_external "$fs"
no_external "$fd"

echo "== real docs"
count "$out/docs/index.html" 'book-version-switcher' 1 "docs version switcher"
exists "$out/docs/api/jupedsim/index.html" "docs api/jupedsim/index.html"
count "$out/docs/index.html" 'class="book-logo book-logo-dark"' 1 "docs logo (light + dark)"
count "$out/docs/404.html" 'toctree-l1' 1 "docs 404 keeps the tree"
count "$out/docs/404.html" '<a class="bracket-label" href="/articles/"' 1 "docs 404 bar nav (Articles)"
for f in "$out/docs/index.html" "$out/docs/404.html"; do
    exactly "$f" "$footer_text" 1 "footer text (${f#"$out"/})"
    exactly "$f" "$footer_links" 1 "footer links (${f#"$out"/})"
done
count "$out/docs/api/jupedsim/index.html" 'data-pagefind-body' 1 "docs api page indexed"
none "$out/docs/_modules/jupedsim/simulation.html" 'data-pagefind-body' "docs _modules page not indexed"
none "$out/docs/genindex.html" 'data-pagefind-body' "docs genindex page not indexed"
# Unresolved autoapi imports are filtered by message in conf.py, never
# suppressed as a whole type (that would hide new gaps in the API reference).
none "$repo/docs/source/conf.py" '"autoapi\.python_import_resolution"' \
    "docs conf does not suppress all autoapi import warnings"

echo "== real site"
if [ "$have_site" -eq 1 ]; then
    exists "$out/site/index.html" "site index.html"
    exists "$out/site/404.html" "site 404.html"
    count "$out/site/index.html" 'class="book-logo book-logo-dark"' 1 "site logo (light + dark)"
    exists "$out/site/notes/feed.xml" "site notes/feed.xml"
    for f in "$out/site/index.html" "$out/site/404.html" "$out/site/articles/writing-guide.html"; do
        exactly "$f" "$footer_text" 1 "footer text (${f#"$out"/})"
        exactly "$f" "$footer_links" 1 "footer links (${f#"$out"/})"
    done
    no_external "$out/site"
    # Build-time MathML: accents keep their scripts to the right, numbered
    # equations show "(n)", amsmath environments render as tables.
    g="$out/site/articles/writing-guide.html"
    none "$g" '<munderover>' "site math: accent + subscript is not an under/over script"
    count "$g" '<span class="eqno">\(1\)' 1 "site math: equation number"
    count "$g" '<mtable' 1 "site math: amsmath align rendered"
    count "$g" 'id="notebook-articles"' 1 "writing guide documents notebook articles"
    # Landing page: hero + [DOCS] rows into /docs/stable/ (each target exists
    # in the docs build) + articles/notes rows (<= 5 each).
    si="$out/site/index.html"
    exactly "$si" '<main class="book book--no-tree book--landing">' 1 "site index is the landing page"
    exactly "$si" '<h1 class="hero-title">Pedestrian dynamics, simulated</h1>' 1 "site landing: hero title"
    exactly "$si" '<a class="btn btn-solid" href="/docs/stable/">Get started →</a>' 1 "site landing: Get started -> /docs/stable/"
    exactly "$si" 'class="hf-alert"' 1 "site landing: hero figure"
    exactly "$si" 'id="toc-control"|class="book-toc"|book-pagenav|class="report-header"' 0 "site landing: no TOC, prev/next or report header"
    exactly "$si" '<div class="rows docutils container">' 1 "site landing: [DOCS] rows"
    exactly "$si" '<li><p><a href="/docs/(stable|v1x)/[^"]*">' 5 "site landing: five docs rows"
    count "$si" '<li><p><a href="/docs/v1x/v1\.4\.2/">1\.x documentation \(archive\)</a>' 1 "site landing: 1.x archive row -> v1.4.2"
    exactly "$si" '<ul class="post-rows">' 2 "site landing: articles + notes rows"
    exactly "$si" '<div class="hero-text" data-pagefind-body>' 1 "site landing: hero text indexed by Pagefind"
    none "$si" 'MyST would resolve' "site landing: source comments stay in index.md"
    exactly "$repo/site/index.md" ':limit: 5' 2 "site landing: rows limited to 5"
    if res=$(python - "$si" "$out/docs" <<'P'
import pathlib, re, sys
html = pathlib.Path(sys.argv[1]).read_text(encoding="utf-8")
docs = pathlib.Path(sys.argv[2])
rows = re.findall(r'<li><p><a href="/docs/stable/([^"]*)">', html)
missing = [r for r in rows if not (docs / (r or "index.html")).is_file()]
print(f"{len(rows)} rows, missing: {missing or '-'}")
sys.exit(len(rows) != 4 or bool(missing))
P
    ); then ok "site landing: docs rows exist in the docs build ($res)"
    else bad "site landing: docs rows exist in the docs build ($res)"; fi
    none "$out/site/index.html" 'Last updated' "site pages carry no build-date footer"
    none "$out/site/notes/2026-09-28-new-website.html" 'Last updated' "notes carry no build-date footer"
    # /notes/ is the archive: it lists every note (only the landing page is limited).
    none "$repo/site/notes/index.md" ':limit:' "notes archive has no :limit:"
    n_notes=$(find "$repo/site/notes" -name '20*.md' | wc -l | tr -d ' ')
    exactly "$out/site/notes/index.html" 'class="post-row"' "$n_notes" "notes archive lists all $n_notes notes as rows"
    exactly "$out/site/notes/index.html" '<span class="post-row-desc">' "$n_notes" "notes archive rows have descriptions"
    # /articles/: rows with descriptions; the hidden toctree keeps prev/next.
    n_art=$(find "$repo/site/articles" \( -name '*.md' -o -name '*.ipynb' \) ! -name 'index.*' | wc -l | tr -d ' ')
    exactly "$out/site/articles/index.html" 'class="post-row"' "$n_art" "articles index lists all $n_art articles as rows"
    exactly "$out/site/articles/index.html" '<span class="post-row-desc">' "$n_art" "articles index rows have descriptions"
    none "$out/site/articles/index.html" 'toctree-l1' "articles toctree is hidden (no visible entries)"
    exactly "$out/site/articles/index.html" 'book-pagenav' 0 "articles index: no prev/next (would repeat its first row)"
    exactly "$out/site/articles/index.html" 'class="report-kicker"' 0 "articles index: no kicker"
    exactly "$out/site/notes/index.html" 'book-pagenav' 0 "notes index: no prev/next"
    count "$out/site/articles/writing-guide.html" 'class="book-pagenav-prev" href="index\.html" rel="prev"' 1 "first article <- articles index"
    # build_site.py keeps build state (doctrees) out of the published tree.
    a="$out/assembled"
    if python "$repo/scripts/build_site.py" --source "$repo" --out "$a" --version v0 \
        --only site --no-notebooks >"$out/assembled.log" 2>&1; then
        ok "build_site.py --only site"
    else
        bad "build_site.py --only site (log: $out/assembled.log)"
    fi
    exists "$a/index.html" "assembled site index.html"
    if [ -e "$a/.doctrees" ]; then bad "assembled site has no .doctrees"; else ok "assembled site has no .doctrees"; fi
    if [ -d "$a-cache/doctrees-site" ]; then ok "doctrees kept in <out>-cache"; else bad "doctrees kept in <out>-cache (missing $a-cache/doctrees-site)"; fi
else
    bad "real site built (site/conf.py missing)"
fi

# The /docs/ stub written at release time forwards query and hash.
count "$repo/.github/workflows/deploy-documentation.yml" \
    'location\.replace\("/docs/stable/" \+ location\.search \+ location\.hash\)' 1 \
    "deploy-documentation.yml: /docs/ stub keeps location.search + hash"

# The deploy scripts against a fake gh-pages tree (design: CI / deploy, Risks):
# the pre-2.0 docs/stable stub dir becomes a symlink, docs/v1x (migrated: no
# stable) and the frozen root versions.json stay untouched, Pagefind indexes the site plus a 2.x
# docs/stable only. Every page carries data-pagefind-body and a unique word,
# so only the directory scope of build_pagefind.sh keeps a page out.
echo "== gh-pages layout"
t="$out/ghpages"
ghp_page() {
    mkdir -p "$(dirname "$t/$1")"
    printf '<!doctype html><html lang="en"><head><title>%s</title></head><body><main data-pagefind-body><h1>%s</h1><p>%s page</p></main></body></html>\n' \
        "$2" "$2" "$2" >"$t/$1"
}
# pf_urls NAME — decompress the Pagefind fragments under $t/pagefind into
# $out/NAME (one "URL CONTENT" line per indexed page).
pf_urls() {
    python - "$t/pagefind/fragment" >"$out/$1" <<'EOF'
import gzip, json, pathlib, sys
for f in sorted(pathlib.Path(sys.argv[1]).glob("*.pf_fragment")):
    data = gzip.decompress(f.read_bytes())
    frag = json.loads(data[data.index(b"{") :])
    print(frag["url"], frag["content"])
EOF
}
ghp_page index.html sitehome
ghp_page notes/n.html sitenote
ghp_page v1.4.2/index.html rootold
ghp_page stable/index.html rootstable
echo '[{"name": "v1.4.2", "url": "https://www.jupedsim.org/docs/v1x/v1.4.2/", "preferred": true}, {"name": "v1.4.1", "url": "https://www.jupedsim.org/docs/v1x/v1.4.1/"}]' >"$t/versions.json"
ghp_page docs/v1x/v1.4.1/index.html archiveolder
ghp_page docs/v1x/v1.4.2/index.html archiveold
cp "$t/versions.json" "$t/docs/v1x/versions.json"
ghp_page docs/stable/index.html stubpage
ghp_page docs/v2.0.0/index.html twozero
ghp_page docs/v2.1.0/index.html twoone
cp "$t/versions.json" "$out/ghpages-root-versions.json"

if bash "$repo/scripts/ci/build_pagefind.sh" "$t" >"$out/ghpages-pf1.log" 2>&1; then
    ok "build_pagefind.sh before 2.0 (stub dir)"
    pf_urls ghpages-urls-stub.txt
    exactly "$out/ghpages-urls-stub.txt" '^/' 2 "before 2.0: only the 2 site pages indexed"
    none "$out/ghpages-urls-stub.txt" '^/(docs|stable|v[0-9])' "before 2.0: no docs, stub or 1.x pages indexed"
else
    bad "build_pagefind.sh before 2.0 (log: $out/ghpages-pf1.log)"
fi

wdv() { python "$repo/scripts/ci/write_docs_versions.py" "$t/docs" >>"$out/ghpages-wdv.log" 2>&1; }
if wdv; then ok "write_docs_versions.py"; else bad "write_docs_versions.py (log: $out/ghpages-wdv.log)"; fi
if [ -L "$t/docs/stable" ] && [ "$(readlink "$t/docs/stable")" = "v2.1.0" ]; then
    ok "docs/stable stub dir replaced by symlink -> v2.1.0"
else
    bad "docs/stable stub dir replaced by symlink -> v2.1.0"
fi
if res=$(python -c '
import json, sys
got = [(e["name"], e["url"], e.get("preferred", False)) for e in json.load(open(sys.argv[1]))]
p = "https://www.jupedsim.org/docs/"
want = [("stable", p + "stable/", True), ("v2.1.0", p + "v2.1.0/", False), ("v2.0.0", p + "v2.0.0/", False),
        ("1.x (archive)", p + "v1x/v1.4.2/", False)]
print(got)
last = json.load(open(sys.argv[1]))[-1]
sys.exit(got != want or last != {"name": "1.x (archive)", "version": "v1x", "url": p + "v1x/v1.4.2/"})
' "$t/docs/versions.json" 2>&1); then
    ok "docs/versions.json = stable (preferred), v2.1.0, v2.0.0, 1.x (archive -> v1x/v1.4.2/, last, not preferred) under /docs/"
else
    bad "docs/versions.json = stable (preferred), v2.1.0, v2.0.0, 1.x (archive -> v1x/v1.4.2/, last, not preferred) under /docs/ (got $res)"
fi
# Without docs/v1x (layout not migrated): no archive entry.
mkdir -p "$out/ghpages-nov1x/v2.0.0"
res=
if python "$repo/scripts/ci/write_docs_versions.py" "$out/ghpages-nov1x" >>"$out/ghpages-wdv.log" 2>&1 &&
    res=$(python -c '
import json, sys
got = [e["name"] for e in json.load(open(sys.argv[1]))]
print(got)
sys.exit(got != ["stable", "v2.0.0"])
' "$out/ghpages-nov1x/versions.json" 2>&1); then
    ok "docs/versions.json without docs/v1x has no archive entry"
else
    bad "docs/versions.json without docs/v1x has no archive entry (got ${res:-error, log: $out/ghpages-wdv.log})"
fi
# docs/v1x without a version directory: no archive entry either.
mkdir -p "$out/ghpages-emptyv1x/v2.0.0" "$out/ghpages-emptyv1x/v1x"
res=
if python "$repo/scripts/ci/write_docs_versions.py" "$out/ghpages-emptyv1x" >>"$out/ghpages-wdv.log" 2>&1 &&
    res=$(python -c '
import json, sys
got = [e["name"] for e in json.load(open(sys.argv[1]))]
print(got)
sys.exit(got != ["stable", "v2.0.0"])
' "$out/ghpages-emptyv1x/versions.json" 2>&1); then
    ok "docs/versions.json with an empty docs/v1x has no archive entry"
else
    bad "docs/versions.json with an empty docs/v1x has no archive entry (got ${res:-error, log: $out/ghpages-wdv.log})"
fi
if cmp -s "$t/versions.json" "$out/ghpages-root-versions.json"; then ok "root versions.json untouched"; else bad "root versions.json untouched"; fi
if cmp -s "$t/docs/v1x/versions.json" "$out/ghpages-root-versions.json" && [ ! -e "$t/docs/v1x/stable" ] && [ ! -L "$t/docs/v1x/stable" ]; then
    ok "docs/v1x untouched"
else
    bad "docs/v1x untouched"
fi
m1=$(python -c 'import os, sys; print(os.stat(sys.argv[1]).st_mtime_ns)' "$t/docs/versions.json")
wdv
m2=$(python -c 'import os, sys; print(os.stat(sys.argv[1]).st_mtime_ns)' "$t/docs/versions.json")
if [ "$m1" = "$m2" ] && [ "$(readlink "$t/docs/stable")" = "v2.1.0" ]; then
    ok "write_docs_versions.py second run changes nothing"
else
    bad "write_docs_versions.py second run changes nothing"
fi

if bash "$repo/scripts/ci/build_pagefind.sh" "$t" >"$out/ghpages-pf2.log" 2>&1; then
    ok "build_pagefind.sh after 2.x (symlink)"
    pf_urls ghpages-urls-2x.txt
    exactly "$out/ghpages-urls-2x.txt" '^/' 3 "2.x: site pages + docs/stable indexed"
    count "$out/ghpages-urls-2x.txt" '^/docs/stable/ .*twoone' 1 "2.x: docs/stable indexed from the newest version"
    none "$out/ghpages-urls-2x.txt" 'stubpage' "2.x: stub page not indexed"
    none "$out/ghpages-urls-2x.txt" '^/(stable|v[0-9])|^/docs/(v1x|v2)' "2.x: no 1.x or versioned docs pages indexed"
else
    bad "build_pagefind.sh after 2.x (log: $out/ghpages-pf2.log)"
fi
# The index records what built it (build_pagefind.sh --stamp), so a publisher
# can tell an index built by an older build_pagefind.sh or Pagefind from a
# current one.
if stamp=$(bash "$repo/scripts/ci/build_pagefind.sh" --stamp 2>&1) &&
    [ "$stamp" = "$(printf 'build_pagefind.sh sha256:%s\n%s' \
        "$(python -c 'import hashlib, sys; print(hashlib.sha256(open(sys.argv[1], "rb").read()).hexdigest())' \
            "$repo/scripts/ci/build_pagefind.sh")" \
        "$(python -m pagefind --version)")" ]; then
    ok "build_pagefind.sh --stamp = script sha256 + Pagefind version"
else
    bad "build_pagefind.sh --stamp = script sha256 + Pagefind version (got: $stamp)"
fi
if [ -f "$t/pagefind/build-stamp.txt" ] && [ "$(cat "$t/pagefind/build-stamp.txt")" = "$stamp" ]; then
    ok "pagefind/build-stamp.txt = build_pagefind.sh --stamp"
else
    bad "pagefind/build-stamp.txt = build_pagefind.sh --stamp"
fi

echo "== requirements"
# The site build (build-docs action, deploy-next-doc) gets Pagefind
# only from site/requirements.txt.
count "$repo/site/requirements.txt" '^pagefind\[bin\]~=1\.5$' 1 "site/requirements.txt pins pagefind[bin]"

echo "== next-doc publish"
# scripts/ci/publish_site_tree.sh <build-out> <pages-root> (deploy-next-doc.yml):
# replaces the site and docs, keeps CNAME, .nojekyll and the manual docs/v1x copy,
# writes robots.txt, versions.json (with the archive entry) and the Pagefind index.
nd_build="$out/nd-build"
nd_pages="$out/nd-pages"
rm -rf "$nd_build" "$nd_pages" "$out/nd-v1x-before"
t="$nd_build"
ghp_page index.html ndsitehome
ghp_page docs/v2.0.0/index.html ndtwozero
ln -s v2.0.0 "$nd_build/docs/stable"
echo '[]' >"$nd_build/docs/versions.json"
t="$nd_pages"
mkdir -p "$nd_pages"
echo next-doc.jupedsim.org >"$nd_pages/CNAME"
touch "$nd_pages/.nojekyll"
ghp_page old.html ndold
ghp_page docs/v1.9.9/index.html ndstale
ghp_page docs/v1x/v1.4.2/index.html ndarchiveword
echo '[{"name": "v1.4.2", "url": "https://www.jupedsim.org/docs/v1x/v1.4.2/", "preferred": true}]' >"$nd_pages/docs/v1x/versions.json"
cp -R "$nd_pages/docs/v1x" "$out/nd-v1x-before"
if bash "$repo/scripts/ci/publish_site_tree.sh" "$nd_build" "$nd_pages" >"$out/nd-publish.log" 2>&1; then
    ok "publish_site_tree.sh runs"
else
    bad "publish_site_tree.sh runs (log: $out/nd-publish.log)"
fi
[ "$(cat "$nd_pages/CNAME" 2>/dev/null)" = "next-doc.jupedsim.org" ] && [ -f "$nd_pages/.nojekyll" ] \
    && ok "CNAME and .nojekyll kept" || bad "CNAME and .nojekyll kept"
count "$nd_pages/robots.txt" '^Disallow: /$' 1 "robots.txt disallows indexing"
[ ! -e "$nd_pages/old.html" ] && [ ! -e "$nd_pages/docs/v1.9.9" ] \
    && ok "stale site page and stale docs version removed" || bad "stale site page and stale docs version removed"
diff -r "$out/nd-v1x-before" "$nd_pages/docs/v1x" >/dev/null \
    && ok "manual docs/v1x copy untouched" || bad "manual docs/v1x copy untouched"
[ "$(readlink "$nd_pages/docs/stable")" = "v2.0.0" ] \
    && ok "docs/stable -> v2.0.0" || bad "docs/stable -> v2.0.0 (got $(readlink "$nd_pages/docs/stable"))"
if res=$(python - "$nd_pages/docs/versions.json" <<'P'
import json, sys
d = json.load(open(sys.argv[1]))
names = [e["name"] for e in d]
print(names, d[-1]["url"])
sys.exit(not (names == ["stable", "v2.0.0", "1.x (archive)"]
              and d[-1]["url"] == "https://www.jupedsim.org/docs/v1x/v1.4.2/"))
P
); then ok "versions.json: stable, v2.0.0, 1.x (archive)"; else bad "versions.json: stable, v2.0.0, 1.x (archive) (got $res)"; fi
if [ -f "$nd_pages/pagefind/pagefind.js" ]; then
    pf_urls nd-urls.txt
    count "$out/nd-urls.txt" 'ndtwozero' 1 "next-doc index has the 2.x docs"
    none "$out/nd-urls.txt" 'ndarchiveword' "next-doc index skips docs/v1x"
else
    bad "next-doc pagefind/pagefind.js written"
fi
# Without the manual 1.x copy: warns, no archive entry, still succeeds.
rm -rf "$nd_pages/docs/v1x"
if bash "$repo/scripts/ci/publish_site_tree.sh" "$nd_build" "$nd_pages" >"$out/nd-publish2.log" 2>&1 &&
    grep -q '::warning::' "$out/nd-publish2.log" &&
    ! grep -q '1.x (archive)' "$nd_pages/docs/versions.json"; then
    ok "without docs/v1x: warning, no archive entry"
else
    bad "without docs/v1x: warning, no archive entry (log: $out/nd-publish2.log)"
fi
# A broken build output is refused before anything is touched.
mkdir -p "$out/nd-empty"
cp "$nd_pages/CNAME" "$out/nd-cname-before"
bash "$repo/scripts/ci/publish_site_tree.sh" "$out/nd-empty" "$nd_pages" >"$out/nd-publish3.log" 2>&1
rc=$?
if [ "$rc" -eq 2 ] && [ -f "$nd_pages/index.html" ] && cmp -s "$out/nd-cname-before" "$nd_pages/CNAME"; then
    ok "empty build output refused (exit 2), pages root unchanged"
else
    bad "empty build output refused (exit 2), pages root unchanged (rc=$rc)"
fi

echo "== preview links"
# scripts/ci/rebase_preview_links.py <tree> <prefix> --stable <dir>
# (deploy-previews.yml): previews live under /pull-requests/<N>/, so every
# root-absolute URL the site uses must be moved under that prefix, and
# docs/stable must exist (PR builds name their docs docs/dev/).
pv="$out/preview"
rm -rf "$pv"
mkdir -p "$pv/docs/dev/concepts" "$pv/notes"
cat >"$pv/index.html" <<'H'
<a href="/docs/stable/">D</a> <a href='/articles/'>A</a> <img src="/_static/x.png">
<input data-pagefind="/pagefind/pagefind.js"> <form action="/search.html"></form>
<a href="//cdn.example.org/x.js">cdn</a> <a href="https://www.jupedsim.org/">abs</a>
<a href="notes/index.html">rel</a> <a href="#top">frag</a>
<p>Escaped text href=&quot;/not-an-attribute&quot;</p>
H
cat >"$pv/docs/dev/concepts/routing.html" <<'H'
<select data-json="/docs/versions.json"></select><a href="/docs/v1x/v1.4.2/">v1</a>
H
echo '[{"name": "dev", "version": "dev", "url": "/docs/dev/", "preferred": true}, {"name": "x", "url": "https://www.jupedsim.org/docs/x/"}]' >"$pv/docs/versions.json"
if python "$repo/scripts/ci/rebase_preview_links.py" "$pv" /pull-requests/42/ --stable dev >"$out/preview.log" 2>&1 &&
    python "$repo/scripts/ci/rebase_preview_links.py" "$pv" /pull-requests/42/ --stable dev >>"$out/preview.log" 2>&1; then
    ok "rebase_preview_links.py runs (twice)"
else
    bad "rebase_preview_links.py runs (twice) (log: $out/preview.log)"
fi
for want in 'href="/pull-requests/42/docs/stable/"' "href='/pull-requests/42/articles/'" \
    'src="/pull-requests/42/_static/x.png"' 'data-pagefind="/pull-requests/42/pagefind/pagefind.js"' \
    'action="/pull-requests/42/search.html"' 'href="//cdn.example.org/x.js"' \
    'href="https://www.jupedsim.org/"' 'href="notes/index.html"' 'href="#top"'; do
    count "$pv/index.html" "$(printf '%s' "$want" | sed 's/[.]/\\./g')" 1 "preview: $want"
done
exactly "$pv/index.html" 'pull-requests/42/pull-requests' 0 "preview: idempotent (no double prefix)"
count "$pv/index.html" 'href=&quot;/not-an-attribute&quot;' 1 "preview: escaped text left alone"
count "$pv/docs/dev/concepts/routing.html" 'data-json="/pull-requests/42/docs/versions\.json"' 1 "preview: switcher JSON under the prefix"
count "$pv/docs/dev/concepts/routing.html" 'href="/pull-requests/42/docs/v1x/v1\.4\.2/"' 1 "preview: docs page nav under the prefix"
count "$pv/docs/versions.json" '"url": "/pull-requests/42/docs/dev/"' 1 "preview: versions.json path urls prefixed"
count "$pv/docs/versions.json" '"url": "https://www\.jupedsim\.org/docs/x/"' 1 "preview: versions.json absolute urls kept"
[ "$(readlink "$pv/docs/stable")" = "dev" ] && ok "preview: docs/stable -> dev" || bad "preview: docs/stable -> dev"
# Pagefind returns result URLs relative to the index's site root; search.js
# derives that root from the pagefind.js URL (production: "/").
count "$theme_dir/jupedsim_book/static/search.js" 'baseUrl' 1 "search.js sets Pagefind's baseUrl from the pagefind.js URL"

echo
echo "$pass ok / $fail fail"
if [ -n "$keep" ]; then echo "outputs kept in $out"; fi
if [ "$fail" -ne 0 ]; then
    echo "CHECK FAILED"
    exit 1
fi
echo "CHECK PASSED"
