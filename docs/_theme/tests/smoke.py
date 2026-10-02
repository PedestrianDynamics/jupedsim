# SPDX-License-Identifier: LGPL-3.0-or-later
"""Browser smoke checks for a served site build (optional, needs Playwright).

Exercises what check.sh cannot see in static HTML: the top bar (layout,
stickiness, grid alignment, the "/" shortcut, search results above it,
anchors below it), row lists (whole row clickable), the landing hero
(layout, dark colours, links), the accent budget, the
footer's grid alignment and visibility on short pages, the version switcher, Pagefind search, the theme toggle,
the mobile drawers, print styles, JS console errors and external requests. Build and serve the site first
(`ninja site`, then `python -m http.server -d <build>/site 8000`), then:

    python -m pip install playwright
    python -m playwright install chromium
    python docs/_theme/tests/smoke.py http://localhost:8000 --shots /tmp/shots

Prints one ok/FAIL line per check and ends with "SMOKE <n> ok / <m> fail"
(exit 1 on any failure). Screenshots (light/dark at 390, 1024, 1440 and
1920 px) go to --shots for a human look.
"""

import argparse
import json
import os
import sys
import tempfile
import urllib.request

try:
    from playwright.sync_api import sync_playwright
except ImportError:
    sys.exit("smoke.py: skipped, playwright is not installed")

parser = argparse.ArgumentParser(
    description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
)
parser.add_argument("base_url", help="URL the site build is served at")
parser.add_argument(
    "--api",
    default="/docs/stable/api/jupedsim/index.html",
    help="API reference page to check (default: %(default)s)",
)
parser.add_argument(
    "--shots",
    default=None,
    help="directory for screenshots (default: a new temp dir)",
)
args = parser.parse_args()
BASE = args.base_url.rstrip("/")
API = args.api
NOTEBOOK = "/docs/stable/notebooks/getting_started.html"
ARTICLE = "/articles/writing-guide.html"
CONCEPT = "/docs/stable/concepts/routing.html"
SHOTS = args.shots or tempfile.mkdtemp(prefix="jupedsim-smoke-")
os.makedirs(SHOTS, exist_ok=True)
results = []


def check(name, cond, detail=""):
    results.append(bool(cond))
    print(
        ("ok   " if cond else "FAIL ")
        + name
        + (f" — {detail}" if detail else ""),
        flush=True,
    )


def guarded(name, fn):
    try:
        fn()
    except Exception as e:  # noqa: BLE001
        check(name, False, f"exception: {e!r}"[:300])


def http():
    for path in [
        "/",
        "/articles/",
        "/notes/",
        "/notes/feed.xml",
        "/docs/stable/",
        "/docs/versions.json",
        "/pagefind/pagefind.js",
        NOTEBOOK,
        API,
    ]:
        try:
            code = urllib.request.urlopen(BASE + path).status
        except Exception as e:  # noqa: BLE001
            code = repr(e)
        check(f"HTTP 200 {path}", code == 200, str(code))


def lum(rgb):
    nums = [
        float(x) for x in rgb[rgb.index("(") + 1 : rgb.index(")")].split(",")
    ]
    alpha = nums[3] if len(nums) > 3 else 1
    return (0.2126 * nums[0] + 0.7152 * nums[1] + 0.0722 * nums[2]) / 255, alpha


def bar_nav(browser):
    page = browser.new_page(viewport={"width": 1280, "height": 900})
    page.goto(BASE + "/")
    links = page.eval_on_selector_all(
        ".site-nav a",
        "as => as.map(a => [a.textContent.trim(), a.getAttribute('href')])",
    )
    want = [
        ["[Articles]", "/articles/"],
        ["[Notes]", "/notes/"],
        ["[Docs]", "/docs/stable/"],
        ["[Docs v1\u00a0↗]", "/docs/v1x/v1.4.2/"],
    ]
    check("/ bar nav: four bracketed items", links == want, str(links))
    shown = page.eval_on_selector(
        ".site-nav a", "a => getComputedStyle(a).textTransform"
    )
    check("/ bar labels rendered uppercase", shown == "uppercase", shown)
    check(
        "/ bar nav: no active item",
        page.locator(".site-nav a[aria-current]").count() == 0,
    )
    for path, href in (
        (ARTICLE, "/articles/"),
        ("/notes/2026-09-28-new-website.html", "/notes/"),
        ("/docs/stable/", "/docs/stable/"),
        (CONCEPT, "/docs/stable/"),
    ):
        page.goto(BASE + path)
        active = page.eval_on_selector_all(
            ".site-nav a[aria-current=page]",
            "as => as.map(a => a.getAttribute('href'))",
        )
        check(f"{path} bar nav: {href} active", active == [href], str(active))

    page.goto(BASE + "/docs/stable/")
    page.wait_for_selector("#book-version-switcher:not([hidden])", timeout=5000)
    opts = page.eval_on_selector_all(
        "#book-version-switcher option", "os => os.map(o => o.textContent)"
    )
    versions = json.loads(
        urllib.request.urlopen(BASE + "/docs/versions.json").read()
    )
    expected = [str(e.get("name") or e.get("version")) for e in versions]
    check(
        "version select matches versions.json",
        opts == expected,
        f"{opts} vs {expected}",
    )
    check(
        "version select: exactly v2.0.0 plus stable",
        sorted(opts) == ["stable", "v2.0.0"],
        str(opts),
    )
    sel = page.eval_on_selector(
        "#book-version-switcher", "s => s.selectedIndex"
    )
    check(
        "version select has a selected entry on /docs/stable/",
        sel >= 0,
        str(sel),
    )
    check(
        "/docs/stable/ left track has the docs tree",
        page.locator(".book-menu .book-tree li.toctree-l1").count() > 0,
    )
    page.goto(BASE + ARTICLE)
    check(
        "site page: no version select, no docs tree",
        page.locator("#book-version-switcher, .book-tree").count() == 0,
    )
    page.close()


def search_results_on_top(browser):
    for width, height in ((1440, 900), (390, 844)):
        page = browser.new_page(viewport={"width": width, "height": height})
        page.goto(BASE + ARTICLE)
        if width < 896:
            page.click(".site-bar-search-btn")
        inp = page.locator("#book-search-input")
        inp.click()
        inp.type("routing", delay=30)
        page.wait_for_selector(
            "#book-search-results:not([hidden]) li a", timeout=10000
        )
        page.wait_for_timeout(300)
        hit = page.evaluate("""() => {
          const r = document.querySelector('#book-search-results li a').getBoundingClientRect();
          const x = r.left + r.width / 2, y = r.top + r.height / 2;
          const e = document.elementFromPoint(x, y);
          return [Math.round(x), Math.round(y), e ? e.tagName + '.' + e.className : null,
                  !!(e && e.closest('#book-search-results'))];
        }""")
        check(f"search-results-on-top ({width}px)", hit[3], str(hit))
        page.close()


def escape_closes(browser):
    """Escape closes the mobile search row and the drawers from the keyboard;
    opening the search row focuses the field; "/" works from a focused toggle."""
    page = browser.new_page(viewport={"width": 390, "height": 844})
    page.goto(BASE + ARTICLE)
    focused = "() => document.activeElement && document.activeElement.id"
    checked = "(id) => document.getElementById(id).checked"
    page.click(".site-bar-search-btn")
    page.wait_for_timeout(100)
    check(
        "search-row-open-focuses-field",
        page.evaluate(focused) == "book-search-input",
        str(page.evaluate(focused)),
    )
    page.keyboard.type("routing", delay=20)
    page.wait_for_timeout(400)
    page.keyboard.press("Escape")
    first = page.evaluate(
        "() => document.getElementById('book-search-input').value"
    )
    check(
        "escape-clears-text-first",
        first == "" and page.evaluate(checked, "search-control"),
        f"value={first!r} open={page.evaluate(checked, 'search-control')}",
    )
    page.keyboard.press("Escape")
    check(
        "escape-closes-search-row",
        not page.evaluate(checked, "search-control")
        and page.evaluate(focused) == "search-control",
        f"open={page.evaluate(checked, 'search-control')} focus={page.evaluate(focused)}",
    )
    page.keyboard.press("/")
    check(
        "slash-from-focused-toggle",
        page.evaluate(checked, "search-control")
        and page.evaluate(focused) == "book-search-input",
        f"open={page.evaluate(checked, 'search-control')} focus={page.evaluate(focused)}",
    )
    page.keyboard.press("Escape")
    for btn, ctl in (
        (".site-bar-menu", "menu-control"),
        (".site-bar-toc", "toc-control"),
    ):
        page.click(btn)
        page.wait_for_timeout(250)
        opened = page.evaluate(checked, ctl)
        page.keyboard.press("Escape")
        page.wait_for_timeout(250)
        check(
            f"escape-closes-{ctl.split('-')[0]}-drawer",
            opened
            and not page.evaluate(checked, ctl)
            and page.evaluate(focused) == ctl,
            f"opened={opened} open={page.evaluate(checked, ctl)} focus={page.evaluate(focused)}",
        )
    page.close()


def single_hairline(browser):
    """A row list directly under the report header must not add a second rule."""
    page = browser.new_page(viewport={"width": 1440, "height": 900})
    for path in ("/notes/", "/articles/"):
        page.goto(BASE + path)
        gap = page.evaluate("""() => {
          const h = document.querySelector('.report-header').getBoundingClientRect().bottom;
          const ul = document.querySelector('.report-body ul.post-rows');
          const r = ul.getBoundingClientRect();
          const bt = parseFloat(getComputedStyle(ul).borderTopWidth);
          // Visible content between the header rule and the list's top rule?
          const between = [...document.querySelectorAll('.report-body > section > *')]
            .filter(e => e !== ul && getComputedStyle(e).display !== 'none'
                    && e.getBoundingClientRect().bottom <= r.top + 1
                    && e.getBoundingClientRect().height > 0).length;
          return {bt, between};
        }""")
        ok = gap["bt"] == 0 if gap["between"] == 0 else True
        check(f"single-hairline {path}", ok, str(gap))
    page.close()


def visited_contrast(browser):
    """Visited links stay readable. Browsers hide :visited from scripts, so
    simulate it: basic.css has ``a:visited { color: #551A8B }`` (0,1,1);
    insert ``a[href]`` (also 0,1,1) with that colour right after basic.css,
    then every link must keep >= 4.5:1 against its background."""
    sim = """() => {
      const basic = [...document.querySelectorAll('link[rel=stylesheet]')]
        .find(l => /basic\\.css/.test(l.href));
      const st = document.createElement('style');
      st.textContent = 'a[href] { color: #551A8B; }';
      basic.after(st);
    }"""
    measure = r"""() => {
      const rgb = c => c.match(/[\d.]+/g).map(Number);
      const lum = ([r, g, b]) => { const f = v => { v /= 255; return v <= 0.03928 ? v / 12.92 : Math.pow((v + 0.055) / 1.055, 2.4); };
        return 0.2126 * f(r) + 0.7152 * f(g) + 0.0722 * f(b); };
      const bg = el => { for (let e = el; e; e = e.parentElement) { const c = rgb(getComputedStyle(e).backgroundColor);
        if (c.length < 4 || c[3] > 0) return c; } return rgb(getComputedStyle(document.body).backgroundColor); };
      const bad = [];
      for (const a of document.querySelectorAll('a[href]')) {
        if (!a.offsetParent || !a.textContent.trim()) continue;
        const f = lum(rgb(getComputedStyle(a).color)), b = lum(bg(a));
        const cr = (Math.max(f, b) + 0.05) / (Math.min(f, b) + 0.05);
        if (cr < 4.5) bad.push(`${a.className || a.parentElement.className}:${a.textContent.trim().slice(0, 20)}:${cr.toFixed(1)}`);
      }
      return bad;
    }"""
    for scheme in ("light", "dark"):
        ctx = browser.new_context(
            viewport={"width": 1440, "height": 900}, color_scheme=scheme
        )
        page = ctx.new_page()
        for path in ("/", ARTICLE, "/notes/", CONCEPT):
            page.goto(BASE + path)
            page.evaluate(sim)
            bad = page.evaluate(measure)
            check(
                f"visited-contrast {scheme} {path}", not bad, "; ".join(bad[:6])
            )
        ctx.close()


def anchor_below_bar(browser):
    targets = [CONCEPT + "#stages"]
    probe = browser.new_page()
    probe.goto(BASE + API)
    sig = probe.evaluate(
        "(document.querySelector('.report-body dt.sig[id]') || {}).id || ''"
    )
    probe.close()
    if sig:
        targets.append(API + "#" + sig)
    for width, height in ((1440, 900), (390, 844)):
        for target in targets:
            page = browser.new_page(viewport={"width": width, "height": height})
            page.goto(BASE + target)
            page.wait_for_load_state("networkidle")
            page.wait_for_timeout(300)
            res = page.evaluate("""() => {
              const t = document.querySelector(':target');
              if (!t) return null;
              const h = t.matches('h1,h2,h3,h4,h5,h6,dt') ? t : t.querySelector('h1,h2,h3,h4,h5,h6') || t;
              return [Math.round(h.getBoundingClientRect().top),
                      Math.round(document.querySelector('.site-bar').getBoundingClientRect().bottom),
                      Math.round(scrollY)];
            }""")
            check(
                f"anchor-below-bar ({width}px, {target})",
                res is not None and res[2] > 0 and res[0] >= res[1],
                f"heading top / bar bottom / scrollY = {res}",
            )
            page.close()


def slash_shortcut(browser):
    for width, height in ((1440, 900), (390, 844)):
        page = browser.new_page(viewport={"width": width, "height": height})
        page.goto(BASE + ARTICLE)
        page.evaluate("document.activeElement && document.activeElement.blur()")
        page.keyboard.press("/")
        page.wait_for_timeout(100)
        res = page.evaluate(
            "[document.activeElement.id, document.getElementById('book-search-input').value]"
        )
        check(
            f"slash-shortcut: / focuses the bar search ({width}px)",
            res == ["book-search-input", ""],
            str(res),
        )
        page.close()
    page = browser.new_page(viewport={"width": 1440, "height": 900})
    page.goto(BASE + "/docs/stable/search.html?q=x")
    field = page.locator(".report-body input[name=q]")
    field.fill("")
    field.click()
    page.keyboard.type("a/b")
    res = page.evaluate(
        "[document.activeElement === document.querySelector('.report-body input[name=q]'),"
        " document.querySelector('.report-body input[name=q]').value]"
    )
    check(
        "slash-shortcut: search page field keeps a/b and focus",
        res == [True, "a/b"],
        str(res),
    )
    page.goto(BASE + CONCEPT)
    page.wait_for_selector("#book-version-switcher:not([hidden])", timeout=5000)
    page.focus("#book-version-switcher")
    page.keyboard.press("/")
    page.wait_for_timeout(100)
    check(
        "slash-shortcut: / on the version select keeps focus there",
        page.evaluate("document.activeElement.id") == "book-version-switcher"
        and page.url.startswith(BASE + CONCEPT),
        page.evaluate("document.activeElement.id"),
    )
    page.evaluate("""() => { const t = document.createElement('textarea');
      t.id = 'smoke-textarea'; document.querySelector('.report-body').prepend(t); t.focus(); }""")
    page.keyboard.type("a/b")
    res = page.evaluate(
        "[document.activeElement.id, document.getElementById('smoke-textarea').value]"
    )
    check(
        "slash-shortcut: textarea keeps a/b and focus",
        res == ["smoke-textarea", "a/b"],
        str(res),
    )
    page.close()


def bar_one_line(browser):
    for width in (900, 1024, 1100, 1440):
        page = browser.new_page(viewport={"width": width, "height": 900})
        for path in ("/", "/docs/stable/"):
            page.goto(BASE + path)
            res = page.evaluate("""() => {
              const rem = parseFloat(getComputedStyle(document.documentElement).fontSize);
              const h = document.querySelector('.site-bar').getBoundingClientRect().height;
              const tops = new Set([...document.querySelectorAll('.site-nav li')].map(l => l.offsetTop));
              return {h, max: 3.5 * rem + 1, rows: tops.size,
                      sw: document.documentElement.scrollWidth, iw: innerWidth};
            }""")
            check(
                f"bar-one-line ({width}px, {path})",
                res["h"] <= res["max"]
                and res["rows"] == 1
                and res["sw"] <= res["iw"],
                str(res),
            )
        page.close()


def bar_sticky(browser):
    page = browser.new_page(viewport={"width": 1440, "height": 900})
    page.goto(BASE + ARTICLE)
    page.evaluate("window.scrollTo(0, 2000)")
    page.wait_for_timeout(200)
    res = page.evaluate(
        "[Math.round(scrollY), document.querySelector('.site-bar').getBoundingClientRect().top]"
    )
    check(
        "bar-sticky: bar at top after scrolling 2000px",
        res[0] > 0 and res[1] == 0,
        str(res),
    )
    page.close()


def same_column(browser):
    page = browser.new_page(viewport={"width": 1440, "height": 900})
    lefts = []
    for path in (ARTICLE, CONCEPT):
        page.goto(BASE + path)
        lefts.append(
            page.evaluate(
                "document.querySelector('.report-body').getBoundingClientRect().left"
            )
        )
    check(
        "same-column: text column left edge on article = docs (1440px)",
        abs(lefts[0] - lefts[1]) <= 1,
        str(lefts),
    )
    page.close()


def rows_clickable(browser):
    """Each row is one link: its title and the empty space right of it
    (before the arrow) both navigate to the entry."""
    for width in (1440, 390):
        page = browser.new_page(viewport={"width": width, "height": 900})
        for spot in ("title", "right side"):
            page.goto(BASE + "/notes/")
            link = page.locator("a.post-row-link").first
            href = link.evaluate("a => a.href")
            # "right side": the gap between the title's grid cell and the
            # arrow, which holds no text.
            pt = link.evaluate(
                """(a, spot) => {
              const t = a.querySelector('.post-row-title').getBoundingClientRect();
              const r = a.querySelector('.post-row-arrow').getBoundingClientRect();
              const x = spot === 'title' ? t.left + Math.min(20, t.width / 2)
                                         : (t.right + r.left) / 2;
              return {x, y: t.top + t.height / 2, gap: r.left - t.right};
            }""",
                spot,
            )
            hit = page.evaluate(
                "([x, y]) => !!document.elementFromPoint(x, y)"
                "?.closest('a.post-row-link')",
                [pt["x"], pt["y"]],
            )
            if hit:
                with page.expect_navigation(timeout=5000):
                    page.mouse.click(pt["x"], pt["y"])
            check(
                f"rows-clickable ({width}px, {spot})",
                page.url == href,
                f"{page.url} vs {href}, {pt}",
            )
        page.close()


# Running-text links only: row lists, post lists and buttons inside the body
# are restrained to ink and must not hide behind the ".report-body a" pass.
ACCENT_OK = (
    ".report-body a:not(.post-row-link, .post-list-title, .btn, .btn-solid,"
    " .rows a), .report-body .admonition, .report-body .callout,"
    " .site-nav [aria-current], .site-nav-drawer [aria-current],"
    " .book-tree a.current"
)


def accent_budget(browser):
    """In the light theme, text in the accent colours (--accent,
    --accent-ink) only appears in running-text links, callouts, the bar's
    active item, the current tree item and focused elements."""
    ctx = browser.new_context(
        viewport={"width": 1440, "height": 900}, color_scheme="light"
    )
    page = ctx.new_page()
    for path in (ARTICLE, "/", "/articles/", "/notes/", CONCEPT):
        page.goto(BASE + path)
        bad = page.evaluate(
            """ok => {
              const probe = v => { const s = document.createElement('span');
                s.style.color = `var(${v})`; document.body.append(s);
                const c = getComputedStyle(s).color; s.remove(); return c; };
              const accents = new Set([probe('--accent'), probe('--accent-ink')]);
              const name = e => e.tagName.toLowerCase() + (e.id ? '#' + e.id : '')
                + [...e.classList].map(c => '.' + c).join('');
              const out = new Set();
              for (const e of document.querySelectorAll('body *')) {
                const own = [...e.childNodes].some(n => n.nodeType === 3 && n.textContent.trim());
                if (!own || !e.getClientRects().length) continue;
                if (!accents.has(getComputedStyle(e).color)) continue;
                if (e.closest(ok) || e.matches(':focus-visible')) continue;
                const p = e.parentElement;
                out.add((p ? name(p) + ' > ' : '') + name(e));
              }
              return [...out];
            }""",
            ACCENT_OK,
        )
        check(f"accent-budget ({path})", not bad, str(bad[:10]))
    ctx.close()


def footer_aligned(browser):
    """The footer text starts at the brand mark's left edge (1440px)."""
    page = browser.new_page(viewport={"width": 1440, "height": 900})
    for path in ("/", ARTICLE, CONCEPT):
        page.goto(BASE + path)
        res = page.evaluate("""() => {
          const logo = [...document.querySelectorAll('.site-bar .book-brand > *')]
            .find(e => e.getClientRects().length);
          const text = document.querySelector('.site-footer-text');
          return [logo.getBoundingClientRect().left, text.getBoundingClientRect().left];
        }""")
        check(
            f"footer-aligned ({path}, 1440px)",
            abs(res[0] - res[1]) <= 1,
            str(res),
        )
    page.close()


def footer_in_view(browser):
    """On a page shorter than the screen the footer is visible without
    scrolling and the page does not scroll."""
    for width, height in ((1440, 900), (390, 844)):
        page = browser.new_page(viewport={"width": width, "height": height})
        for path in ("/404.html", "/docs/stable/404.html"):
            page.goto(BASE + path)
            res = page.evaluate("""() => [
              document.documentElement.scrollHeight, innerHeight,
              document.querySelector('.site-footer').getBoundingClientRect().bottom]""")
            check(
                f"footer-in-view ({path}, {width}x{height})",
                res[0] <= res[1] and res[2] <= res[1] + 1,
                f"scrollHeight, innerHeight, footer bottom = {res}",
            )
        page.close()


def keyboard_order(browser):
    page = browser.new_page(viewport={"width": 1440, "height": 900})
    page.goto(BASE + ARTICLE)
    seq = []
    for _ in range(8):
        page.keyboard.press("Tab")
        seq.append(
            page.evaluate("""() => { const e = document.activeElement;
              if (e.closest('.book-brand')) return 'mark';
              if (e.closest('.site-nav')) return 'nav';
              if (e.id === 'book-search-input') return 'search';
              if (e.id === 'book-theme-toggle') return 'toggle';
              if (e.closest('.book-page')) return 'content';
              return e.tagName + '#' + e.id + '.' + e.className; }""")
        )
    want = ["mark", "nav", "nav", "nav", "nav", "search", "toggle", "content"]
    check(
        "keyboard order: mark, nav, search, toggle, content",
        seq == want,
        str(seq),
    )
    page.close()


def unique_ids(browser):
    page = browser.new_page()
    dups = []
    for path in ("/", ARTICLE, "/docs/stable/", CONCEPT, API):
        page.goto(BASE + path)
        d = page.evaluate("""() => { const seen = new Set(), out = [];
          for (const e of document.querySelectorAll('[id]')) { if (seen.has(e.id)) out.push(e.id); seen.add(e.id); }
          return out; }""")
        dups += [f"{path}#{i}" for i in d]
    check("no duplicate ids", not dups, str(dups[:10]))
    page.close()


def hero_layout(browser):
    """Landing hero: text and figure side by side at 1440px, the figure
    below the text at 390px; no horizontal overflow at either width. The
    figure's SVG is flush with the hero's right edge (1440px) and with the
    text's left edge (390px), i.e. not inset by basic.css figure padding."""
    for width in (1440, 390):
        page = browser.new_page(viewport={"width": width, "height": 900})
        page.goto(BASE + "/")
        res = page.evaluate("""() => {
          const r = (s) => document.querySelector(s).getBoundingClientRect();
          const h = r('.hero'), t = r('.hero-text'), f = r('.hero-figure'),
                s = r('.hero-figure svg');
          return {hr: h.right, tl: t.left, tr: t.right, tb: t.bottom,
                  fl: f.left, ft: f.top, fw: f.width, sl: s.left, sr: s.right,
                  sw: document.documentElement.scrollWidth, iw: innerWidth};
        }""")
        if width == 1440:
            placed = (
                res["fl"] > res["tr"]
                and res["fw"] > 0
                and abs(res["sr"] - res["hr"]) < 1
            )
            what = "figure right of the text, flush right"
        else:
            placed = (
                res["ft"] >= res["tb"]
                and res["fw"] > 0
                and abs(res["sl"] - res["tl"]) < 1
            )
            what = "figure below the text, flush left"
        check(
            f"hero-layout ({width}px): {what}, no overflow",
            placed and res["sw"] <= res["iw"],
            str(res),
        )
        page.close()


def hero_dark(browser):
    """Dark scheme: the alert agent is #e04b30 and the walls use --ink."""
    ctx = browser.new_context(
        viewport={"width": 1440, "height": 900}, color_scheme="dark"
    )
    page = ctx.new_page()
    page.goto(BASE + "/")
    res = page.evaluate("""() => {
      const probe = document.createElement('span');
      probe.style.color = 'var(--ink)';
      document.body.append(probe);
      const ink = getComputedStyle(probe).color;
      probe.remove();
      return {theme: document.documentElement.dataset.theme,
              alert: getComputedStyle(document.querySelector('.hf-alert')).fill,
              wall: getComputedStyle(document.querySelector('.hf-wall')).fill,
              ink};
    }""")
    check(
        "hero-dark: .hf-alert fill rgb(224, 75, 48), .hf-wall fill = --ink",
        res["theme"] == "dark"
        and res["alert"] == "rgb(224, 75, 48)"
        and res["wall"] == res["ink"],
        str(res),
    )
    ctx.close()


CONTRAST_JS = """named => {
  const lin = c => { c /= 255; return c <= 0.03928 ? c / 12.92 : ((c + 0.055) / 1.055) ** 2.4; };
  const rgba = s => s.match(/[\\d.]+/g).map(Number);
  const L = s => { const [r, g, b] = rgba(s); return 0.2126 * lin(r) + 0.7152 * lin(g) + 0.0722 * lin(b); };
  const ratio = (a, b) => { const [x, y] = [L(a), L(b)].sort((m, n) => n - m); return (x + 0.05) / (y + 0.05); };
  const bg = e => { for (; e; e = e.parentElement) { const c = getComputedStyle(e).backgroundColor;
    const v = rgba(c); if (v.length < 4 || v[3] === 1) return c; }
    return getComputedStyle(document.body).backgroundColor; };
  const probe = document.createElement('span');
  probe.style.color = 'var(--ink-faint)'; document.body.append(probe);
  const faint = getComputedStyle(probe).color; probe.remove();
  const name = e => e.tagName.toLowerCase() + [...e.classList].map(c => '.' + c).join('');
  const out = {};
  for (const e of document.querySelectorAll('body *')) {
    const own = [...e.childNodes].some(n => n.nodeType === 3 && n.textContent.trim());
    if (!own || !e.getClientRects().length || getComputedStyle(e).color !== faint) continue;
    out[name(e)] = Math.min(out[name(e)] ?? 99, ratio(faint, bg(e)));
  }
  for (const sel of named) {
    const e = document.querySelector(sel);
    if (e) out[sel] = ratio(getComputedStyle(e).color, bg(e));
  }
  const inp = document.querySelector('#book-search-input');
  if (inp) out['#book-search-input::placeholder'] =
    ratio(getComputedStyle(inp, '::placeholder').color, bg(inp));
  return out;
}"""


def muted_contrast(browser):
    """Muted (--ink-faint) text reaches 4.5:1 against its background in the
    light and the dark scheme: hero caption, TOC title, prev/next labels,
    footer meta, search placeholder, results and excerpts."""
    for scheme in ("light", "dark"):
        ctx = browser.new_context(
            viewport={"width": 1440, "height": 900}, color_scheme=scheme
        )
        page = ctx.new_page()
        seen = {}
        for path, named in (
            ("/", [".hero-figure figcaption"]),
            (ARTICLE, [".book-toc-title", ".book-pagenav small"]),
            ("/notes/", []),
            (CONCEPT, [".book-toc-title", ".book-pagenav small"]),
        ):
            page.goto(BASE + path)
            if path == CONCEPT:
                inp = page.locator("#book-search-input")
                inp.click()
                inp.type("routing", delay=30)
                page.wait_for_selector(
                    "#book-search-results:not([hidden]) li a", timeout=10000
                )
                page.wait_for_timeout(300)
            for k, v in page.evaluate(CONTRAST_JS, named).items():
                seen[k] = min(seen.get(k, 99), v)
        must = {
            ".hero-figure figcaption",
            ".book-toc-title",
            ".book-pagenav small",
            "#book-search-input::placeholder",
            "span.book-search-excerpt",
        }
        low = {k: round(v, 2) for k, v in seen.items() if v < 4.5}
        check(
            f"muted-contrast ({scheme}): --ink-faint text >= 4.5:1",
            not low and must <= seen.keys(),
            f"low={low} missing={sorted(must - seen.keys())}",
        )
        ctx.close()


def toc_wraps(browser):
    """Long monospace API names wrap in the right TOC instead of making the
    column scroll sideways."""
    page = browser.new_page(viewport={"width": 1440, "height": 900})
    page.goto(BASE + API)
    res = page.evaluate("""() => {
      const t = document.querySelector('.book-toc');
      return t && {sw: t.scrollWidth, cw: t.clientWidth};
    }""")
    check(
        "toc-wraps (API reference, 1440px)",
        res and res["sw"] <= res["cw"],
        str(res),
    )
    page.close()


def landing_links(browser):
    """Every [DOCS] row and hero button answers 200 on the local server.
    Skipped: off-site links (GitHub) and the 1.x archive (/docs/v1x/), which
    only exists on the deployed gh-pages branch."""
    page = browser.new_page(viewport={"width": 1440, "height": 900})
    page.goto(BASE + "/")
    hrefs = page.eval_on_selector_all(
        ".landing-body .rows a, .hero-ctas a",
        "as => as.map(a => a.getAttribute('href'))",
    )
    page.close()
    local = [h for h in hrefs if h.startswith("/") and "/docs/v1x/" not in h]
    skipped = [h for h in hrefs if h not in local]
    check("landing-links: rows and buttons found", len(local) >= 5, str(hrefs))
    for href in local:
        try:
            code = urllib.request.urlopen(BASE + href).status
        except Exception as e:  # noqa: BLE001
            code = repr(e)
        check(f"landing-links: HTTP 200 {href}", code == 200, str(code))
    print("info landing-links skipped: " + ", ".join(skipped))


def search_fallback(browser):
    ctx = browser.new_context(viewport={"width": 1440, "height": 900})
    ctx.route("**/pagefind/**", lambda route: route.abort())
    page = ctx.new_page()
    page.goto(BASE + ARTICLE)
    inp = page.locator("#book-search-input")
    inp.click()
    inp.type("routing", delay=20)
    page.wait_for_timeout(300)
    inp.press("Enter")
    page.wait_for_load_state("load")
    check(
        "search without Pagefind submits to Sphinx search.html",
        "/search.html?q=routing" in page.url,
        page.url,
    )
    ctx.close()


def search(browser):
    page = browser.new_page(viewport={"width": 1280, "height": 900})
    page.goto(BASE + "/")
    for word, prefix in (
        ("triangulates", "/docs/stable/"),
        ("intersphinx", "/notes/"),
    ):
        inp = page.locator("#book-search-input")
        inp.fill("")
        inp.click()
        inp.type(word, delay=30)
        page.wait_for_selector(
            "#book-search-results:not([hidden]) li a", timeout=10000
        )
        page.wait_for_timeout(500)
        hrefs = page.eval_on_selector_all(
            "#book-search-results a",
            "as => as.map(a => a.getAttribute('href'))",
        )
        check(
            f"search '{word}' returns {prefix}…",
            any(h.startswith(prefix) for h in hrefs),
            str(hrefs[:5]),
        )
    page.close()


def theme(browser):
    ctx = browser.new_context(
        viewport={"width": 1280, "height": 900}, color_scheme="light"
    )
    ctx.add_init_script("""
      new MutationObserver((ms, obs) => {
        if (document.body) { window.__firstTheme = document.documentElement.dataset.theme || null; obs.disconnect(); }
      }).observe(document, {childList: true, subtree: true});
    """)
    page = ctx.new_page()
    page.goto(BASE + ARTICLE)
    btn = page.locator("#book-theme-toggle")
    seq = [
        (
            btn.text_content(),
            page.evaluate("document.documentElement.dataset.theme"),
        )
    ]
    for _ in range(3):
        btn.click()
        seq.append(
            (
                btn.text_content(),
                page.evaluate("document.documentElement.dataset.theme"),
            )
        )
    want = [
        ("Theme: auto", "light"),
        ("Theme: light", "light"),
        ("Theme: dark", "dark"),
        ("Theme: auto", "light"),
    ]
    check(
        "theme toggle cycles auto -> light -> dark -> auto",
        seq == want,
        str(seq),
    )
    btn.click()
    btn.click()  # -> dark
    page.reload()
    first = page.evaluate("window.__firstTheme")
    check(
        "theme persists after reload (dark)",
        page.evaluate("document.documentElement.dataset.theme") == "dark"
        and btn.text_content() == "Theme: dark",
        f"{page.evaluate('document.documentElement.dataset.theme')} / {btn.text_content()}",
    )
    check(
        "first paint has data-theme=dark before body",
        first == "dark",
        str(first),
    )
    page.goto(BASE + "/docs/stable/")
    check(
        "theme persists across site -> docs",
        page.evaluate("window.__firstTheme") == "dark",
        str(page.evaluate("window.__firstTheme")),
    )
    ctx.close()


def visible(page, sel):
    return page.eval_on_selector(
        sel,
        "e => getComputedStyle(e).visibility === 'visible' && e.getBoundingClientRect().right > 0 && e.getBoundingClientRect().left < innerWidth",
    )


def mobile(browser):
    ctx = browser.new_context(
        viewport={"width": 390, "height": 844}, has_touch=False
    )
    page = ctx.new_page()
    page.goto(BASE + CONCEPT)
    check("drawers-390: menu closed initially", not visible(page, ".book-menu"))
    page.click(".site-bar-menu")
    page.wait_for_timeout(400)
    check(
        "drawers-390: menu opens via the bar button, with nav + docs tree",
        visible(page, ".book-menu")
        and visible(page, ".site-nav-drawer")
        and visible(page, ".book-menu .book-tree"),
    )
    page.mouse.click(380, 600)  # overlay area right of the drawer
    page.wait_for_timeout(400)
    check(
        "drawers-390: menu closes via overlay", not visible(page, ".book-menu")
    )
    page.click(".site-bar-menu")
    page.wait_for_timeout(400)
    page.click(".site-bar-menu")
    page.wait_for_timeout(400)
    check(
        "drawers-390: menu closes via the bar button",
        not visible(page, ".book-menu"),
    )
    page.click(".site-bar-toc")
    page.wait_for_timeout(400)
    check(
        "drawers-390: TOC opens via the bar button", visible(page, ".book-toc")
    )
    page.mouse.click(10, 600)
    page.wait_for_timeout(400)
    check("drawers-390: TOC closes via overlay", not visible(page, ".book-toc"))
    page.click(".site-bar-search-btn")
    page.wait_for_timeout(200)
    check(
        "drawers-390: search button reveals the search row",
        page.is_visible("#book-search-input"),
    )
    page.goto(BASE + ARTICLE)
    page.click(".site-bar-menu")
    page.wait_for_timeout(400)
    check(
        "drawers-390: site menu shows the nav, no docs tree",
        visible(page, ".site-nav-drawer")
        and page.locator(".book-menu .book-tree").count() == 0,
    )
    page.goto(BASE + CONCEPT)

    # Keyboard: with drawers closed, Tab never lands on an invisible element.
    page.reload()
    page.keyboard.press("Tab")
    bad = []
    for _ in range(12):
        info = page.evaluate("""() => { const e = document.activeElement; const r = e.getBoundingClientRect();
          const cs = getComputedStyle(e);
          return [e.tagName, e.id || e.className || e.textContent.trim().slice(0, 20),
                  e.closest('.book-menu, .book-toc') ? getComputedStyle(e.closest('.book-menu, .book-toc')).visibility : 'n/a',
                  cs.visibility]; }""")
        if info[2] == "hidden" or info[3] == "hidden":
            bad.append(info)
        page.keyboard.press("Tab")
    check("mobile: Tab skips closed drawers", not bad, str(bad))
    # Keyboard open: focus the menu checkbox, Space opens the drawer, Tab reaches its links.
    page.reload()
    page.keyboard.press("Tab")
    first = page.evaluate("document.activeElement.id")
    page.keyboard.press("Space")
    page.wait_for_timeout(400)
    opened = visible(page, ".book-menu")
    inside = False
    for _ in range(6):
        page.keyboard.press("Tab")
        if page.evaluate("!!document.activeElement.closest('.book-menu')"):
            inside = True
            break
    check(
        "mobile: keyboard opens menu and Tab enters it",
        first == "menu-control" and opened and inside,
        f"first={first} opened={opened} inside={inside}",
    )
    ring = page.evaluate(
        "getComputedStyle(document.activeElement).outlineStyle"
    )
    check(
        "mobile: focused menu link has a visible outline",
        ring not in ("none", ""),
        ring,
    )
    ctx.close()


def printing(browser):
    ctx = browser.new_context(
        viewport={"width": 1280, "height": 900}, color_scheme="dark"
    )
    ctx.add_init_script("try{localStorage.setItem('theme','dark')}catch(e){}")
    page = ctx.new_page()
    page.goto(BASE + ARTICLE)
    page.emulate_media(media="print", color_scheme="dark")
    bg = page.evaluate("getComputedStyle(document.body).backgroundColor")
    html_bg = page.evaluate(
        "getComputedStyle(document.documentElement).backgroundColor"
    )
    ink = page.evaluate(
        "getComputedStyle(document.querySelector('.report-body p') || document.body).color"
    )
    lb, ab = lum(bg)
    lh, ah = lum(html_bg)
    light = (lb > 0.85 and ab > 0) or (ab == 0 and (ah == 0 or lh > 0.85))
    check(
        "print (dark scheme, theme=dark): body background light",
        light,
        f"body={bg} html={html_bg}",
    )
    check("print: text dark", lum(ink)[0] < 0.4, ink)
    check(
        "print: no top bar",
        page.eval_on_selector(".site-bar", "e => getComputedStyle(e).display")
        == "none",
    )
    page.pdf(path=f"{SHOTS}/article-print.pdf")
    ctx.close()


def shots(browser):
    pages = {
        "home": "/",
        "article": ARTICLE,
        "notes": "/notes/",
        "articles": "/articles/",
        "concept": CONCEPT,
        "api": API,
    }
    widths = (390, 1024, 1440, 1920)
    n = 0
    for scheme in ("light", "dark"):
        for width in widths:
            ctx = browser.new_context(
                viewport={"width": width, "height": 900}, color_scheme=scheme
            )
            page = ctx.new_page()
            for name, path in pages.items():
                page.goto(BASE + path)
                page.wait_for_load_state("networkidle")
                page.screenshot(
                    path=f"{SHOTS}/{name}-{width}-{scheme}.png", full_page=False
                )
                n += 1
            ctx.close()
    want = 2 * len(widths) * len(pages)
    check("screenshots written", n == want, f"{n} of {want}")


with sync_playwright() as p:
    browser = p.chromium.launch()
    errors = []
    guarded("http", http)
    for fn in (
        bar_nav,
        search,
        search_results_on_top,
        search_fallback,
        anchor_below_bar,
        slash_shortcut,
        escape_closes,
        single_hairline,
        visited_contrast,
        bar_one_line,
        bar_sticky,
        same_column,
        rows_clickable,
        hero_layout,
        hero_dark,
        muted_contrast,
        toc_wraps,
        landing_links,
        accent_budget,
        footer_aligned,
        footer_in_view,
        keyboard_order,
        unique_ids,
        theme,
        mobile,
        printing,
        shots,
    ):
        guarded(fn.__name__, lambda fn=fn: fn(browser))
    # Console errors on a few pages; external requests: site pages must make
    # none, docs pages are reported only (their CDN scripts are out of scope,
    # design.md -> Context).
    ctx = browser.new_context()
    page = ctx.new_page()
    ext = []
    page.on(
        "console",
        lambda m: (
            errors.append(f"{page.url}: {m.text}")
            if m.type == "error"
            else None
        ),
    )
    page.on("pageerror", lambda e: errors.append(f"{page.url}: {e}"))
    page.on(
        "request",
        lambda r: (
            ext.append((page.url, r.url))
            if not r.url.startswith((BASE, "data:"))
            else None
        ),
    )
    for path in (
        "/",
        ARTICLE,
        "/notes/",
        "/notes/2026-09-28-new-website.html",
        "/articles/",
        "/docs/stable/",
        NOTEBOOK,
        API,
    ):
        page.goto(BASE + path)
        page.wait_for_load_state("networkidle")
    check("no JS console errors", not errors, str(errors[:5]))
    site_ext = sorted({u for p_, u in ext if "/docs/" not in p_})
    docs_ext = sorted({u.split("?")[0] for p_, u in ext if "/docs/" in p_})
    check("site pages make no external requests", not site_ext, str(site_ext))
    print(
        "info docs pages external requests (known, out of scope): "
        + ", ".join(docs_ext)
    )
    ctx.close()
    ctx = browser.new_context(viewport={"width": 1280, "height": 900})
    page = ctx.new_page()
    for name, path in (("notebook", NOTEBOOK), ("api", API)):
        page.goto(BASE + path)
        page.wait_for_load_state("networkidle")
        page.screenshot(
            path=f"{SHOTS}/{name}-desktop-light-full.png", full_page=True
        )
    ctx.close()
    browser.close()

print(f"screenshots in {SHOTS}")
print(f"SMOKE {sum(results)} ok / {len(results) - sum(results)} fail")
sys.exit(0 if all(results) else 1)
