# SPDX-License-Identifier: LGPL-3.0-or-later
"""Sphinx extension ``notes``: dated note listings and an RSS feed.

The ``notes-list`` directive (options ``dirs``, comma-separated, default
``notes_dirs``; ``limit``; ``style``) leaves a placeholder that is replaced
on ``doctree-resolved`` with a newest-first list. Entries are the documents
under each dir (except ``<dir>/index``), sorted by their front matter
``date`` (ISO ``YYYY-MM-DD``). ``:style: list`` (default) renders
``ul.post-list`` (kicker with date and author, title link, description);
``rows`` renders ``ul.post-rows``, one ``a.post-row-link`` per entry holding
date, title and an ``aria-hidden`` arrow; ``rows-desc`` adds the
description as the link's last span. HTML builds also write an RSS 2.0 feed
of all ``notes_dirs`` entries to ``<outdir>/<notes_feed_path>``, with links
based on ``html_baseurl``.
"""

import html
import os
import xml.etree.ElementTree as ET
from datetime import date, datetime, timezone
from email.utils import format_datetime

from docutils import nodes
from docutils.parsers.rst import Directive, directives
from sphinx.util import logging

logger = logging.getLogger(__name__)


STYLES = ("list", "rows", "rows-desc")


class NotesListNode(nodes.General, nodes.Element):
    """Placeholder for a ``notes-list`` directive."""


class NotesListDirective(Directive):
    has_content = False
    option_spec = {
        "dirs": directives.unchanged,
        "limit": directives.nonnegative_int,
        "style": lambda arg: directives.choice(arg, STYLES),
    }

    def run(self):
        node = NotesListNode()
        dirs = self.options.get("dirs")
        node["dirs"] = (
            [d.strip().strip("/") for d in dirs.split(",") if d.strip()]
            if dirs
            else None
        )
        node["limit"] = self.options.get("limit")
        node["style"] = self.options.get("style", "list")
        env = self.state.document.settings.env
        if not hasattr(env, "notes_list_docs"):
            env.notes_list_docs = set()
        env.notes_list_docs.add(env.docname)
        return [node]


def _dirs(config, dirs):
    return [d.strip("/") for d in (dirs or config.notes_dirs)]


def collect_entries(env, dirs):
    """Return ``[(date, docname)]`` newest first for documents under dirs."""
    entries = []
    for d in dirs:
        prefix = d + "/"
        for docname in env.found_docs:
            if not docname.startswith(prefix) or docname == prefix + "index":
                continue
            raw = env.metadata.get(docname, {}).get("date")
            try:
                when = date.fromisoformat(str(raw).strip())
            except ValueError:
                logger.warning(
                    "notes: missing or invalid 'date' (%r); expected YYYY-MM-DD",
                    raw,
                    location=docname,
                )
                continue
            entries.append((when, docname))
    entries.sort(key=lambda e: (e[0], e[1]), reverse=True)
    return entries


def _title(env, docname):
    title = env.titles.get(docname)
    return title.astext() if title is not None else docname


def _rows(app, fromdocname, entries, with_desc):
    """``ul.post-rows``: each entry is one link (date, title, arrow and,
    ``with_desc``, the description)."""
    env = app.env
    ul = nodes.bullet_list(classes=["post-rows"])
    for when, docname in entries:
        href = app.builder.get_relative_uri(fromdocname, docname)
        iso = when.isoformat()
        parts = [
            f'<time class="post-row-date" datetime="{iso}">{iso}</time>',
            '<span class="post-row-title">'
            f"{html.escape(_title(env, docname))}</span>",
            '<span class="post-row-arrow" aria-hidden="true">→</span>',
        ]
        description = env.metadata.get(docname, {}).get("description")
        if with_desc and description:
            parts.append(
                '<span class="post-row-desc">'
                f"{html.escape(str(description))}</span>"
            )
        # Newlines between the grid items: not rendered, but they keep the
        # parts apart in text extracts (search index, excerpts).
        link = (
            f'<a class="post-row-link" href="{html.escape(href)}">'
            + "\n".join(parts)
            + "</a>"
        )
        li = nodes.list_item(classes=["post-row"])
        li += nodes.raw("", link, format="html")
        ul += li
    return ul


def process_notes_lists(app, doctree, fromdocname):
    env = app.env
    for node in list(doctree.findall(NotesListNode)):
        entries = collect_entries(env, _dirs(app.config, node["dirs"]))
        if node["limit"] is not None:
            entries = entries[: node["limit"]]
        style = node.get("style", "list")
        if style != "list":
            node.replace_self(
                _rows(app, fromdocname, entries, style == "rows-desc")
            )
            continue
        ul = nodes.bullet_list(classes=["post-list"])
        for when, docname in entries:
            meta = env.metadata.get(docname, {})
            li = nodes.list_item(classes=["post-list-item"])
            kicker = nodes.paragraph(classes=["post-list-kicker"])
            iso = when.isoformat()
            kicker += nodes.raw(
                "", f'<time datetime="{iso}">{iso}</time>', format="html"
            )
            author = meta.get("author")
            if author:
                kicker += nodes.Text(f" · {author}")
            li += kicker
            # The theme expects the title link directly in the <li>, not in a <p>.
            href = app.builder.get_relative_uri(fromdocname, docname)
            li += nodes.raw(
                "",
                f'<a class="post-list-title" href="{html.escape(href)}">'
                f"{html.escape(_title(env, docname))}</a>\n",
                format="html",
            )
            description = meta.get("description")
            if description:
                li += nodes.paragraph(
                    "", str(description), classes=["post-list-desc"]
                )
            ul += li
        node.replace_self(ul)


def purge_notes_lists(app, env, docname):
    if hasattr(env, "notes_list_docs"):
        env.notes_list_docs.discard(docname)


def merge_notes_lists(app, env, docnames, other):
    if not hasattr(env, "notes_list_docs"):
        env.notes_list_docs = set()
    env.notes_list_docs |= getattr(other, "notes_list_docs", set())


def outdated_notes_lists(app, env, added, changed, removed):
    """Re-read listing pages whenever any document was added/changed/removed."""
    if not (added or changed or removed):
        return []
    return sorted(getattr(env, "notes_list_docs", set()) - set(removed))


def _absolute_url(baseurl, path):
    return baseurl.rstrip("/") + "/" + path.lstrip("/")


def write_feed(app, exception):
    # With -W, Sphinx keeps going and only sets statuscode after this event,
    # so check its (private) warning counters to skip failed builds.
    failed_by_warnings = getattr(app, "_fail_on_warnings", False) and getattr(
        app, "_warncount", 0
    )
    if (
        exception is not None
        or failed_by_warnings
        or app.builder.format != "html"
    ):
        return
    env, config = app.env, app.config
    baseurl = config.html_baseurl or ""
    if not baseurl:
        logger.warning(
            "notes: html_baseurl is not set; feed links are relative"
        )
    rss = ET.Element("rss", version="2.0")
    channel = ET.SubElement(rss, "channel")
    ET.SubElement(channel, "title").text = (
        config.notes_feed_title or config.project
    )
    ET.SubElement(channel, "link").text = _absolute_url(baseurl, "")
    ET.SubElement(channel, "description").text = (
        config.notes_feed_title or config.project
    )
    for when, docname in collect_entries(env, _dirs(config, None)):
        meta = env.metadata.get(docname, {})
        link = _absolute_url(baseurl, app.builder.get_target_uri(docname))
        item = ET.SubElement(channel, "item")
        ET.SubElement(item, "title").text = _title(env, docname)
        ET.SubElement(item, "link").text = link
        ET.SubElement(item, "pubDate").text = format_datetime(
            datetime(when.year, when.month, when.day, tzinfo=timezone.utc)
        )
        ET.SubElement(item, "guid", isPermaLink="true").text = link
        ET.SubElement(item, "description").text = str(
            meta.get("description", "")
        )
    path = os.path.join(app.outdir, config.notes_feed_path)
    os.makedirs(os.path.dirname(path), exist_ok=True)
    ET.ElementTree(rss).write(path, encoding="utf-8", xml_declaration=True)


def setup(app):
    app.add_config_value("notes_dirs", ["notes"], "env")
    app.add_config_value("notes_feed_path", "notes/feed.xml", "html")
    app.add_config_value("notes_feed_title", "", "html")
    app.add_node(NotesListNode)
    app.add_directive("notes-list", NotesListDirective)
    app.connect("doctree-resolved", process_notes_lists)
    app.connect("env-purge-doc", purge_notes_lists)
    app.connect("env-merge-info", merge_notes_lists)
    app.connect("env-get-outdated", outdated_notes_lists)
    app.connect("build-finished", write_feed)
    return {
        "version": "0.1",
        "parallel_read_safe": True,
        "parallel_write_safe": True,
    }
