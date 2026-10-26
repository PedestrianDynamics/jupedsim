# SPDX-License-Identifier: LGPL-3.0-or-later
"""``python -m jps_website_tooling``: the console script without an install."""

from jps_website_tooling.cli import main

raise SystemExit(main())
