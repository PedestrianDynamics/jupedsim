# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information
project = 'JuPedSim'
copyright = "2023, Forschungszentrum Jülich GmbH, IAS-7"
author = 'Dev-Team'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    "sphinx.ext.autodoc",
    "sphinx_design",
    "sphinx_copybutton",
    "sphinx.ext.mathjax",
    "sphinx.ext.viewcode",
    "sphinxcontrib.apidoc",
    "sphinx.ext.napoleon",
]

templates_path = ['_templates']
exclude_patterns = []

# automatic generation of api doc

apidoc_module_dir = "../../python_modules/jupedsim/jupedsim"
apidoc_output_dir = "api"
apidoc_excluded_paths = ["tests"]
apidoc_separate_modules = True
apidoc_toc_file = "index"
apidoc_extra_args = ["--implicit-namespaces", "-d 10"]
apidoc_module_first = True

napoleon_include_init_with_doc = False
napoleon_include_private_with_doc = False
napoleon_include_special_with_doc = True
napoleon_use_admonition_for_examples = False
napoleon_use_admonition_for_notes = False
napoleon_use_admonition_for_references = False
napoleon_use_ivar = False
napoleon_use_param = True
napoleon_use_rtype = False
napoleon_preprocess_types = False
napoleon_type_aliases = None
napoleon_attr_annotations = True

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'pydata_sphinx_theme'
html_static_path = ['_static']

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
