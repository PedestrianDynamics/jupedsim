# SPDX-License-Identifier: LGPL-3.0-or-later
from typing import Protocol


class _Named(Protocol):
    @property
    def id(self) -> int: ...

    @property
    def name(self) -> str | None: ...


def id_and_name(handle: _Named) -> str:
    """Id of ``handle``, followed by its name in parentheses if it has one,
    e.g. ``3 (wp1)``, as the messages from C++ name it. Messages put the kind
    in front: ``Target 3 (wp1) belongs to another simulation``."""
    if handle.name is None:
        return str(handle.id)
    return f"{handle.id} ({handle.name})"
