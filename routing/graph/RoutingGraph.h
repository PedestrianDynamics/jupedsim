/*
 * GraphRouter.h
 *
 *  Created on: Aug 20, 2012
 *      Author: David Haensel
 */

#ifndef ROUTINGGRAPH_H_
#define ROUTINGGRAPH_H_


#include <vector>
#include <map>

#include "../../geometry/Transition.h"
#include "../../geometry/Crossing.h"
#include "../../geometry/SubRoom.h"

struct edge;
struct exit_distance; 
struct vertex;


class RoutingGraph {

public:
    /****************************
     * Constructors & Destructors
     ****************************/
    
    RoutingGraph();
    RoutingGraph(const vector<Transition*> &trans , const vector<Crossing*> &cross);
    virtual ~RoutingGraph();

    /**************************
     * init functions for Graph building
     **************************/
    RoutingGraph * BuildGraph();
    
    // debug
    void print();
    
    /***************************
     * Routing helpers
     **************************/

    Crossing * GetNextDestination(int crossing_index);
    Crossing * GetNextDestination(Pedestrian * p);
    
    /**
     * Getter and Setter
     */
    map<int,vertex> * GetAllVertexes();
    
private:
    /***********************
     * variabels
     ***********************/
    const vector<Crossing*> * crossings;
    const vector<Transition*> * transitions;
    map<int, vertex> vertexes;

    /***********************
     * internal graph init functions
     **********************/
    void processSubroom(SubRoom * sub, map<int, vertex> & vertexes, Crossing * crossing);
    void processNewCrossingEdge(SubRoom * sub, Crossing * new_crossing, Crossing * act_crossing);
    void calculateDistances(vertex * exit, vertex * last_vertex, int edge_index, double act_distance);


};


struct edge 
{
public:
    vertex* next_vertex;
    double distance;
    SubRoom * sub;
};

struct exit_distance 
{
public:
    double distance;
    vertex* last_vertex;
    SubRoom * exit_subroom;
};

struct vertex
{
public:
    Crossing* crossing;
    int id;
    vector<edge> edges;
    map<int, exit_distance> distances;

    exit_distance getShortestExit();
};


#endif /* ROUTINGGRAPH_H_ */

