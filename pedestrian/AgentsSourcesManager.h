/*
 * AgentsSourcesManager.h
 *
 *  Created on: 14.04.2015
 *      Author: piccolo
 */

#ifndef AGENTSSOURCESMANAGER_H_
#define AGENTSSOURCESMANAGER_H_

#include <vector>
#include <memory>
#include <atomic>

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


private:

     /**
      * Position incoming pedestrian using voronoi methods
      * @param src
      * @param agent
      */
     void ComputeBestPositionVoronoi(AgentsSource* src, Pedestrian* agent) const;

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
