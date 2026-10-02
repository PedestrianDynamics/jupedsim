# SPDX-License-Identifier: LGPL-3.0-or-later
"""Sphinx configuration for the jupedsim.org site (landing, articles, notes).

Built from master and published at the root of jupedsim.org; the versioned
documentation lives in ``docs/`` and is published under ``/docs/<version>/``.
"""

import html
import os
import re
import sys

from docutils import nodes
from docutils.utils.math import MathError, latex2mathml
from sphinx.util import logging
from sphinx.util.math import get_node_equation_number

_here = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.normpath(os.path.join(_here, "../docs/_theme")))
sys.path.insert(0, os.path.join(_here, "_ext"))

project = "JuPedSim"
copyright = "Forschungszentrum Jülich GmbH, IAS-7"
author = "The JuPedSim Development Team"

extensions = [
    "jupedsim_book",
    "notes",
    "myst_nb",
    "sphinx_copybutton",
    "sphinx_favicon",
    "notfound.extension",
]

exclude_patterns = ["_build", "jupyter_execute", "_ext"]

# -- MyST / notebooks ------------------------------------------------------
myst_enable_extensions = [
    "amsmath",
    "colon_fence",
    "deflist",
    "dollarmath",
    "html_image",
]

# "auto" executes notebooks that carry no outputs, which requires an
# importable jupedsim (the build's bindings or the PyPI wheel).
nb_execution_mode = "auto"
nb_execution_timeout = 900
nb_execution_raise_on_error = True

# -- Notes -----------------------------------------------------------------
notes_dirs = ["notes"]
notes_feed_path = "notes/feed.xml"
notes_feed_title = "JuPedSim notes"

# -- HTML ------------------------------------------------------------------
html_theme = "jupedsim_book"
html_theme_path = ["../docs/_theme"]
html_baseurl = "https://www.jupedsim.org/"
html_static_path = ["_static"]
html_permalinks_icon = "#"
html_copy_source = False
html_show_sourcelink = False
# Math is rendered to MathML at build time (see setup below), so the site
# makes no request to a MathJax CDN.
html_math_renderer = "mathml"
favicons = ["logo.png"]

html_theme_options = {
    "site_name": "JuPedSim",
    "nav_home_url": "/",
    "nav_items": [
        {
            "title": "Articles",
            "url": "/articles/",
            "section": "articles",
            "prefix": "articles/",
        },
        {
            "title": "Notes",
            "url": "/notes/",
            "section": "notes",
            "prefix": "notes/",
        },
        {"title": "Docs", "url": "/docs/stable/", "section": "docs"},
        {
            "title": "Docs v1",
            "url": "/docs/v1x/v1.4.2/",
            "section": "docs-v1",
            "external": True,
        },
    ],
    "kicker": "JuPedSim",
    "kicker_by_prefix": {"articles/": "Article", "notes/": "Note"},
    "stats_prefixes": ["articles/"],
    "feed_url": "/notes/feed.xml",
    "footer_text": "© JuPedSim contributors · LGPL-3.0",
    "footer_links": [
        {
            "title": "GitHub",
            "url": "https://github.com/PedestrianDynamics/jupedsim",
        },
        {"title": "RSS", "url": "/notes/feed.xml"},
        {"title": "1.x docs", "url": "/docs/v1x/v1.4.2/"},
    ],
}

# This 404 page is the one GitHub Pages serves for the whole domain.
notfound_urls_prefix = "/"


# docutils' latex2mathml turns an accent followed by a script (``\vec{F}_i``)
# into an under/over construct, which puts the script below the symbol.
# Attach scripts to the right of accented symbols instead.
_handle_script_or_limit = latex2mathml.handle_script_or_limit


def _script_right_of_accents(node, c, limits=""):
    child = node[-1] if len(node) else None
    if (
        limits == ""
        and child is not None
        and not child.get("movablelimits")
        and not (len(child) and child[0].get("movablelimits"))
    ):
        if (c == "_" and isinstance(child, latex2mathml.mover)) or (
            c == "^" and isinstance(child, latex2mathml.munder)
        ):
            node.pop()
            script = latex2mathml.msub if c == "_" else latex2mathml.msup
            new_node = script(child)
            node.append(new_node)
            return new_node
    return _handle_script_or_limit(node, c, limits)


latex2mathml.handle_script_or_limit = _script_right_of_accents

# MyST's amsmath extension passes whole environments; latex2mathml renders
# their content (rows separated by \\ become an aligned table).
_AMS_ENV = re.compile(
    r"^\s*\\begin\{(equation|align|gather|multline)(\*?)\}(.*)"
    r"\\end\{\1\2\}\s*$",
    re.DOTALL,
)
logger = logging.getLogger(__name__)


def _mathml(node, block):
    tex = node.astext()
    if block:
        m = _AMS_ENV.match(tex)
        if m:
            tex = m.group(3)
    try:
        return latex2mathml.tex2mathml(tex, as_block=block)
    except MathError as err:
        logger.warning("cannot render math: %s", err, location=node)
        tag = "div" if block else "span"
        return f'<{tag} class="math">{html.escape(node.astext())}</{tag}>'


def _visit_inline_math(self, node):
    self.body.append(_mathml(node, False))
    raise nodes.SkipNode


def _visit_block_math(self, node):
    if node.get("number"):
        self.body.append(f'<div class="math" id="{node["ids"][0]}">')
        number = get_node_equation_number(self, node)
        self.body.append(f'<span class="eqno">({number})')
        self.add_permalink_ref(node, "")
        self.body.append("</span>")
    else:
        self.body.append('<div class="math">')
    self.body.append(_mathml(node, True) + "</div>\n")
    raise nodes.SkipNode


def setup(app):
    app.add_html_math_renderer(
        "mathml",
        (_visit_inline_math, None),
        (_visit_block_math, None),
    )
