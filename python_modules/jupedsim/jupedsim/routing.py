# SPDX-License-Identifier: LGPL-3.0-or-later

import jupedsim.native as py_jps

# Alias the routing engines.
RoutingEngine = py_jps.RoutingEngine
AStarRoutingEngine = py_jps.AStarRoutingEngine


class DirectPathRoutingEngine(RoutingEngine):
    """Trivial routing engine that returns a straight line ignoring geometry.

    Useful as a reference implementation and for open-space scenarios where
    agents should walk directly towards their destination.
    """

    def name(self) -> str:
        return "DirectPath"

    def set_geometry(self, _) -> None:
        pass

    def compute_waypoints(
        self,
        frm: tuple[float, float],
        to: tuple[float, float],
    ) -> list[tuple[float, float]]:
        return [frm, to]

    def is_routable(self, _: tuple[float, float]) -> bool:
        return True
