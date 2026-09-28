# SPDX-License-Identifier: LGPL-3.0-or-later
"""Tests for the optional jupedsim routing adapter.

The adapter is the only place in the package that may import jupedsim, and
it must degrade gracefully when jupedsim is missing, too old or broken.
Every failure mode is exercised here without jupedsim being installed; the
one test that needs the real package skips when it is absent.
"""

import dataclasses
import sys
import types

import pytest
from shapely.geometry import Polygon

from jupedsim_vis.routing import (
    Routing,
    check_available,
    create_router,
)

SQUARE = Polygon([(0, 0), (10, 0), (10, 10), (0, 10)])


class WorkingEngine:
    """A stand-in for ``jupedsim.RoutingEngine``."""

    def __init__(self, geometry):
        self.geometry = geometry

    def is_routable(self, p):
        return True

    def compute_waypoints(self, frm, to):
        return [frm, to]


def install_fake_jupedsim(monkeypatch, engine, version="9.9.9"):
    """Put a fake ``jupedsim`` module into ``sys.modules``."""
    module = types.ModuleType("jupedsim")
    module.__version__ = version
    if engine is not None:
        module.RoutingEngine = engine
    monkeypatch.setitem(sys.modules, "jupedsim", module)
    return module


def test_routing_available_reflects_router() -> None:
    assert Routing(router=None, unavailable_reason="nope").available is False
    assert Routing(router=object(), unavailable_reason=None).available is True


def test_missing_jupedsim_is_reported(monkeypatch) -> None:
    monkeypatch.setitem(sys.modules, "jupedsim", None)

    routing = create_router(SQUARE)

    assert routing.router is None
    assert routing.available is False
    assert (
        routing.unavailable_reason
        == "Cannot calculate shortest path: jupedsim is not installed."
    )


def test_check_available_reports_missing_jupedsim(monkeypatch) -> None:
    monkeypatch.setitem(sys.modules, "jupedsim", None)

    assert (
        check_available()
        == "Cannot calculate shortest path: jupedsim is not installed."
    )


def test_import_failing_with_other_error_is_reported(monkeypatch) -> None:
    class Raiser:
        def find_spec(self, name, path=None, target=None):
            if name == "jupedsim":
                raise RuntimeError("boom")
            return None

    monkeypatch.delitem(sys.modules, "jupedsim", raising=False)
    monkeypatch.setattr(sys, "meta_path", [Raiser(), *sys.meta_path])

    routing = create_router(SQUARE)

    assert routing.router is None
    assert routing.unavailable_reason == (
        "Cannot calculate shortest path: jupedsim could not be imported (boom)."
    )
    assert check_available() == routing.unavailable_reason


def test_module_without_routing_engine_is_reported(monkeypatch) -> None:
    install_fake_jupedsim(monkeypatch, None)

    routing = create_router(SQUARE)

    assert routing.router is None
    assert routing.unavailable_reason == (
        "Cannot calculate shortest path: the installed jupedsim 9.9.9 does "
        "not provide RoutingEngine."
    )
    assert check_available() == routing.unavailable_reason


def test_engine_without_compute_waypoints_is_reported(monkeypatch) -> None:
    class Engine:
        def __init__(self, geometry):
            pass

        def is_routable(self, p):
            return True

    install_fake_jupedsim(monkeypatch, Engine)

    routing = create_router(SQUARE)

    assert routing.router is None
    assert routing.unavailable_reason == (
        "Cannot calculate shortest path: the installed jupedsim 9.9.9 does "
        "not provide RoutingEngine.compute_waypoints()."
    )
    assert check_available() == routing.unavailable_reason


def test_engine_without_is_routable_is_reported(monkeypatch) -> None:
    class Engine:
        def __init__(self, geometry):
            pass

    install_fake_jupedsim(monkeypatch, Engine, version="1.2.3")

    routing = create_router(SQUARE)

    assert routing.router is None
    assert routing.unavailable_reason == (
        "Cannot calculate shortest path: the installed jupedsim 1.2.3 does "
        "not provide RoutingEngine.is_routable()."
    )


def test_unknown_version_is_rendered_as_question_mark(monkeypatch) -> None:
    module = types.ModuleType("jupedsim")
    monkeypatch.setitem(sys.modules, "jupedsim", module)

    routing = create_router(SQUARE)

    assert routing.unavailable_reason == (
        "Cannot calculate shortest path: the installed jupedsim ? does not "
        "provide RoutingEngine."
    )


def test_failing_constructor_is_reported(monkeypatch) -> None:
    class Engine:
        def __init__(self, geometry):
            raise ValueError("geometry is not simple")

        def is_routable(self, p):
            return True

        def compute_waypoints(self, frm, to):
            return []

    install_fake_jupedsim(monkeypatch, Engine)

    routing = create_router(SQUARE)

    assert routing.router is None
    assert routing.available is False
    assert routing.unavailable_reason == (
        "Cannot calculate shortest path: jupedsim cannot route this geometry "
        "(geometry is not simple)."
    )
    assert "cannot route this geometry" in routing.unavailable_reason
    # The import itself is fine, so the import level check stays happy.
    assert check_available() is None


def test_working_engine_yields_a_router(monkeypatch) -> None:
    install_fake_jupedsim(monkeypatch, WorkingEngine)

    routing = create_router(SQUARE)

    assert routing.unavailable_reason is None
    assert routing.available is True
    assert isinstance(routing.router, WorkingEngine)
    assert routing.router.geometry is SQUARE
    assert check_available() is None


def test_routing_is_frozen(monkeypatch) -> None:
    install_fake_jupedsim(monkeypatch, WorkingEngine)

    routing = create_router(SQUARE)

    with pytest.raises(dataclasses.FrozenInstanceError):
        routing.router = None


def test_real_jupedsim_routes_a_square() -> None:
    """The one test that needs the real jupedsim.

    ``importorskip`` rather than a ``find_spec`` guard: a jupedsim that is
    on ``sys.path`` but does not import (a source checkout without the
    compiled extension, say) has to skip this test, not fail it -- which is
    what ``exc_type=ImportError`` asks for, the default narrows to
    ``ModuleNotFoundError`` in pytest 9.1.
    """
    pytest.importorskip("jupedsim", exc_type=ImportError)

    routing = create_router(SQUARE)

    assert routing.unavailable_reason is None
    assert routing.available is True
    assert routing.router.is_routable((5, 5))
    assert len(routing.router.compute_waypoints((1, 1), (9, 9))) >= 2
