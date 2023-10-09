# Copyright © 2012-2023 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later
# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information
import datetime

project = "JuPedSim"
copyright = (
    f"{datetime.datetime.today().year}, Forschungszentrum Jülich GmbH, IAS-7"
)

import jupedsim

version = "v" + jupedsim.__version__

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    "sphinx_copybutton",
    "sphinx.ext.mathjax",
    "sphinx.ext.viewcode",
    "autoapi.extension",
    "sphinx_favicon",
    "notfound.extension",
    "sphinx.ext.mathjax",
    "myst_nb",
    "sphinx.ext.intersphinx",
    "sphinx.ext.napoleon",
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
autoapi_dirs = [
    "../../python_modules/jupedsim",
]
autoapi_root = "api"
autoapi_options = [
    "members",
    "undoc-members",
    "show-inheritance",
    "show-module-summary",
    "imported-members",
]
autoapi_ignore = ["**/tests/**"]
# autoapi_member_order = "groupwise"
autodoc_typehints = "description"
autoapi_python_class_content = "both"

add_module_names = False

# -- Automatic execution of jupyter notebooks --------------------------------
nb_execution_excludepatterns = []
nb_execution_timeout = 120
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

html_theme = "sphinx_book_theme"
html_static_path = ["_static"]

html_logo = "_static/jupedsim.svg"
favicons = [
    "logo.png",
]
html_css_files = [
    "css/custom.css",
]
html_js_files = [
    "https://cdnjs.cloudflare.com/ajax/libs/require.js/2.3.4/require.min.js",
]
html_context = {"default_mode": "light"}

html_theme_options = {
    "home_page_in_toc": False,
    "use_fullscreen_button": False,
    "use_issues_button": False,
    "use_download_button": False,
    "article_header_end": ["search-button", "toggle-secondary-sidebar"],
    "icon_links": [
        {
            "name": "GitHub",
            "url": "https://github.com/PedestrianDynamics/jupedsim",
            "icon": "fa-brands fa-github",
        },
        {
            "name": "PyPI",
            "url": "https://pypi.org/project/jupedsim/",
            "icon": "https://img.shields.io/pypi/v/jupedsim",
            "type": "url",
        },
        {
            "name": "DOI",
            "url": "https://doi.org/10.5281/zenodo.6470814",
            "icon": "https://zenodo.org/badge/DOI/10.5281/zenodo.6470814.svg",
            "type": "url",
        },
    ],
    "switcher": {
        "json_url": "https://www.jupedsim.org/versions.json",
        "version_match": version,
    },
    "primary_sidebar_end": ["version-switcher"],
    "show_toc_level": 3,
}

html_sidebars = {
    "**": ["navbar-logo", "icon-links", "search-field", "sbt-sidebar-nav.html"]
}

# -- Options for EPUB output
epub_show_urls = "footnote"
