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


#ifndef AGENTSSOURCESMANAGER_H_
#define AGENTSSOURCESMANAGER_H_

#include <vector>
#include <memory>
#include <atomic>

//jaaa
#include "boost/polygon/voronoi.hpp"
using boost::polygon::voronoi_builder;
using boost::polygon::voronoi_diagram;
using boost::polygon::x;
using boost::polygon::y;
using boost::polygon::low;
using boost::polygon::high;
#include "../geometry/SubRoom.h"

//Forward declarations
class AgentsSource;
class Building;
class Pedestrian;
class Point;

class AgentsSourcesManager
{
public:
     /**
      * Constructor
      */
     AgentsSourcesManager();

     /**
      * disable copying
      */
     AgentsSourcesManager(const AgentsSourcesManager& ) = delete;

     /**
      * Destructor
      */
     virtual ~AgentsSourcesManager();

     /**
      * Make the class "runnable" by overloading the operator
      * @param value
      */
     void operator()();

     /**
      *
      */
     void Run();

     /**
      *  Add a new agent source
      */
     void AddSource(std::shared_ptr<AgentsSource> src);

     /**
      * @return all sources
      */
     const std::vector<std::shared_ptr<AgentsSource> >& GetSources() const;

     /**
      * Set the building object
      */
     void SetBuilding(Building* building);

     /**
      * @return true if all agents have been generated
      * and the class is ready to leave
      */
     bool IsCompleted() const;

     /**
      * Return a pointer to the building object
      */
     Building* GetBuilding() const;

     /**
      *Schedule the pedestrians for the simulation
      * @return true if all source are empty
      */
     bool ProcessAllSources() const;

     /**
      * Trigger the sources to generate the specified
      * number of agents for this frequency
      */
     void GenerateAgents();

     /**
      * Return the total number of agents that will be generated.
      * used by visualisation to allocate space
      */
     long GetMaxAgentNumber() const;


private:

     /**
      * Position incoming pedestrian using voronoi methods
      * @param src
      * @param agent
      */
     void ComputeBestPositionVoronoi(AgentsSource* src, Pedestrian* agent) const;

     bool IsEnoughInSubroom( SubRoom* subroom, Point& pt ) const;

     /**
      * Position incoming pedestrian using voronoi from boost library
      * @param src
      * @param ped
      */
      void ComputeBestPositionVoronoiBoost(AgentsSource* src,Pedestrian* ped) const;

      /**
       * Position incoming pedestrian using voronoi from boost library
       * @param src
       * @param peds
       */
      bool ComputeBestPositionVoronoiBoost(AgentsSource* src, std::vector<Pedestrian*>& peds) const;

      void ComputeBestPositionDummy(AgentsSource* src, std::vector<Pedestrian*>& peds) const;

      /**Position incoming pedestrian on the vertex with the greatest distance
       *
       */
      void VoronoiBestVertexMax (const std::vector<Point>& discrete_positions, const voronoi_diagram<double>& vd, SubRoom* subroom, double factor,
      		voronoi_diagram<double>::const_vertex_iterator& max_it, double& max_dis	) const ;

      /**Position incoming pedestrian on the vertex with greater probability for greater distances
       *
       */
      void VoronoiBestVertexRandMax (const std::vector<Point>& discrete_positions, const voronoi_diagram<double>& vd, SubRoom* subroom, double factor,
            		voronoi_diagram<double>::const_vertex_iterator& max_it, double& max_dis	) const ;

      /**Position incoming pedestrian on a random vertex
       *
       */
      void VoronoiBestVertexRand (const std::vector<Point>& discrete_positions, const voronoi_diagram<double>& vd, SubRoom* subroom, double factor,
            		voronoi_diagram<double>::const_vertex_iterator& max_it, double& max_dis	) const ;

      void VoronoiAdjustVelocityNeighbour( const voronoi_diagram<double>& vd, voronoi_diagram<double>::const_vertex_iterator& chosen_it,
      			Pedestrian* ped, const std::vector<Point>& velocities_vector ) const;

     /**
      * Position incoming pedestrians randomly
      * @param src
      * @param peds
      */
     void ComputeBestPositionRandom(AgentsSource* src, std::vector<Pedestrian*>& peds) const;


     /**
      * Adjust the velocity of the pedestrian using the weidmann fundamental diagram
      */
     void AdjustVelocityUsingWeidmann(Pedestrian* ped) const;

     /**
      *
      * @param ped adjust the velocity by using the mean velocity of the neighbor in front of me
      */
     void AdjustVelocityByNeighbour(Pedestrian* ped) const;

     /**
      * Sort the given position vector by decreasing density
      * @param positions,
      * @param extra_position, an additional vector containing position to be considered in the density calculation
      */
     void SortPositionByDensity(std::vector<Point>& positions, std::vector<Point>& extra_positions) const;


private:
     /// contain the sources
     std::vector<std::shared_ptr<AgentsSource> > _sources;
     ///to control the trigger of the events
     long int _lastUpdateTime = 0;
     /// building object
     Building* _building=nullptr;
     /// whether all agents have been dispatched
     static bool _isCompleted;
     //std::atomic<bool>_isCompleted=false;
};

#endif /* AGENTSSOURCESMANAGER_H_ */
