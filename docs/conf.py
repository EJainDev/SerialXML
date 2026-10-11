"""Sphinx configuration for SerialXML's Read the Docs website."""

import os
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent / "_ext"))

project = "SerialXML"
author = "SerialXML contributors"
html_show_copyright = False
version = "1.0"
release = "1.0.0"

extensions = ["myst_parser", "cpp26"]
source_suffix = {".rst": "restructuredtext", ".md": "markdown"}
root_doc = "index"
exclude_patterns = ["README.md", "_snippets"]
templates_path = ["_templates"]
myst_heading_anchors = 3
highlight_language = "cpp"
pygments_style = "friendly"
pygments_dark_style = "github-dark"
nitpicky = True
# The standard library is external, and marker backing types are intentionally opaque.
# Keep checks enabled for all actual SerialXML public symbols.
nitpick_ignore_regex = [("cpp:identifier", r"std::.*")]
nitpick_ignore = [
    ("cpp:identifier", marker + "_")
    for marker in (
        "attribute", "skip", "raw", "cdata", "unpack", "no_unpack", "no_iter",
        "exclude_on_empty", "optional", "setter",
    )
]

html_theme = "furo"
html_title = "SerialXML documentation"
html_static_path = ["_static"]
html_css_files = ["custom.css"]
html_js_files = ["api-search.js"]
html_theme_options = {
    # Blue/cyan accents and navy text echo the README banner. Link colors use
    # darker/lighter variants to retain contrast in each theme.
    "light_css_variables": {
        "color-brand-primary": "#3679f5",
        "color-brand-content": "#245fd1",
        "color-foreground-primary": "#0b1425",
        "color-foreground-secondary": "#536078",
        "color-background-primary": "#ffffff",
        "color-background-secondary": "#f5f9ff",
        "color-background-border": "#d8e4f4",
        "color-sidebar-background": "#f5f9ff",
        "color-sidebar-link-text": "#536078",
    },
    "dark_css_variables": {
        "color-brand-primary": "#6cbdff",
        "color-brand-content": "#82c4ff",
        "color-foreground-primary": "#e8effb",
        "color-foreground-secondary": "#b0bfd6",
        "color-background-primary": "#0b1425",
        "color-background-secondary": "#14223b",
        "color-background-border": "#2a3d5b",
        "color-sidebar-background": "#101c30",
        "color-sidebar-link-text": "#b0bfd6",
    },
    "source_repository": "https://github.com/EJainDev/SerialXML/",
    "source_branch": "main",
    "source_directory": "docs/",
}

# Fill this in after importing the repository into Read the Docs.
# Read the Docs supplies its canonical URL automatically during hosted builds.
READTHEDOCS_URL = ""
html_baseurl = os.environ.get("READTHEDOCS_CANONICAL_URL", READTHEDOCS_URL)
if os.environ.get("READTHEDOCS") == "True":
    html_context = {
        "READTHEDOCS": True,
        "READTHEDOCS_PROJECT": os.environ.get("READTHEDOCS_PROJECT", ""),
        "READTHEDOCS_VERSION": os.environ.get("READTHEDOCS_VERSION", ""),
    }
