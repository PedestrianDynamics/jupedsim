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

struct Edge;
struct ExitDistance; 
struct Vertex;


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

    ExitDistance  GetNextDestination(int nav_line_index, Pedestrian * p );
    ExitDistance  GetNextDestination(Pedestrian * p);
    void closeDoor(int id);
    
    /**
     * Getter and Setter
     */
    Vertex * GetVertex(int id);
    map<int,Vertex> * GetAllVertexes();
    
private:
    /***********************
     * variabels
     ***********************/
    Building * building;
    map<int, Vertex> vertexes;

    /***********************
     * internal graph init functions
     **********************/
    int addVertex(NavLine * nav_line, bool exit = false);
    void removeVertex(Vertex * remove_vertex);
    void processSubroom(SubRoom * sub);
    void addEdge(Vertex * v1, Vertex * v2, SubRoom * sub);
    bool checkVisibility(Line * l1, Line * l2, SubRoom * sub);
  
    bool checkVisibility(Point  p1, Point  p2, SubRoom * sub);
    bool checkVisibility(Pedestrian * p, NavLine* l, SubRoom * sub);
    void calculateDistancesForExit(Vertex * act_vertex);
    void calculateDistances(Vertex * exit, Vertex * last_vertex, Vertex * act_vertex, double act_distance);


};


struct Edge 
{
public:
    Vertex * dest;
    Vertex * src;
    double distance;
    SubRoom * sub;
};

struct ExitDistance 
{
public:
    double distance;
    Edge * exit_edge;
    SubRoom * GetSubRoom() const;
    Vertex * GetDest() const;
    Vertex * GetSrc() const;
    
};

struct Vertex
{
public:
    NavLine * nav_line;
    int id;
    bool exit;
    map<int, Edge> edges;
    map<int, ExitDistance> distances;

    ExitDistance getShortestExit();
};


#endif /* ROUTINGGRAPH_H_ */

