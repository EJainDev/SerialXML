# Building and publishing the documentation

This site uses [Sphinx](https://www.sphinx-doc.org/),
[MyST](https://myst-parser.readthedocs.io/) for Markdown, and the
[Furo theme](https://furo.readthedocs.io/). It builds without the C++ toolchain.
Python 3.13 is used by Read the Docs and documentation CI.

## Build locally

Run these commands from the repository root:

```bash
python3 -m venv build/docs-venv
build/docs-venv/bin/python -m pip install -r docs/requirements.txt
build/docs-venv/bin/python -m sphinx -b html -n -W --keep-going docs build/docs/html
python3 -m http.server 8000 --directory build/docs/html
```

Open <http://localhost:8000>. Search, the symbol index, source links, and the
light/dark theme toggle are available in the generated site. Generated HTML and
the virtual environment stay under the ignored `build/` directory.

`-n -W --keep-going` checks references and treats warnings as build failures.
The documentation workflow runs the same strict build for relevant pull requests.

## Connect Read the Docs

1. Push the documentation and root `.readthedocs.yaml` to GitHub.
2. Import `EJainDev/SerialXML` in your Read the Docs account and select the branch
   containing these files.
3. Trigger a build. The supplied configuration uses Ubuntu 24.04 and Python 3.13,
   installs `docs/requirements.txt`, and builds `docs/conf.py` with warnings treated
   as errors.
4. Set `READTHEDOCS_URL` near the bottom of `docs/conf.py` to your final documentation
   URL, for example `https://YOUR-PROJECT.readthedocs.io/en/latest/`. Hosted builds
   automatically use the canonical URL provided by Read the Docs.
5. Add the public URL to the repository's GitHub website field and README when it
   is available.

See the [Read the Docs configuration reference](https://docs.readthedocs.io/en/stable/config-file/v2.html)
for supported hosting options. The public URL is optional for local builds.

## Maintain the content

- The site follows [Diátaxis](https://diataxis.fr/). Put guided learning in
  `tutorials/`, task recipes in `how-to/`, technical contracts in reference pages,
  and conceptual discussion in `explanation/`. Each section has an `index.md`
  landing page; add new pages to exactly one section's toctree so breadcrumbs,
  sidebar context, and previous/next links agree.
- Keep existing page URLs and section anchors working when reorganizing content.
  `examples.md` retains the original cookbook anchors and links to the new recipes.
- Tutorials should include prerequisites, complete code, expected output, and a
  next step. Programs and checkpoints for the project lessons live in
  `_snippets/tutorials/`; compile and run each checkpoint when editing a lesson. How-to guides should begin with a task and link to the relevant
  reference rather than repeat its contract.
- `api.rst` is an indexed, curated reference to the exported symbols in
  `include/serial_xml.cxx`. Update its signatures and behavior descriptions when
  the public API changes. The reference does not require a reflection-aware
  Doxygen parser or a C++ compilation step.
- Recipes under `how-to/` include complete, focused programs from
  `_snippets/how-to/` in portions using `literalinclude`. Keep the imports,
  declarations, and complete `main` visible; compile each program and compare its
  output with the documented result after edits. If formatting changes line
  positions, update the included ranges too.
- `examples.md` includes the repository's actual hello-world source. Shared CMake
  setup lives in `_snippets/CMakeLists.txt` to keep installation and the tutorial
  aligned.
- The homepage reuses `assets/SerialXML.png`, the README banner. Theme colors
  in `conf.py` and button accents in `_static/custom.css` follow its blue, cyan,
  and navy palette.
- `_templates/page.html` adds section navigation and breadcrumbs to Furo.
  `_static/custom.css` styles the landing cards, navigation, and light/dark themes.
  Keep navigation usable without JavaScript and test narrow screens and keyboard focus.
- `requirements.txt` pins the documentation dependencies and their dependencies.
  Update and verify them together when upgrading Sphinx or the theme.
