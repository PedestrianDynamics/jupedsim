---
date: "2026-09-28"
author: "JuPedSim team"
description: "jupedsim.org now has articles and notes; the documentation moves to /docs/ and the 1.x docs are archived."
---

<!-- SPDX-License-Identifier: LGPL-3.0-or-later -->

# A new jupedsim.org

jupedsim.org used to be the versioned documentation and nothing else. The site
now has a home for content that does not change with every release:
[articles](../articles/index.md) and notes like this one, with an
<a href="/notes/feed.xml">RSS feed</a>.

## What moved where

- **Landing page, articles and notes** are at the root: `/`, `/articles/`,
  `/notes/`. They are built from the `master` branch.
- **Documentation** of the 2.x releases is published under
  `/docs/<version>/`; <a href="/docs/stable/">/docs/stable/</a> always points to the
  latest release. Until 2.0.0 is released, `/docs/stable/` forwards to the
  archived 1.x documentation.
- **The 1.x documentation** (v1.0.0 to v1.4.2) is frozen and archived
  unchanged at <a href="/docs/v1x/v1.4.2/">/docs/v1x/v1.4.2/</a>. Old links to pages
  such as `/stable/...` or `/v1.4.2/...` keep working and redirect there.

## Intersphinx users

If your project links to the 1.x API with `sphinx.ext.intersphinx`, point the
mapping to the archive:

```python
intersphinx_mapping = {
    "jupedsim": ("https://www.jupedsim.org/docs/v1x/v1.4.2/", None),
}
```

Once 2.0.0 is released, projects using the 2.x API map `jupedsim` to
`https://www.jupedsim.org/docs/stable/` instead.
