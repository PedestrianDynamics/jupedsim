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

    geo_file = pathlib.Path("geometry") / "turnstiles.wkt"
    router = jps.RoutingEngine(geo_file.read_text())

    # buggy path
    frm_b, to_b = (
        (70.09649309628304, 19.678671507447426),
        (75.7540758131708, 19.014848468665928),
    )

    # correct path
    frm_c, to_c = (
        (70.24613811286295, 19.681624559617),
        (75.89010451934591, 19.39324671403028),
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

    print("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~")

    res = router.compute_considered_waypoints(frm_c, to_c)
    print("CORRECT PATH")
    print(f"#paths = {len(res)}")
    for p in res:
        print("############")
        print(f"#vertices in path = {len(p)}")
        print(f"length of path = {length_of_path(p)}")
        print(p)
        print("========")


if __name__ == "__main__":
    main()
