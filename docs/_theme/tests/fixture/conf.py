# SPDX-License-Identifier: LGPL-3.0-or-later
"""Kitchen-sink fixture for the jupedsim_book theme.

``FIXTURE_MODE=docs`` selects the docs option set; default is site mode;
``FIXTURE_MODE=site-no-notfound`` is site mode without ``notfound.extension``;
``FIXTURE_MODE=bad-template`` is site mode plus ``bad-template.md`` (an
unknown ``template:`` value, built without ``-W``).
This is test input for check.sh, not a build switch.
"""

import os
import sys

_here = os.path.dirname(os.path.abspath(__file__))
# dummymod must be importable so viewcode emits _modules/ and [source] links.
sys.path.insert(0, _here)
sys.path.insert(0, os.path.normpath(os.path.join(_here, "../..")))
sys.path.insert(
    0, os.path.normpath(os.path.join(_here, "../../../../site/_ext"))
)

project = "JuPedSim fixture"
author = "JuPedSim"

extensions = [
    "jupedsim_book",
    "notes",
    "myst_nb",
    "sphinx_copybutton",
    "autoapi.extension",
    "sphinx.ext.viewcode",
    "sphinx_favicon",
]
# site-no-notfound: site options without notfound.extension (404 sidebar check).
if os.environ.get("FIXTURE_MODE") != "site-no-notfound":
    extensions.append("notfound.extension")
notfound_urls_prefix = "/"

myst_enable_extensions = ["colon_fence", "dollarmath"]
nb_execution_mode = "auto"

autoapi_dirs = ["dummymod"]
autoapi_add_toctree_entry = False
autoapi_root = "api"

exclude_patterns = ["_build", "jupyter_execute", "dummymod"]
if os.environ.get("FIXTURE_MODE") != "bad-template":
    exclude_patterns.append("bad-template.md")

html_theme = "jupedsim_book"
html_theme_path = ["../.."]
html_baseurl = "https://example.org/"
html_static_path = ["_static"]
mathjax_path = "mathjax-local-placeholder.js"
html_permalinks_icon = "#"
html_last_updated_fmt = "%Y-%m-%d"
favicons = [{"rel": "icon", "href": "favicon.svg", "type": "image/svg+xml"}]
numfig = True

# Same top-bar items as site/conf.py and docs/source/conf.py.
_nav_items = [
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
]

# Same footer as site/conf.py and docs/source/conf.py (all modes).
_footer_text = "© JuPedSim contributors · LGPL-3.0"
_footer_links = [
    {
        "title": "GitHub",
        "url": "https://github.com/PedestrianDynamics/jupedsim",
    },
    {"title": "RSS", "url": "/notes/feed.xml"},
    {"title": "1.x docs", "url": "/docs/v1x/v1.4.2/"},
]

_site_options = {
    "site_name": "JuPedSim",
    "nav_home_url": "/",
    "nav_items": _nav_items,
    "footer_text": _footer_text,
    "footer_links": _footer_links,
    "kicker": "JuPedSim",
    "kicker_by_prefix": {"articles/": "Article", "notes/": "Note"},
    "stats_prefixes": ["articles/"],
    "feed_url": "/notes/feed.xml",
}

_docs_options = {
    "site_name": "JuPedSim",
    "nav_home_url": "/",
    "nav_items": _nav_items,
    "footer_text": _footer_text,
    "footer_links": _footer_links,
    "nav_section": "docs",
    "kicker": "Docs",
    "switcher_json_url": "/docs/versions.json",
    "switcher_version_match": "v9.9.9",
}

if os.environ.get("FIXTURE_MODE") == "docs":
    html_theme_options = _docs_options
else:
    html_theme_options = _site_options
