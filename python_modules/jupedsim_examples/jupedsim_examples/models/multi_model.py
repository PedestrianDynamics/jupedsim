# SPDX-License-Identifier: LGPL-3.0-or-later
from __future__ import annotations

from dataclasses import dataclass, replace

from jupedsim.models.custom_model import (
    CustomModelAgentState,
    CustomOperationalModel,
)
from jupedsim.models.collision_free_speed import CollisionFreeSpeedModel

from jupedsim_examples.models.pysocial_force import (
    PythonSocialForceModel,
    PythonSocialForceModelState,
)
from jupedsim.native import CollisionFreeSpeedModelState


@dataclass
class MultiModelAgentState(CustomModelAgentState):
    position: tuple[float, float]
    model_type: str
    state: Any


class MultiModel(CustomOperationalModel):
    def __init__(self):
        super().__init__()
        self._routes = {
            "SFM": PythonSocialForceModel(),
            "CFSM": CollisionFreeSpeedModel(),
        }

    def repack_neighbors(self, ini_list, state_type):
        from dataclasses import fields

        new_list = []
        for ini_neighbor in ini_list:
            ini_neighbor = ini_neighbor.state
            print(ini_neighbor)
            if type(ini_neighbor) == state_type:
                new_list.append(ini_neighbor)
                continue
            new_state = state_type(position=ini_neighbor.position)
            
            for field in dir(new_state):
                
                if field.startswith("_") or "position" == field:
                    continue
                print(field)
                if hasattr(ini_neighbor, field):
                    setattr(
                        new_state, field, getattr(ini_neighbor, field)
                    )
            print(new_state)
            new_list.append(new_state)
        return new_list

    def compute_next_state(
        self, dt, state, destination, geometry, neighbor_states
    ):
        model_type = state.model_type
        sub_state = state.state
        print("Model: ", model_type, "Before Repack, sum of agents ", len(neighbor_states))
        new_states = self.repack_neighbors(neighbor_states, type(state.state))
        print(self._routes[model_type])
        new_sub_state = self._routes[model_type].compute_next_state(
            dt, sub_state, destination, geometry, new_states
        )
        print("New State: ",replace(
            state, state=new_sub_state, position=new_sub_state.position
        ))
        return replace(
            state, state=new_sub_state, position=new_sub_state.position
        )

    def check_model_constraint(self, agent, neighborhood_search, geometry):
        pass
