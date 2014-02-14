/**
 * @file   CognitiveMap.h
 * @author David Haensel (d.haensel@fz-juelich.de)
 * @date   January, 2014
 * @brief  Cognitive Map models the pedestrian knowledge of building space in simulation.
 *
 */

#ifndef COGNITIVEMAP_H_
#define COGNITIVEMAP_H_

class NavigationGraph;
class SubRoom;
class Transition;
class Crossing;
class Building;


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
    CognitiveMap(const Building * building);
    virtual ~CognitiveMap();

    void Add(const SubRoom * sub_room);
    void Add(const Crossing * crossing);
    void AddExit(const Transition * exit);

    const NavigationGraph * GetNavigationGraph();

private:
    NavigationGraph * navigation_graph;
    const Building * const building;

};


#endif /* COGNITIVEMAP_H_ */
