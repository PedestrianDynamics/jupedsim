---
date: "2026-09-28"
author: "JuPedSim team"
description: "How to write articles and notes for jupedsim.org, and a live reference of the markup the site supports."
---

<!-- SPDX-License-Identifier: LGPL-3.0-or-later -->

# Writing for jupedsim.org

jupedsim.org is a Sphinx project in the `site/` folder of the
[JuPedSim repository](https://github.com/PedestrianDynamics/jupedsim). Articles
live in `site/articles/`, notes in `site/notes/`. Both are MyST Markdown (or
Jupyter notebooks for articles). This page is itself an article and shows every
piece of markup the site supports, so you can compare source and rendering.

## Articles and notes

**Articles** are longer, curated pieces. Add the file to `site/articles/` and
list it in the hidden toctree of `site/articles/index.md`; the order there is the
previous/next order. The list on `/articles/` itself is sorted by date, newest
first.

**Notes** are short, dated news items. Name the file
`site/notes/YYYY-MM-DD-slug.md`; the notes index picks it up automatically,
newest first, and the note appears in the RSS feed at `/notes/feed.xml` and on
the landing page.

## Front matter

Every Markdown article and note starts with YAML front matter. All three keys are
required and are plain strings:

```yaml
---
date: "2026-09-28"          # ISO date, used for sorting and the feed
author: "Jane Doe"
description: "One sentence shown in listings and the feed."
---
```

The first heading of the document is its title. Keep exactly one top-level
heading per page. A missing or invalid `date` fails the build.


## Notebook articles

An article can also be a Jupyter notebook (`.ipynb`). Its front matter goes
into the notebook metadata instead of a Markdown cell, as top-level keys:

```json
{
  "metadata": {
    "date": "2026-09-28",
    "author": "Jane Doe",
    "description": "One sentence shown in listings and the feed.",
    "kernelspec": {"name": "python3", "display_name": "Python 3"}
  }
}
```

The first Markdown cell starts with the `# Title`. Notebooks stored without
outputs are executed during the build (this needs an importable `jupedsim`;
the timeout is 900 s per notebook), notebooks with outputs are used as they
are. The published site executes notebooks against the latest `jupedsim`
release on PyPI, not against master; commit the outputs of an article that
needs unreleased features. Interactive outputs such as plotly figures are
not supported and fail the build; save them as static images instead.
## Headings

Use `##` for sections and `###` for subsections; they appear in the
"On this page" table of contents.

### A subsection

Deeper levels work too, but rarely help the reader.

## Text

Paragraphs are separated by a blank line. Use *emphasis*, **strong**,
`inline code` and [links](https://www.jupedsim.org/). Link to other pages of the
site with relative paths, e.g. [the notes](../notes/index.md), and to the
documentation (which is a separate project) with an HTML link and an
absolute path, `<a href="/docs/stable/">/docs/stable/</a>`, which renders as
<a href="/docs/stable/">/docs/stable/</a>.

- Bullet lists
- use `-`

1. Numbered lists
2. use `1.`

## Admonitions

````md
```{note}
Something the reader should know.
```
````

```{note}
Something the reader should know.
```

```{tip}
A helpful shortcut.
```

```{warning}
Something that can go wrong.
```

:::{important}
With the `colon_fence` extension, `:::` works as a fence as well.
:::

## Code

Fenced code blocks carry a language label that selects the highlighting:

```python
import jupedsim as jps
import shapely

area = shapely.Polygon([(0, 0), (10, 0), (10, 10), (0, 10)])
simulation = jps.Simulation(model=jps.CollisionFreeSpeedModel(), geometry=area)
```

```bash
pip install jupedsim
```

```{code-block} python
:caption: A code block with a caption
:emphasize-lines: 2

for _ in range(100):
    simulation.iterate()
```

## Tables

| Model | Kind | Since |
|-------|------|-------|
| Collision-free speed | velocity based | 1.0 |
| Social force | force based | 1.1 |
| Anticipation velocity | velocity based | 1.3 |

## Math

Inline math uses dollars, e.g. $v_i = \frac{\Delta x_i}{\Delta t}$. Display
math uses double dollars:

$$
\vec{F}_i = m_i \frac{\vec{v}_i^0 - \vec{v}_i}{\tau} + \sum_{j \neq i} \vec{F}_{ij}
$$

Give an equation a label to number it and refer to it with
`` {eq}`label` ``:

````md
```{math}
:label: velocity-update

\vec{v}_i(t + \Delta t) = \vec{v}_i(t) + \frac{\vec{F}_i}{m_i} \Delta t
```
````

```{math}
:label: velocity-update

\vec{v}_i(t + \Delta t) = \vec{v}_i(t) + \frac{\vec{F}_i}{m_i} \Delta t
```

Equation {eq}`velocity-update` is the explicit Euler step. Multi-line
equations use amsmath environments directly in the Markdown:

\begin{align*}
x_i(t + \Delta t) &= x_i(t) + v_i \Delta t \\
y_i(t + \Delta t) &= y_i(t) + w_i \Delta t
\end{align*}

Math is rendered to MathML at build time; no external script is loaded. The
renderer covers common LaTeX math; `\label` and `\tag` inside the formula
are not supported (use the `:label:` option), and anything it cannot render
is reported as a build warning with the file and line.

## Figures

```{figure} ../_static/jupedsim.svg
:alt: The JuPedSim logo
:width: 160px

A figure with a caption. Put images next to the article or in `site/_static/`.
```

## Footnotes

Footnotes use labels[^label] and are collected at the end of the page[^2].

[^label]: This is the first footnote.
[^2]: And this is the second one.
