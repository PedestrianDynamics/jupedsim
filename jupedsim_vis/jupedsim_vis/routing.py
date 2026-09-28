# SPDX-License-Identifier: LGPL-3.0-or-later
"""Optional shortest path support backed by jupedsim.

This is the only module of the package that may import jupedsim, and it
imports it lazily inside functions: the viewer works without jupedsim, it
just cannot preview shortest paths. Every failure is turned into a human
readable reason for the status bar instead of an exception.
"""

from dataclasses import dataclass
from typing import Any, Protocol

_PREFIX = "Cannot calculate shortest path: "

_REQUIRED_METHODS = ("is_routable", "compute_waypoints")


class Router(Protocol):
    """The part of ``jupedsim.RoutingEngine`` the viewer relies on."""

    def is_routable(self, p: tuple[float, float]) -> bool: ...

    def compute_waypoints(
        self, frm: tuple[float, float], to: tuple[float, float]
    ) -> list[tuple[float, float]]: ...


@dataclass(frozen=True)
class Routing:
    """A router, or the reason why there is none."""

    router: Router | None
    unavailable_reason: str | None

    @property
    def available(self) -> bool:
        return self.router is not None


def check_available() -> str | None:
    """Check whether routing could work at all, without any geometry.

    Returns ``None`` if jupedsim provides a usable ``RoutingEngine``, else
    the reason why it does not.
    """
    _, reason = _routing_engine_class()
    return reason


def create_router(geometry: Any) -> Routing:
    """Build a router for ``geometry``. Never raises.

    ``geometry`` is passed to ``jupedsim.RoutingEngine`` unchanged; the
    released engine accepts a shapely ``Polygon``, ``MultiPolygon`` or
    ``GeometryCollection``.
    """
    engine_class, reason = _routing_engine_class()
    if engine_class is None:
        return Routing(router=None, unavailable_reason=reason)

    try:
        router = engine_class(geometry)
    except Exception as e:
        return Routing(
            router=None,
            unavailable_reason=(
                f"{_PREFIX}jupedsim cannot route this geometry ({e})."
            ),
        )
    return Routing(router=router, unavailable_reason=None)


def _routing_engine_class() -> tuple[Any | None, str | None]:
    """Import jupedsim and look up a usable ``RoutingEngine``.

    Returns ``(class, None)`` on success and ``(None, reason)`` otherwise.
    """
    try:
        import jupedsim
    except ImportError:
        return None, f"{_PREFIX}jupedsim is not installed."
    except Exception as e:
        return None, f"{_PREFIX}jupedsim could not be imported ({e})."

    try:
        version = getattr(jupedsim, "__version__", "?")
        engine_class = getattr(jupedsim, "RoutingEngine", None)
        if engine_class is None:
            return None, (
                f"{_PREFIX}the installed jupedsim {version} does not "
                f"provide RoutingEngine."
            )
        for method in _REQUIRED_METHODS:
            if not hasattr(engine_class, method):
                return None, (
                    f"{_PREFIX}the installed jupedsim {version} does not "
                    f"provide RoutingEngine.{method}()."
                )
    except Exception as e:
        return None, f"{_PREFIX}jupedsim could not be imported ({e})."

    return engine_class, None
