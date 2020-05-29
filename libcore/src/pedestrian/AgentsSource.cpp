/**
 * \file        AgentSource.cpp
 * \date        Apr 04, 2015
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
#include "AgentsSource.h"

#include "Pedestrian.h"

#include <Logger.h>

AgentsSource::AgentsSource(
    int id,
    std::string caption,
    int max_agents,
    int group_id,
    int frequency,
    bool greedy,
    double time,
    int agent_id,
    float startx,
    float starty,
    float percent,
    float rate,
    int chunkAgents,
    std::vector<float> boundaries,
    std::vector<int> lifeSpan) :
    _id(id),
    _frequency(frequency),
    _maxAgents(max_agents),
    _groupID(group_id),
    _caption(std::move(caption)),
    _greedy(greedy),
    _agent_id(agent_id),
    _time(time),
    _startx(startx),
    _starty(starty),
    _chunkAgents(chunkAgents),
    _percent(percent),
    _rate(rate)
{
    _remainingAgents = _chunkAgents;
    _agentsGenerated = 0;
    _boundaries      = boundaries;
    _lifeSpan        = lifeSpan;
    _agents.clear();
}

AgentsSource::~AgentsSource() {}

void AgentsSource::GenerateAgentsAndAddToPool(int count, Building * building)
{
    std::vector<Pedestrian *> peds;
    GenerateAgents(peds, count, building);
    _agents.insert(_agents.begin(), peds.begin(), peds.end());
    _agentsGenerated += count;
}

void AgentsSource::RemoveAgentsFromPool(std::vector<Pedestrian *> & ped, int count)
{
    if((int) _agents.size() >= count) {
        ped.insert(ped.begin(), _agents.begin(), _agents.begin() + count);
        _agents.erase(_agents.begin(), _agents.begin() + count);
    } else {
        ped.insert(ped.begin(), _agents.begin(), _agents.end());
        _agents.clear();
    }
}

void AgentsSource::AddAgentsToPool(std::vector<Pedestrian *> & peds)
{
    _agents.insert(_agents.begin(), peds.begin(), peds.end());
}

bool AgentsSource::Greedy() const
{
    return _greedy;
}
int AgentsSource::GetPoolSize() const
{
    return (int) _agents.size();
}

void AgentsSource::AddToPool(Pedestrian * ped)
{
    _agents.push_back(ped);
}

int AgentsSource::GetAgentsGenerated() const
{
    return _agentsGenerated;
}

void AgentsSource::SetAgentsGenerated(int agentsGenerated)
{
    _agentsGenerated = agentsGenerated;
}

const std::vector<float> AgentsSource::GetBoundaries() const
{
    return _boundaries;
}

void AgentsSource::Setboundaries(std::vector<float> bounds)
{
    _boundaries = bounds;
}

const std::string & AgentsSource::GetCaption() const
{
    return _caption;
}

int AgentsSource::GetFrequency() const
{
    return _frequency;
}

int AgentsSource::GetGroupId() const
{
    return _groupID;
}

int AgentsSource::GetId() const
{
    return _id;
}

int AgentsSource::GetAgentId() const
{
    return _agent_id;
}

double AgentsSource::GetPlanTime() const
{
    return _time;
}


int AgentsSource::GetMaxAgents() const
{
    return _maxAgents;
}

int AgentsSource::GetChunkAgents() const
{
    return _chunkAgents;
}

int AgentsSource::GetRemainingAgents() const
{
    return _remainingAgents;
}

void AgentsSource::ResetRemainingAgents()
{
    _remainingAgents = _chunkAgents;
}

void AgentsSource::UpdateRemainingAgents(int remaining)
{
    _remainingAgents = (remaining < _remainingAgents) ? _remainingAgents - remaining : 0;
}
float AgentsSource::GetPercent() const
{
    return _percent;
}
float AgentsSource::GetRate() const
{
    return _rate;
}
std::vector<int> AgentsSource::GetLifeSpan() const
{
    return _lifeSpan;
}

float AgentsSource::GetStartX() const
{
    return _startx;
}

float AgentsSource::GetStartY() const
{
    return _starty;
}


void AgentsSource::SetStartDistribution(std::shared_ptr<StartDistribution> startDistribution)
{
    _startDistribution = startDistribution;
}

const std::shared_ptr<StartDistribution> AgentsSource::GetStartDistribution() const
{
    return _startDistribution;
}

void AgentsSource::GenerateAgents(std::vector<Pedestrian *> & peds, int count, Building * building)
{
    std::vector<Point> emptyPositions;
    // TODO: this vector is empty with purpose!
    //       1. Source-Pedestrians are created without setting their positions.
    //       2. In a second step (in AgentsSourcesManager::ProcessAllSources())
    //          the positions will be calculated and initializes.
    // TODO: We should reverse the order of points 1 and 2.
    int pid;
    pid = (this->GetAgentId() >= 0) ?
              this->GetAgentId() :
              Pedestrian::GetAgentsCreated() + building->GetAllPedestrians().size();
    for(int i = 0; i < count; i++) {
        if(GetStartDistribution()) {
            auto ped = GetStartDistribution()->GenerateAgent(building, &pid, emptyPositions);
            peds.push_back(ped);
        } else {
            std::cout << " \n Source: StartDistribution is null!\n"
                         " This happens when group_id in <source> does not much any group_id in "
                         "<agents>\n"
                         " Check again your inifile. If the problem persists report an issue\n";
            exit(EXIT_FAILURE);
        }
    }
}


void AgentsSource::Dump() const
{
    auto tmpB = this->GetBoundaries();
    auto tmpL = this->GetLifeSpan();
    LOG_DEBUG(
        "Dumping Source: Caption={} SourceID={:d}, GroupID={:d}, Frequency={:d}, AgentsMax={:d}, "
        "AgentsPool={:d}, AgentID={:d}, Time={:.2f}, Pos=({:.2f},{:.2f}), Percent={:.2f}, "
        "Rate={:.2f}, N_create={:d}, Boundaries=(XAxis=({:.4f}, {:.4f}), YAxis=({:.4f},{:.4f})), "
        "LifeSpan=({:d},{:d})",
        this->GetCaption(),
        _id,
        _groupID,
        _frequency,
        _maxAgents,
        _agents.size(),
        this->GetAgentId(),
        this->GetPlanTime(),
        this->GetStartX(),
        this->GetStartY(),
        this->GetPercent(),
        this->GetRate(),
        this->GetChunkAgents(),
        tmpB[0],
        tmpB[1],
        tmpB[2],
        tmpB[3],
        tmpL[0],
        tmpL[1]);
}
