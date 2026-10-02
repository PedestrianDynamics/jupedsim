# SPDX-License-Identifier: LGPL-3.0-or-later
"""Sphinx theme/extension ``jupedsim_book``: page-context keys for the templates."""

import ast
import datetime
import functools
import json
import math
import os
import re
import xml.etree.ElementTree as ET

from docutils import nodes
from sphinx.util import logging

logger = logging.getLogger(__name__)

_SKIP_TOC_PAGES = {"search", "genindex", "py-modindex"}
_HIDDEN_NODES = (nodes.raw, nodes.comment, nodes.system_message)


def _parse(value):
    """Parse a string option (JSON or Python literal); return other values as is."""
    if not isinstance(value, str):
        return value
    value = value.strip()
    if not value:
        return None
    for parser in (json.loads, ast.literal_eval):
        try:
            return parser(value)
        except Exception:
            pass
    return value


_NAV_KEYS = ("title", "url", "section", "prefix", "external")


@functools.cache
def _warn_nav_item(entry, option="nav_items"):
    """Warn once per build about an unusable ``nav_items`` entry."""
    logger.warning(
        "jupedsim_book: %s entry %s has no title/url; skipped", option, entry
    )


def _as_bool(value):
    if isinstance(value, str):
        return value.strip().lower() in ("1", "true", "yes", "on")
    return bool(value)


def _nav_items(value, option="nav_items"):
    """Normalise ``nav_items`` or ``footer_links`` to a list of dicts.

    Accepts a list of dicts (or ``(title, url)`` pairs), a single dict, or
    their JSON/Python-literal string form. Keeps ``title``, ``url``,
    ``section``, ``prefix`` (strings) and ``external`` (bool); other keys
    are dropped. Entries without ``title``/``url`` are skipped with a warning.
    """
    value = _parse(value)
    if not value:
        return []
    if isinstance(value, dict):
        value = [value]
    if not isinstance(value, (list, tuple)):
        _warn_nav_item(repr(value), option)
        return []
    items = []
    for entry in value:
        if isinstance(entry, dict):
            item = {k: entry[k] for k in _NAV_KEYS if entry.get(k) is not None}
        elif isinstance(entry, (list, tuple)) and len(entry) == 2:
            item = {"title": entry[0], "url": entry[1]}
        else:
            item = {}
        for key in ("title", "url", "section", "prefix"):
            if key in item:
                item[key] = str(item[key]).strip()
        if not item.get("title") or not item.get("url"):
            _warn_nav_item(repr(entry), option)
            continue
        item["external"] = _as_bool(item.get("external", False))
        items.append(item)
    return items


def _nav_active(items, nav_section, pagename):
    """The page's section: ``nav_section`` if set, else the first item whose
    ``prefix`` the page name starts with; ``""`` when none matches."""
    if nav_section:
        return nav_section
    for item in items:
        prefix = item.get("prefix")
        if prefix and pagename.startswith(prefix):
            return item.get("section", "")
    return ""


def _as_dict(value):
    value = _parse(value)
    if isinstance(value, dict):
        return {str(k): str(v) for k, v in value.items()}
    if isinstance(value, (list, tuple)):
        return {
            str(item[0]): str(item[1])
            for item in value
            if isinstance(item, (list, tuple)) and len(item) == 2
        }
    return {}


def _as_list(value):
    value = _parse(value)
    if not value:
        return []
    if isinstance(value, str):
        return [p.strip() for p in value.split(",") if p.strip()]
    if isinstance(value, (list, tuple, set)):
        return [str(v) for v in value]
    return []


_CODE_BLOCK_RE = re.compile(
    r'(<div class="highlight-(?!(?:default|none|text) )([\w+#.-]+) notranslate")'
    r"(?![^>]*\sdata-lang=)(?=[\s>])"
)
_LANG_ALIASES = {
    "ipython3": "python",
    "ipython": "python",
    "python3": "python",
    "py": "python",
}


def _tag_code_block(match):
    lang = match.group(2)
    label = _LANG_ALIASES.get(lang, lang)
    return f'{match.group(1)} data-lang="{label}"'


def add_code_langs(body: str) -> str:
    """Tag highlighted code blocks with ``data-lang`` for the CSS label.

    Skips ``default``/``none``/``text`` and maps IPython/Python aliases to
    ``python``. Further attributes after ``class`` (e.g. the ``id`` of a
    ``:name:``d block) are kept. Idempotent: tags that already carry
    ``data-lang`` are left alone. Blocks with extra classes before
    ``highlight-*`` (``:class:``) are not labelled.
    """
    return _CODE_BLOCK_RE.sub(_tag_code_block, body)


def _page_date(value):
    """Return ``{"iso", "text"}`` for a front-matter date, or ``None`` if empty.

    Parsed strictly like ``notes.py`` (``date.fromisoformat``); a valid date
    is shown as ``iso``, anything else as plain text with ``iso`` ``None``.
    """
    if isinstance(value, (datetime.date, datetime.datetime)):
        value = value.isoformat()
    text = str(value or "").strip()
    if not text:
        return None
    try:
        iso = datetime.date.fromisoformat(text).isoformat()
    except ValueError:
        return {"iso": None, "text": text}
    return {"iso": iso, "text": iso}


def _svg_size(path: str) -> tuple[int, int] | None:
    """Return the root ``viewBox`` width/height of an SVG, rounded to ints.

    ``None`` (with a warning) when the file is unreadable, not SVG or has no
    usable ``viewBox``. Cached per path and modification time, so each file
    is read (and warned about) once per build, and a long-lived process
    (``sphinx-autobuild``) picks up an edited or newly added logo.
    """
    try:
        mtime = os.stat(path).st_mtime_ns
    except OSError:
        mtime = None
    return _svg_size_at(path, mtime)


@functools.cache
def _svg_size_at(path, mtime):
    """``_svg_size`` for one version (``mtime``) of the file."""
    try:
        with open(path, "rb") as f:
            _, root = next(ET.iterparse(f, events=("start",)))
    except OSError as exc:
        logger.warning("jupedsim_book: cannot read logo %s: %s", path, exc)
        return None
    except (ET.ParseError, StopIteration):
        root = None
    if root is None or root.tag not in (
        "svg",
        "{http://www.w3.org/2000/svg}svg",
    ):
        logger.warning(
            "jupedsim_book: logo %s is not an SVG; <img> gets no width/height",
            path,
        )
        return None
    try:
        _, _, width, height = (
            float(v)
            for v in re.split(r"[\s,]+", root.get("viewBox", "").strip())
        )
    except ValueError:
        width = height = 0.0
    if not all(math.isfinite(v) and v > 0 for v in (width, height)):
        logger.warning("jupedsim_book: logo %s has no usable viewBox", path)
        return None
    return round(width), round(height)


@functools.cache
def _warn_missing_logo(name):
    """Warn once that the logo ``name`` exists in no static directory."""
    logger.warning(
        "jupedsim_book: logo %s is in neither html_static_path nor the "
        "theme's static/; <img> gets no width/height",
        name,
    )


def _logo_size(app, name):
    """``(width, height)`` of the logo ``_static/<name>``; ``None`` if unset.

    Looks the file up where the HTML builder copies it from: project
    ``html_static_path`` entries override the theme's ``static/`` (later
    entries win). A file that exists nowhere is ``None`` with a warning.
    """
    if not name:
        return None
    candidates = [
        os.path.join(app.confdir, entry, name)
        for entry in reversed(app.config.html_static_path)
    ]
    theme = getattr(app.builder, "theme", None)
    for theme_dir in theme.get_theme_dirs() if theme else []:
        candidates.append(os.path.join(theme_dir, "static", name))
    candidates.append(
        os.path.join(os.path.dirname(os.path.abspath(__file__)), "static", name)
    )
    path = next((c for c in candidates if os.path.isfile(c)), None)
    if path is None:
        _warn_missing_logo(name)
        return None
    return _svg_size(os.path.normpath(path))


def _is_hidden_text(node):
    """True if a text node is not reader-visible prose (raw/cell output)."""
    parent = node.parent
    while parent is not None:
        if isinstance(parent, _HIDDEN_NODES):
            return True
        if isinstance(parent, nodes.container) and "cell_output" in parent.get(
            "classes", []
        ):
            return True
        parent = parent.parent
    return False


def _count_words(doctree):
    """Count reader-visible words; raw HTML/JS and notebook outputs are skipped."""
    return sum(
        len(node.astext().split())
        for node in doctree.findall(nodes.Text)
        if not _is_hidden_text(node)
    )


_TEMPLATES = {"landing": "landing.html"}
_CTA_KEYS = ("cta_primary", "cta_secondary")


def _page_template(meta, pagename):
    """Template for the front matter ``template`` value, or ``None``.

    ``landing`` selects ``landing.html``; any other non-empty value warns
    (naming the page) and the page keeps the default template.
    """
    value = str(meta.get("template") or "").strip()
    if not value:
        return None
    if value in _TEMPLATES:
        return _TEMPLATES[value]
    logger.warning(
        "jupedsim_book: page %s: unknown template %r (known: %s); "
        "using the default template",
        pagename,
        value,
        ", ".join(sorted(_TEMPLATES)),
        location=pagename,
    )
    return None


def _page_ctas(meta, pagename):
    """Buttons from ``cta_primary``/``cta_secondary``: ``"Label|url"``.

    Split at the first ``|``; label and url are stripped. A value without
    ``|`` or with an empty label or url is skipped with a warning.
    """
    ctas = []
    for key in _CTA_KEYS:
        value = meta.get(key)
        if value is None or not str(value).strip():
            continue
        label, sep, url = str(value).partition("|")
        label, url = label.strip(), url.strip()
        if not sep or not label or not url:
            logger.warning(
                "jupedsim_book: page %s: %s %r is not 'Label|url'; skipped",
                pagename,
                key,
                str(value),
                location=pagename,
            )
            continue
        ctas.append({"label": label, "url": url})
    return ctas


def _on_page_context(app, pagename, templatename, context, doctree):
    meta = context.get("meta") or {}

    # Front matter ``template: landing`` -> landing.html (hero, no TOC,
    # no page nav); its buttons come from cta_primary/cta_secondary.
    template = _page_template(meta, pagename)
    context["landing"] = template == "landing.html"
    context["page_ctas"] = (
        _page_ctas(meta, pagename) if context["landing"] else []
    )

    # Brand <img> width/height from each logo's viewBox (same fallback as
    # topbar.html: an empty logo_dark reuses logo_light).
    logo_light = str(context.get("theme_logo_light") or "").strip()
    logo_dark = str(context.get("theme_logo_dark") or "").strip() or logo_light
    context["logo_light_size"] = _logo_size(app, logo_light)
    context["logo_dark_size"] = _logo_size(app, logo_dark)

    # Top-bar navigation: the item of the page's section is marked active.
    # Only the docs project sets nav_section; it also gets the docs tree.
    nav_section = str(context.get("theme_nav_section") or "").strip()
    items = _nav_items(context.get("theme_nav_items"))
    active = _nav_active(items, nav_section, pagename)
    for item in items:
        item["active"] = bool(active) and item.get("section") == active
    context["nav_items"] = items
    context["docs_tree"] = bool(nav_section)
    context["footer_links"] = _nav_items(
        context.get("theme_footer_links"), "footer_links"
    )

    # A section's own index (listing) page is not an article: no prefix
    # kicker, no stats (set ``kicker:`` in its front matter instead).
    def _in_section(prefix):
        return pagename.startswith(prefix) and pagename != prefix + "index"

    kicker_by_prefix = _as_dict(context.get("theme_kicker_by_prefix"))
    # A section's listing page (``articles/index``, ``notes/index``): its
    # title already names the section and the bar marks it as current, so
    # there is no default kicker, and no prev/next (the next page would be
    # the first row of its own list).
    listing = any(pagename == prefix + "index" for prefix in kicker_by_prefix)

    kicker = meta.get("kicker")
    if not kicker:
        for prefix, value in kicker_by_prefix.items():
            if _in_section(prefix):
                kicker = value
                break
    if not kicker and not listing:
        kicker = context.get("theme_kicker")
    context["page_kicker"] = kicker or ""
    context["pagenav_enabled"] = not listing

    context["page_date"] = _page_date(meta.get("date"))

    stats = None
    if doctree is not None and any(
        _in_section(p) for p in _as_list(context.get("theme_stats_prefixes"))
    ):
        words = _count_words(doctree)
        stats = {"words": words, "minutes": max(1, round(words / 200))}
    context["page_stats"] = stats

    context["pagefind_index"] = (
        doctree is not None
        and not pagename.startswith("_modules/")
        and "nosearch" not in meta
    )

    # notfound.extension rewrites the 404 page's links to absolute ones, so
    # the tree stays valid there; without it the relative links would break.
    context["sidebar_tree"] = (
        pagename != "404" or "notfound.extension" in app.extensions
    )

    context["toc_enabled"] = bool(
        context.get("display_toc")
        and context.get("toc")
        and pagename not in _SKIP_TOC_PAGES
        and not context["landing"]
        and str(meta.get("toc", "")).lower() != "false"
    )

    if "body" in context:
        context["body"] = add_code_langs(context["body"])

    return template


def setup(app):
    app.add_html_theme(
        "jupedsim_book", os.path.dirname(os.path.abspath(__file__))
    )
    app.add_css_file("sphinx.css", priority=900)
    app.add_js_file("theme.js", loading_method="defer")
    app.add_js_file("search.js", loading_method="defer")
    app.add_js_file("switcher.js", loading_method="defer")
    app.connect("html-page-context", _on_page_context)
    return {"parallel_read_safe": True, "parallel_write_safe": True}
