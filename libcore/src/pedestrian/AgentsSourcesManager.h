/**
 * \file        AgentsSourcesManager.h
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
#pragma once

#include "AgentsSource.h"

#include <atomic>
#include <memory>
#include <vector>

//Forward declarations
class Building;
class Pedestrian;
class Point;

class AgentsSourcesManager
{
public:
    /**
      * Constructor
      */
    AgentsSourcesManager(Building * building);

    /**
      * disable copying
      */
    AgentsSourcesManager(const AgentsSourcesManager &) = delete;

    /**
      * Destructor
      */
    ~AgentsSourcesManager() = default;

    /**
      *  Add a new agent source
      */
    void AddSource(std::shared_ptr<AgentsSource> src);

    /**
      * @return true if all agents have been generated
      * and the class is ready to leave
      */
    bool IsCompleted() const;

    /**
      *Schedule the pedestrians for the simulation
      * @return true if all source are empty
      */
    std::vector<std::unique_ptr<Pedestrian>> ProcessAllSources(double current_time) const;

    /**
      * Trigger the sources to generate the specified
      * number of agents for this frequency
      */
    void GenerateAgents();

    /**
      * Return the total number of agents that will be generated.
      * used by visualisation to allocate space
      *
      */
    long GetMaxAgentNumber() const;

    void SetMaxSimTime(int t);

private:
    /**
      * Position incoming pedestrian using voronoi methods
      * @param src
      * @param agent
      */
    void ComputeBestPositionVoronoi(AgentsSource * src, Pedestrian * agent) const;


    void InitFixedPosition(AgentsSource * src, std::vector<Pedestrian *> & peds) const;

    /**
      * Position incoming pedestrians completely random
      */
    void
    ComputeBestPositionCompleteRandom(AgentsSource * src, std::vector<Pedestrian *> & peds) const;

    /**
      * Position incoming pedestrians randomly
      * @param src
      * @param peds
      */
    void ComputeBestPositionRandom(AgentsSource * src, std::vector<Pedestrian *> & peds) const;

    /**
      * Sort the given position vector by decreasing density
      * @param positions,
      * @param extra_position, an additional vector containing position to be considered in the density calculation
      */
    void SortPositionByDensity(std::vector<Point> & positions, std::vector<Point> & extra_positions)
        const;


private:
    Building * _building;
    std::vector<std::shared_ptr<AgentsSource>> _sources{};
    ///to control the trigger of the events
    long int _lastUpdateTime = 0;
    int maxSimTime           = 0;
};
