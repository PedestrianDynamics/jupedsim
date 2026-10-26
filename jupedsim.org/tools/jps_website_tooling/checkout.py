# SPDX-License-Identifier: LGPL-3.0-or-later
"""Check a checkout path given on the command line.

Shared by the subcommands that work in a git checkout (``publish``, ``tag``,
``resolve``, ``preview``); it only looks at paths, never at git.
"""

from __future__ import annotations

from pathlib import Path

from jps_website_tooling.errors import UsageError


def check_checkout(path: Path, build: Path | None = None) -> Path:
    """Check a checkout path given on the command line.

    The publishers reset and clean the Pages checkout, so their build must
    not lie inside it. This package is not checked: ``tag`` runs against the
    source checkout, which holds the package when it is used uninstalled;
    the publishers' guard (``check_pages_checkout``) refuses the source
    repository by its layout.

    Args:
        path: The checkout as given on the command line.
        build: The build directory, which must not lie inside the checkout.

    Returns:
        The resolved checkout path.

    Raises:
        UsageError: If the checkout does not exist or ``build`` lies inside
            it (``git clean`` would delete it).
    """
    if not path.is_dir():
        raise UsageError(f"checkout {path} does not exist")
    checkout = path.resolve()
    if build is not None and build.resolve().is_relative_to(checkout):
        raise UsageError(
            f"the build directory ({build}) lies inside the checkout "
            f"{checkout}; git clean would delete it. Check out the published "
            "branch next to the source and the build, not around them."
        )
    return checkout
