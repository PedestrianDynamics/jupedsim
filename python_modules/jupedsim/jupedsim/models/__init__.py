# SPDX-License-Identifier: LGPL-3.0-or-later
"""Operational models of JuPedSim.

Every operational model is passed to a
:class:`~jupedsim.simulation.Simulation` as a configured instance carrying its
model-level parameters, e.g.
:class:`~jupedsim.models.social_force.SocialForceModel`,
:class:`~jupedsim.models.collision_free_speed.CollisionFreeSpeedModel` or
:class:`~jupedsim.models.warp_driver.WarpDriverModel`. Custom Python models are
passed as instances of a
:class:`~jupedsim.models.custom_model.CustomOperationalModel` subclass.
"""
