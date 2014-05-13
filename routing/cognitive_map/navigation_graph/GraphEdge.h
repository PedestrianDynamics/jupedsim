/**
 * @file   GraphEdge.h
 * @author David Haensel (d.haensel@fz-juelich.de)
 * @date   January, 2014
 * @brief  Edge of a Graph.
 *
 */

#ifndef GRAPHEDGE_H_
#define GRAPHEDGE_H_

#include "../../../geometry/Line.h"
#include <map>

class SubRoom;
class GraphVertex;
class Crossing;

/**
 * @brief Graph Edge.
 *
 */

class GraphEdge {

public:
    typedef std::map<std::string, std::pair<double, double>> FactorContainer;

    /****************************
     * Constructors & Destructors
     ****************************/

    GraphEdge(const GraphVertex * const s, const GraphVertex * const d, const Crossing * const crossing);
    GraphEdge(GraphEdge const & ge);
    virtual ~GraphEdge();

    void CalcApproximateDistance();

    // Getter collection
    const GraphVertex * GetDest() const;
    const GraphVertex * GetSrc() const;
    const Crossing * GetCrossing() const;

    double GetApproximateDistance() const;
    double GetApproximateDistance(const Point &) const;



    void SetFactor(double factor, std::string name);

    double GetRoomToFloorFactor() const;

    double GetFactor() const;
    double GetFactorWithDistance(double distance) const;

    double GetWeight(const Point &) const;


    bool IsExit() const;


private:
    const GraphVertex  * const src;
    const GraphVertex  * const dest;
    const Crossing  * const crossing;

    /**
     * Factor Bag
     *
     * The Factor map is filled up by sensors. The key string is for identification while sharing information.
     * The second pair value is the GlobalTime value from Pedestrian Class of this information.
     * For calculating the weight we just iterate over the factors and dont care which factors we acutally use.
     */
    FactorContainer factors;

    //WEIGHTS DEPRECATED!
    double approximate_distance;
    double density = 0.0;

};


#endif /* GRAPHEDGE_H_ */
