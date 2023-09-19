# Configuration file for the Sphinx documentation builder.

project = "Otter"
copyright = "2023, Adam Tuft"
author = "Adam Tuft"

release = "0.1"
version = "0.1.0"

root_doc = "source/index"

extensions = [
    "sphinx.ext.duration",
    "sphinx.ext.doctest",
    "sphinx.ext.autodoc",
    "sphinx.ext.autosummary",
    "sphinx.ext.intersphinx",
]

intersphinx_mapping = {
    "python": ("https://docs.python.org/3/", None),
    "sphinx": ("https://www.sphinx-doc.org/en/master/", None),
}
intersphinx_disabled_domains = ["std"]

templates_path = ["_templates"]

# Options for HTML output

html_theme = "renku"

# Options for EPUB output
epub_show_urls = "footnote"
