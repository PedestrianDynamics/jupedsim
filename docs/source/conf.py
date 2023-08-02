# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information
project = "JuPedSim"
copyright = "2023, Forschungszentrum Jülich GmbH, IAS-7"
author = "Dev-Team"

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    "sphinx_design",
    "sphinx_copybutton",
    "sphinx.ext.mathjax",
    "sphinx.ext.viewcode",
    "sphinxcontrib.apidoc",
    "sphinx.ext.napoleon",
    "autoapi.extension",
]

templates_path = ["_templates"]
exclude_patterns = []

# automatic generation of api doc
autoapi_dirs = [
    "../../python_modules/jupedsim",
]
autoapi_root = "api"
autoapi_options = [
    "members",
    "undoc-members",
    # "inherited-members",
    # "private-members",
    "show-inheritance",
    "show-module-summary",
    # "special-members",
    "imported-members",
]
autoapi_ignore = ["**/tests/**"]
autoapi_member_order = ["groupwise"]


# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = "pydata_sphinx_theme"
html_static_path = ["_static"]

html_logo = "_static/jupedsim.png"

html_css_files = [
    "css/custom.css",
]
html_context = {"default_mode": "light"}

html_theme_options = {
    "show_nav_level": 5,
    "github_url": "https://github.com/PedestrianDynamics/jupedsim",
    "header_links_before_dropdown": 5,
    "show_toc_level": 5,
    "navbar_end": ["navbar-icon-links"],
}

# -- Options for EPUB output
epub_show_urls = "footnote"
