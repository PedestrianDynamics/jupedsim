/**
 * \file        GraphRouter.h
 * \date        Aug 20, 2012
 * \version     v0.5
 * \copyright   <2009-2014> Forschungszentrum Jülich GmbH. All rights reserved.
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
 *
 *
 **/


#ifndef ROUTINGGRAPH_H_
#define ROUTINGGRAPH_H_


#include <vector>
#include <map>
#include "../../geometry/Line.h"



struct Edge;
struct ExitDistance;
struct Vertex;

class Pedestrian;
class NavLine;
class Building;
class SubRoom;
class Room;




class RoutingGraph {

public:
     /****************************
      * Constructors & Destructors
      ****************************/

     RoutingGraph();
     RoutingGraph(Building * b);
     RoutingGraph(RoutingGraph * orig);

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
     std::map<int,Vertex> * GetAllVertexes();

private:

     Building * building;
     std::map<int, Vertex> vertexes;



     int addVertex(NavLine * nav_line, bool exit = false);
     void removeVertex(Vertex * remove_vertex);
     void processSubroom(SubRoom * sub);
     void addEdge(Vertex * v1, Vertex * v2, SubRoom * sub);
     bool checkVisibility(Line * l1, Line * l2, SubRoom * sub);

     bool checkVisibility(Point&  p1, Point&  p2, SubRoom * sub);
     bool checkVisibility(Pedestrian * p, NavLine* l, SubRoom * sub);
     void calculateDistancesForExit(Vertex * act_vertex);
     void calculateDistances(Vertex * exit, Vertex * last_vertex, Vertex * act_vertex, double act_distance);


};


struct Edge {
public:
     Vertex * dest;
     Vertex * src;
     double distance;
     SubRoom * sub;
};

struct ExitDistance {
public:
     double distance;
     Edge * exit_edge;
     Vertex * exit_vertex;

     ExitDistance();

     ~ExitDistance();
     SubRoom * GetSubRoom() const;
     Vertex * GetDest() const;
     Vertex * GetSrc() const;

};

struct Vertex {
public:
     NavLine * nav_line;
     int id;
     bool exit;
     std::map<int, Edge> edges;
     std::map<int, ExitDistance> distances;

     ExitDistance getShortestExit();
};


#endif /* ROUTINGGRAPH_H_ */

