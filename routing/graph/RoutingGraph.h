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

#include "../../geometry/Building.h"
#include "../../geometry/NavLine.h"

struct edge;
struct exit_distance; 
struct vertex;


class RoutingGraph {

public:
    /****************************
     * Constructors & Destructors
     ****************************/
    
    RoutingGraph();
    RoutingGraph(Building * b);
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

    NavLine * GetNextDestination(int crossing_index);
    NavLine * GetNextDestination(Pedestrian * p);
    
    /**
     * Getter and Setter
     */
    vertex * GetVertex(int id);
    map<int,vertex> * GetAllVertexes();
    
private:
    /***********************
     * variabels
     ***********************/
    Building * building;
    map<int, vertex> vertexes;

    /***********************
     * internal graph init functions
     **********************/
    int addVertex(NavLine * nav_line, bool exit = false);
    void processSubroom(SubRoom * sub);
    void addEdge(vertex * v1, vertex * v2, SubRoom * sub);
    bool checkVisibility(Line * l1, Line * l2, SubRoom * sub);
    void calculateDistancesForExit(vertex * act_vertex);
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
    NavLine * nav_line;
    int id;
    bool exit;
    vector<edge> edges;
    map<int, exit_distance> distances;

    exit_distance getShortestExit();
};


#endif /* ROUTINGGRAPH_H_ */

