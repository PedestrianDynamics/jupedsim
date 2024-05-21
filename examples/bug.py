#! /usr/bin/env python3

# Copyright © 2012-2024 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later
import math
import pathlib

import jupedsim as jps


def main():
    jps.set_debug_callback(lambda x: print(x))
    jps.set_info_callback(lambda x: print(x))
    jps.set_warning_callback(lambda x: print(x))
    jps.set_error_callback(lambda x: print(x))

    geo_file = pathlib.Path("geometry") / "small.wkt"
    router = jps.RoutingEngine(geo_file.read_text())

    # buggy path
    frm_b, to_b = (
        (70.19708657860755, 20.035852912068368),
        (74.0455438733101, 19.591800147294997),
    )

    def length_of_path(p):
        length = 0
        for s, d in zip(p[:-1], p[1:]):
            dx = math.fabs(s[0] - d[0])
            dy = math.fabs(s[1] - d[1])
            length = length + math.sqrt(dx**2 + dy**2)
        return length

    res = router.compute_considered_waypoints(frm_b, to_b)
    print("BUGGY PATH")
    print(f"#paths = {len(res)}")
    for p in res:
        print("############")
        print(f"#vertices in path = {len(p)}")
        print(f"length of path = {length_of_path(p)}")
        print(p)
        print("========")


if __name__ == "__main__":
    main()
