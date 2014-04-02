/**
 * @file   CognitiveMap.h
 * @author David Haensel (d.haensel@fz-juelich.de)
 * @date   January, 2014
 * @brief  Cognitive Map models the pedestrian knowledge of building space in simulation.
 *
 */

#ifndef COGNITIVEMAP_H_
#define COGNITIVEMAP_H_

#include "NavigationGraph.h"

class SubRoom;
class Transition;
class Crossing;
class Building;
class NavLine;
class Pedestrian;



/**
 * @brief Cognitive Map
 *
 * The CognitiveMap is a simplified representation of human knowledge of building space.
 * It takes the different quantity and certainty of knowledge into account.
 * It is the basis for further developments like individual behavior and information sharing algorithms.
 *
 */
class CognitiveMap {

public:
    /****************************
     * Constructors & Destructors
     ****************************/
    CognitiveMap(const Building * building, const Pedestrian * pedestrian);
    virtual ~CognitiveMap();

    void Add(const SubRoom * sub_room);
    void Add(const Crossing * crossing);
    void AddExit(const Transition * exit);

    NavigationGraph::VerticesContainer * GetAllVertices();

    const NavigationGraph * GetNavigationGraph() const;

    const NavLine * GetDestination();
    const NavLine * GetLocalDestination();

private:
    NavigationGraph * navigation_graph;
    const Building * const building;
    const Pedestrian * const pedestrian;

};


#endif /* COGNITIVEMAP_H_ */
