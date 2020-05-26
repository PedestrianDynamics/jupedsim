/**
 * \file        AgentsSourcesManager.cpp
 * \date        Apr 14, 2015
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
 * This class is responsible for materialising agent in a given location at a given frequency up to a maximum number.
 * The optimal position where to put the agents is given by various algorithms, for instance
 * the Voronoi algorithm or the Mitchell Best candidate algorithm.
 *
 **/
#include "AgentsSourcesManager.h"

#include "AgentsQueue.h"
#include "Pedestrian.h"
#include "neighborhood/NeighborhoodSearch.h"
#include "voronoi-boost/VoronoiPositionGenerator.h"

#include <Logger.h>
#include <thread>

bool AgentsSourcesManager::_isCompleted = true;

AgentsSourcesManager::AgentsSourcesManager()
{
    //Generate all agents required for the complete simulation
    //It might be more efficient to generate at each frequency step
    GenerateAgents();
    //first call ignoring the return value
    ProcessAllSources();

    //the loop is updated each x second.
    //it might be better to use a timer
    SetBuildingUpdated(false);
}

AgentsSourcesManager::~AgentsSourcesManager() {}


bool AgentsSourcesManager::ProcessAllSources() const
{
    bool empty          = true;
    double current_time = Pedestrian::GetGlobalTime();
    std::vector<Pedestrian *>
        source_peds; // we have to collect peds from all sources, so that we can consider them  while computing new positions
    for(const auto & src : _sources) {
        auto srcLifeSpan = src->GetLifeSpan();
        bool inTime      = (current_time >= srcLifeSpan[0]) && (current_time <= srcLifeSpan[1]);
        // inTime is always true if src got some PlanTime (default values
        // if src has no PlanTime, then this is set to 0. In this case inTime
        // is important in the following condition
        bool frequencyTime = std::fmod(current_time - srcLifeSpan[0], src->GetFrequency()) ==
                             0; // time of creation wrt frequency
        bool newCycle = almostEqual(current_time, srcLifeSpan[0], 1.e-5) || frequencyTime;
        bool subCycle;
        int quotient      = (int) (current_time - srcLifeSpan[0]) / (int) src->GetFrequency();
        int timeReference = src->GetFrequency() * quotient;
        subCycle =
            (current_time > srcLifeSpan[0]) ?
                std::fmod(current_time - timeReference - srcLifeSpan[0], src->GetRate()) == 0 :
                false;

        if(newCycle)
            src->ResetRemainingAgents();

        bool timeToCreate = newCycle || subCycle;
        LOG_DEBUG(
            "timeToCreate: {} pool size: {} plan time < current time: {} inTime: {} "
            "remainingAgents: {}",
            timeToCreate,
            src->GetPoolSize(),
            (src->GetPlanTime() <= current_time),
            inTime,
            src->GetRemainingAgents());
        if(timeToCreate && src->GetPoolSize() && (src->GetPlanTime() <= current_time) && inTime &&
           src->GetRemainingAgents()) // maybe diff<eps
        {
            std::vector<Pedestrian *> peds;
            src->RemoveAgentsFromPool(peds, src->GetChunkAgents() * src->GetPercent());
            src->UpdateRemainingAgents(src->GetChunkAgents() * src->GetPercent());
            source_peds.reserve(source_peds.size() + peds.size());
            LOG_INFO(
                "Source {:d} generating {:d} agents at {:3.3f}s, {:d} ({:d} remaining in pool)",
                src->GetId(),
                peds.size(),
                current_time,
                src->GetRemainingAgents(),
                src->GetPoolSize());

            if(!std::isnan(src->GetStartX()) && !std::isnan(src->GetStartY())) {
                LOG_INFO(
                    "Set source agent on fixed position ({:.2f}, {:.2f})",
                    src->GetStartX(),
                    src->GetStartY());
                InitFixedPosition(src.get(), peds);
            } else if(!ComputeBestPositionVoronoiBoost(src.get(), peds, _building, source_peds))
                LOG_WARNING("There was no place for some pedestrians");

            // Having set the positions, now we can set the velocity
            for(auto ped : peds) {
                AdjustVelocityUsingWeidmann(ped);
            }
            source_peds.insert(source_peds.end(), peds.begin(), peds.end());
            AgentsQueueIn::Add(peds);
            empty = false;
        }
        bool timeConstraint =
            (src->GetPlanTime() > current_time) || (current_time < srcLifeSpan[1]);
        if(timeConstraint) // for the case we still expect
            // agents coming
            empty = false;
    }
    return empty;
}


void AgentsSourcesManager::InitFixedPosition(AgentsSource * src, std::vector<Pedestrian *> & peds)
    const
{
    for(auto && ped : peds) {
        ped->SetPos(Point(src->GetStartX(), src->GetStartY()));
    }
}


void AgentsSourcesManager::AdjustVelocityByNeighbour(Pedestrian * ped) const
{
    //get the density
    std::vector<Pedestrian *> neighbours = _building->GetNeighborhoodSearch().GetNeighbourhood(ped);

    double speed         = 0.0;
    double radius_square = 0.56 * 0.56; //corresponding to an area of 1m3
    int count            = 0;

    for(const auto & p : neighbours) {
        //only pedestrians in a specific range
        if((ped->GetPos() - p->GetPos()).NormSquare() <= radius_square) {
            //only peds with the same destination
            if(ped->GetExitIndex() == p->GetExitIndex()) {
                double dist1 = ped->GetDistanceToNextTarget();
                double dist2 = p->GetDistanceToNextTarget();
                //only peds in front of me
                if(dist2 < dist1) {
                    speed += p->GetV().Norm();
                    count++;
                }
            }
        }
    }
    //mean speed
    if(count == 0) {
        speed = ped->GetEllipse().GetV0(); // FIXME:  bad fix for: peds without navline (ar.graf)
                                           //speed=ped->GetV0Norm();
    } else {
        speed = speed / count;
    }

    if(ped->FindRoute() != -1) {
        //get the next destination point
        Point v = (ped->GetExitLine()->ShortestPoint(ped->GetPos()) - ped->GetPos()).Normalized();
        v       = v * speed;
        ped->SetV(v);
    } else {
        LOG_ERROR(
            "No route could be found for agent {:d} going to {:d}",
            ped->GetID(),
            ped->GetFinalDestination());
        //that will be most probably be fixed in the next computation step.
        // so do not abort
    }
}

void AgentsSourcesManager::AdjustVelocityUsingWeidmann(Pedestrian * ped) const
{
    //get the density
    std::vector<Pedestrian *> neighbours = _building->GetNeighborhoodSearch().GetNeighbourhood(ped);

    //density in pers per m2
    double density = 1.0;
    //radius corresponding to a surface of 1m2
    double radius_square = 1.0;

    for(const auto & p : neighbours) {
        if((ped->GetPos() - p->GetPos()).NormSquare() <= radius_square)
            density += 1.0;
    }
    density = density / (radius_square * M_PI);

    //get the velocity
    double density_max = 5.4;

    //speed from taken from weidmann FD
    double speed = 1.34 * (1 - exp(-1.913 * (1.0 / density - 1.0 / density_max)));
    if(speed >= ped->GetV0Norm()) {
        speed = ped->GetV0Norm();
    }

    //set the velocity vector
    if(ped->FindRoute() != -1) {
        //get the next destination point
        Point v = (ped->GetExitLine()->ShortestPoint(ped->GetPos()) - ped->GetPos()).Normalized();
        v       = v * speed;
        ped->SetV(v);
    } else {
        LOG_WARNING(
            "No route could be found for source-agent {:d} going to {:d}",
            ped->GetID(),
            ped->GetFinalDestination());
        //that will be most probably be fixed in the next computation step.
        // so do not abort
    }
}

void AgentsSourcesManager::GenerateAgents()
{
    for(const auto & src : _sources) {
        LOG_INFO("Generate src: {}", src->GetId());
        src->GenerateAgentsAndAddToPool(src->GetMaxAgents(), _building);
    }
}

void AgentsSourcesManager::AddSource(std::shared_ptr<AgentsSource> src)
{
    _sources.push_back(src);
    _isCompleted = false; //at least one source was provided
}

const std::vector<std::shared_ptr<AgentsSource>> & AgentsSourcesManager::GetSources() const
{
    return _sources;
}

void AgentsSourcesManager::SetBuilding(Building * building)
{
    _building = building;
}

bool AgentsSourcesManager::IsCompleted() const
{
    const auto remaining_agents =
        std::accumulate(_sources.cbegin(), _sources.cend(), 0, [](auto sum, auto src) {
            return sum + src->GetRemainingAgents();
        });
    return remaining_agents == 0;
}


bool AgentsSourcesManager::IsBuildingUpdated() const
{
    return _buildingUpdated;
}

void AgentsSourcesManager::SetBuildingUpdated(bool update)
{
    _buildingUpdated = update;
}


Building * AgentsSourcesManager::GetBuilding() const
{
    return _building;
}

long AgentsSourcesManager::GetMaxAgentNumber() const
{
    long pop = 0;
    for(const auto & src : _sources) {
        pop += src->GetMaxAgents();
    }
    return pop;
}


int AgentsSourcesManager::GetMaxSimTime() const
{
    return maxSimTime;
}
void AgentsSourcesManager::SetMaxSimTime(int t)
{
    maxSimTime = t;
}
