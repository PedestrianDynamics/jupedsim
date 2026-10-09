# SPDX-License-Identifier: LGPL-3.0-or-later
# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information
import datetime
import logging
import os
import re
import sys

sys.path.insert(
    0, os.path.abspath(os.path.join(os.path.dirname(__file__), "_scripts"))
)
from generate_bibtex import get_latest_jupedsim_bibtex

sys.path.insert(
    0, os.path.abspath(os.path.join(os.path.dirname(__file__), "../_theme"))
)

import jupedsim
from sphinx.errors import ConfigError

project = "JuPedSim"
author = "The JuPedSim Development Team"
copyright = (
    f"{datetime.datetime.today().year}, Forschungszentrum Jülich GmbH, IAS-7"
)

version = "v" + jupedsim.__version__

logging.info(f"Create documentation for JuPedSim {version}")

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    "jupedsim_book",
    "sphinx_copybutton",
    "sphinx.ext.mathjax",
    "sphinx.ext.viewcode",
    "autoapi.extension",
    "sphinx_favicon",
    "notfound.extension",
    "myst_nb",
    "sphinx.ext.intersphinx",
    "sphinx.ext.napoleon",
    "sphinx.ext.autosectionlabel",
]

templates_path = ["_templates"]
exclude_patterns = []

# -- Linking ---------------------------------------------------------
intersphinx_mapping = {
    "python": ("https://docs.python.org/3/", None),
    "sphinx": ("https://www.sphinx-doc.org/en/master/", None),
    "shapely": ("https://shapely.readthedocs.io/en/2.0.1/", None),
}

# -- Automatic generation of API doc -----------------------------------------
# Analyse the imported (built or installed) package, not python_modules/: the
# native types exist for autoapi only as the pybind11-stubgen stubs
# (py_jupedsim.pyi) that the build generates into the package.
autoapi_dirs = [os.path.dirname(jupedsim.__file__)]
# A build stage keeps symlinks to deleted sources until CMake re-runs; autoapi
# would crash on them with a bare FileNotFoundError.
_DANGLING_LINKS = sorted(
    path
    for root, _, files in os.walk(autoapi_dirs[0])
    for path in (os.path.join(root, name) for name in files)
    if os.path.islink(path) and not os.path.exists(path)
)
if _DANGLING_LINKS:
    raise ConfigError(
        "Dangling symlinks in the analysed jupedsim package (stale build "
        "stage?): "
        + ", ".join(_DANGLING_LINKS)
        + ". Re-run the build (ninja re-runs CMake, which removes them) or "
        "delete them: find <build-dir>/stage -xtype l -delete"
    )
autoapi_root = "api"
autoapi_options = [
    "members",
    "undoc-members",
    "show-inheritance",
    "show-module-summary",
    "imported-members",
]
autoapi_ignore = [
    "**/tests/**",
    "**/internal/**",
]
autoapi_add_toctree_entry = False
autoapi_python_class_content = "class"
autoapi_template_dir = "_templates/autoapi"
autoapi_member_order = "groupwise"
autoapi_python_use_implicit_namespaces = True

add_module_names = False

suppress_warnings = [
    # autosectionlabel creates a label per section title; several pages
    # repeat titles (per model / per example, e.g. "Download"). Sphinx names
    # these warnings by the emitting document, which depends on read order
    # (parallel builds), so no narrower scope is stable.
    "autosectionlabel.*",
]


# jupedsim.native and jupedsim.py_jupedsim must be analysed so that autoapi can
# resolve the re-exports in jupedsim, but their pages would duplicate every
# native type under an internal path (and make cross-references ambiguous).
_INTERNAL_PACKAGES = ("jupedsim.native", "jupedsim.py_jupedsim")


def skip_rules(app, what, name, obj, skip, options):
    if what == "module":
        skip = True
    if what == "package" and name in _INTERNAL_PACKAGES:
        skip = True
    if what == "method":
        if name.endswith("as_native"):
            skip = True
        if "tracing" in name:
            skip = True
        if "get_last_trace" in name:
            skip = True
    return skip


# jupedsim.internal.{aabb,grid,tracing} are excluded via autoapi_ignore, so
# autoapi cannot resolve imports from them and drops the imported names. Only
# these known imports are silenced; any other unresolved import (including
# jupedsim.native, which must resolve through the stubs) fails a -W build.
# Known gap: the Python tracing helpers (Timer, trace_event from
# jupedsim.internal.tracing) are missing from the API reference.
_KNOWN_UNRESOLVED_IMPORT = re.compile(
    r"Cannot resolve import of unknown module "
    r"jupedsim\.internal\.(aabb|grid|tracing) in "
)
# pybind11-stubgen lists the bound submodule in the stub's __all__. autoapi
# does not count submodules as members when it expands the star import in
# jupedsim.native and warns about this entry; the submodule is internal.
_KNOWN_INVALID_ALL_ENTRY = re.compile(
    r"Invalid __all__ entry floorfield in jupedsim\.py_jupedsim$"
)


class _KnownUnresolvedImportFilter(logging.Filter):
    def filter(self, record):
        message = record.getMessage()
        return not (
            _KNOWN_UNRESOLVED_IMPORT.match(message)
            or _KNOWN_INVALID_ALL_ENTRY.match(message)
        )


# The docs build warns and continues without network. CI builds with -W
# (build_site.py --strict), so network failures (Zenodo, intersphinx hosts)
# are printed to stderr instead of being Sphinx warnings: an outage must not
# fail a PR or release build. Sphinx gives the intersphinx fetch warning no
# type, so suppress_warnings cannot target it; this filter takes it out of the
# warning count and prints it instead. Every other warning stays fatal.
def _warn_network(message):
    print(f"WARNING (network, not fatal): {message}", file=sys.stderr)


class _UnreachableInventoryFilter(logging.Filter):
    def filter(self, record):
        message = record.getMessage()
        if record.levelno == logging.WARNING and message.startswith(
            "failed to reach any of the inventories"
        ):
            _warn_network(message)
            return False
        return True


def setup(sphinx):
    sphinx.connect("autoapi-skip-member", skip_rules)
    logging.getLogger("sphinx.autoapi._mapper").addFilter(
        _KnownUnresolvedImportFilter()
    )
    logging.getLogger("sphinx.sphinx.ext.intersphinx").addFilter(
        _UnreachableInventoryFilter()
    )


# -- Automatic execution of jupyter notebooks --------------------------------
nb_execution_excludepatterns = []
nb_execution_timeout = 900
# Execute in a temporary directory so the trajectory files the notebooks write
# stay out of the source tree. Notebooks that read data files relative to
# themselves opt out in their metadata ("mystnb": {"execution_in_temp": false}).
# A FileNotFoundError for demo-data/... under a temporary path during the docs
# build means the notebook reads such a file and needs that opt-out.
nb_execution_in_temp = True
myst_enable_extensions = [
    "amsmath",
    "colon_fence",
    "deflist",
    "dollarmath",
    "html_image",
]

nb_execution_raise_on_error = True

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = "jupedsim_book"
html_theme_path = ["../_theme"]
html_static_path = ["_static"]
html_baseurl = "https://www.jupedsim.org/docs/stable/"
html_permalinks_icon = "#"
html_copy_source = False
html_show_sourcelink = False
html_last_updated_fmt = "%Y-%m-%d"

favicons = [
    "logo.png",
]
html_js_files = [
    "https://cdnjs.cloudflare.com/ajax/libs/require.js/2.3.4/require.min.js",
]

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
    "nav_section": "docs",
    "kicker": "Docs",
    "switcher_json_url": "/docs/versions.json",
    "switcher_version_match": version,
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

notfound_urls_prefix = "/docs/stable/"

# -- Options for EPUB output
epub_show_urls = "footnote"

# -- Automatic fetch citation info from zenodo --------------------------------
try:
    bibtex = get_latest_jupedsim_bibtex(version)
except Exception as e:
    bibtex = None
    _fetch_error = e
else:
    _fetch_error = None

if bibtex is None:
    _warn_network(
        f"Could not fetch bibtex for JuPedSim {version} from Zenodo"
        + (f" ({_fetch_error})" if _fetch_error else "")
        + "; writing a placeholder citation file"
    )
    bibtex = (
        f"% Citation information for JuPedSim {version} could not be fetched.\n"
        "% Please check on Zenodo: https://doi.org/10.5281/zenodo.1293771\n"
    )
else:
    logging.info(f"Bibtex fetched successfully:\n{bibtex}")

output_dir = os.path.join(os.path.dirname(__file__), "citation")
os.makedirs(output_dir, exist_ok=True)
with open(os.path.join(output_dir, "jupedsim_bibtex.bib"), "w") as f:
    f.write(bibtex)
