/**
 * \file        RoutingEngine.cpp
 * \date        Jan 10, 2013
 * \version     v0.7
 * \copyright   <2009-2015> Forschungszentrum Jülich GmbH. All rights reserved.
 *
 * \section License
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * \section Description
 *
 *
 **/
#include "RoutingEngine.h"

#include "pedestrian/Pedestrian.h"

#include <Logger.h>
#include <memory>
#include <utility>

void RoutingEngine::UpdateTime(double time)
{
    for(auto && [_, r] : _routers) {
        r->UpdateTime(time);
    }
}

void RoutingEngine::SetSimulation(Simulation * simulation)
{
    for(auto && [_, r] : _routers) {
        r->SetSimulation(simulation);
    }
}

void RoutingEngine::AddRouter(int id, Router * router)
{
    if(const auto [_, success] = _routers.try_emplace(id, std::unique_ptr<Router>(router));
       !success) {
        LOG_ERROR("Duplicate router found with 'id' [{:d}].", id);
        exit(EXIT_FAILURE);
    }
}

Router * RoutingEngine::GetRouter(int id) const
{
    const auto iter = _routers.find(id);
    if(iter == _routers.end()) {
        LOG_ERROR("Could not Find any router with ID:  [{:d}].", id);
        return nullptr;
    }
    return iter->second.get();
}

bool RoutingEngine::Init(Building * building)
{
    bool status = false;
    for(auto && [_, r] : _routers) {
        status &= r->Init(building);
    }
    return status;
}

bool RoutingEngine::NeedsUpdate() const
{
    return _needUpdate;
}

void RoutingEngine::setNeedUpdate(bool needUpdate)
{
    _needUpdate = needUpdate;
}

void RoutingEngine::UpdateRouter()
{
    for(auto && [_, r] : _routers) {
        r->Update();
    }
    _needUpdate = false;
}
