/**
 * \file        AgentSource.h
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
 * This class is responsible for materialising agent in a given location at a given frequency.
 * The optimal position where to put the agents is given by various algorithms, for instance
 * the Voronoi algorithm or the Mitchell Best candidate algorithm.
 *
 **/
#pragma once

#include <string>
#include <vector>
#include <memory>

class Pedestrian;
class OutputHandler;
class StartDistribution;
class Building;

// external variables
extern OutputHandler* Log;


class AgentsSource
{
public:
     /**
      * Constructor
      */
     AgentsSource(int id,
                  const std::string& caption,
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
                  std::vector<int> lifeSpan);

     /**
      * Destructor
      */
     virtual ~AgentsSource();

     /**
      * Add a new agent to this source
      * @param ped
      */
     void AddToPool(Pedestrian* ped);

     /**
      * Generate a number of agents, based on the frequency given in the constructor.
      * No agents are generated if the maximum (_maxAgents) is reached.
      * @see _maxAgents
      * @param ped
      */
     void GenerateAgentsAndAddToPool(int count, Building* building);

     /**
      * Generate agents, but do not add them to the pool
      * @param ped, the container for the agents
      * @param count, the number of agents to generate
      * @param building, a pointer to the building object
      */
     void GenerateAgents(std::vector<Pedestrian*>& peds, int count, Building* building);

     /**
      * Generate count agents and save them in the vector
      * @param ped, the container for the agents
      * @param count, the number of agents to generate
      */
     void RemoveAgentsFromPool(std::vector<Pedestrian*>& peds, int count);

     /**
      * Add the agents to the pool. This might be important in the case the removed agents could not
      * be placed correctly. They can be requeued using this function.
      * @param peds
      */
     void AddAgentsToPool(std::vector<Pedestrian*>& peds);

     /**
      * @return the number of agents remaining
      */
     int GetPoolSize() const;

     /**
      * Print relevant information
      */
     void Dump() const;

     int GetAgentsGenerated() const;
     void SetAgentsGenerated(int agentsGenerated);
     const std::vector<float> GetBoundaries() const;
     void Setboundaries(std::vector<float> bounds);
     const std::string& GetCaption() const;
     int GetFrequency() const;
     int GetGroupId() const;
     int GetId() const;
     int GetAgentId() const;
     float GetStartX() const;
     float GetStartY() const;
     double GetPlanTime() const;
     int GetMaxAgents() const;
     int GetChunkAgents() const;
     int GetRemainingAgents() const;
     void ResetRemainingAgents();
     void UpdateRemainingAgents(int remaining);
     float GetPercent() const;
     float GetRate() const;
     std::vector<int> GetLifeSpan() const;
     bool Greedy() const;
     void SetStartDistribution(std::shared_ptr<StartDistribution>);
     const std::shared_ptr<StartDistribution> GetStartDistribution() const;

private:
     int _id=-1;
     int _frequency=1; /// create \var _chunkAgents every \var _frequency seconds
     int _maxAgents=0;
     int _groupID=-1;
     std::string _caption="no caption";
     bool _greedy = false;
     int _agentsGenerated=0;
     std::vector<float> _boundaries;
     int _agent_id;
     double _time; /// planned generation time. here \var _maxAgents = 1
     float _startx; /// \var _maxAgents = 1
     float _starty; /// \var _maxAgents = 1
     std::vector<int> _lifeSpan;

     int _chunkAgents; /// generate \var chunk_agents per \var frequency seconds
     int _remainingAgents; /// After generating \var chunk_agents \time \var
                           /// _percent per \var frequency seconds, this is the
                           /// remaining of agents still to be produced
     float _percent=1.0; /// generate \var _percent * \var _chunkAgents
     float _rate=1.0;

     std::vector<Pedestrian*> _agents;
     std::shared_ptr<StartDistribution> _startDistribution;
};
