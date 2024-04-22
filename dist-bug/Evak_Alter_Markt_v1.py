#! /usr/bin/env python

# Copyright © 2012-2023 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later
import logging
import pathlib
import sys
import time

import shapely
from shapely import affinity

import jupedsim as jps


def log_debug(msg):
    logging.debug(msg)


def log_info(msg):
    logging.info(msg)


def log_warn(msg):
    logging.warning(msg)


def log_error(msg):
    logging.error(msg)


def main():
    logging.basicConfig(
        level=logging.DEBUG, format="%(levelname)s : %(message)s"
    )
    jps.set_debug_callback(log_debug)
    jps.set_info_callback(log_info)
    jps.set_warning_callback(log_warn)
    jps.set_error_callback(log_error)

    geo = shapely.from_wkt(
        pathlib.Path("Aufbau_Alter_Markt_JuPedSim.wkt").read_text()
    )

    cp = geo.centroid
    offset = (-cp.x, -cp.y)
    geo = affinity.translate(geo, offset[0], offset[1])

    simulation = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(
            strength_geometry_repulsion=35, range_geometry_repulsion=0.019
        ),
        dt=0.02,
        geometry=geo,
        trajectory_writer=jps.SqliteTrajectoryWriter(
            output_file=pathlib.Path("evac_5exits.sqlite"), every_nth_frame=1
        ),
    )

    WPs = [
        [x + offset[0], y + offset[1]]
        for [x, y] in [
            [32467900.00, 5763410.00],  # right side
        ]
    ]

    radius = [6.90]

    WP_ids = []
    for wp_info in zip(WPs, radius):
        WP_ids.append(simulation.add_waypoint_stage(wp_info[0], wp_info[1]))
        print(f"{WP_ids[-1]}: {wp_info[0]}")

    sys.exit(0)


if __name__ == "__main__":
    main()
