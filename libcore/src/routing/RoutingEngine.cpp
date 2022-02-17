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
#include "RoutingEngine.hpp"

#include "geometry/Building.hpp"
#include "math/OperationalModel.hpp"
#include "pedestrian/Pedestrian.hpp"
#include "routing/RoutingStrategy.hpp"
#include "routing/ff_router/ffRouter.hpp"
#include "routing/global_shortest/GlobalRouter.hpp"

#include <Logger.hpp>
#include <memory>
#include <stdexcept>
#include <utility>

RoutingEngine::RoutingEngine(
    Configuration * config,
    Building * building,
    DirectionManager * directionManager)
{
    auto buildRouter = [config, building, directionManager](
                           const auto & strategy_info) -> std::unique_ptr<Router> {
        const auto & [strategy, parameters] = strategy_info;
        switch(strategy) {
            case RoutingStrategy::ROUTING_FF_GLOBAL_SHORTEST:
                return std::make_unique<FFRouter>(config, building, directionManager);
            case RoutingStrategy::ROUTING_GLOBAL_SHORTEST:
                return std::make_unique<GlobalRouter>(building, *parameters);
            case RoutingStrategy::UNKNOWN:
                throw std::logic_error("Unexpected RoutingStrategy encountered");
        }
    };
    for(const auto & [id, strategy_info] : config->routingStrategies) {
        if(const auto & [strategy, parameters] = strategy_info;
           strategy == RoutingStrategy::ROUTING_GLOBAL_SHORTEST) {
            for(auto && l : (*parameters).optionalNavLines) {
                building->AddHline(l);
            }
        }
        _routers.emplace(id, buildRouter(strategy_info));
    }
}

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

Router * RoutingEngine::GetRouter(int id) const
{
    const auto iter = _routers.find(id);
    if(iter == _routers.end()) {
        LOG_ERROR("Could not Find any router with ID:  [{:d}].", id);
        return nullptr;
    }
    return iter->second.get();
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
