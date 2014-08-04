/**
 * \file        RoutingGraph.cpp
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


#include "RoutingGraph.h"
#include "../../geometry/Obstacle.h"
#include "../../pedestrian/Pedestrian.h"
#include "../../geometry/Building.h"
#include "../../geometry/NavLine.h"
#include "../../geometry/Hline.h"
#include "../../geometry/Transition.h"
#include "../../general/Macros.h"
#include "../../geometry/SubRoom.h"
#include "../../geometry/Wall.h"
#include "../../geometry/Line.h"
#include "../../IO/OutputHandler.h"

#include <cmath>

using namespace std;

/**
 * Constructors & Destructors
 */
RoutingGraph::RoutingGraph(Building * b) : building(b)
{
     vertexes = map<int, Vertex>();
     BuildGraph();
}

RoutingGraph::RoutingGraph()
{
     vertexes = map<int, Vertex>();
     building=NULL;
}
RoutingGraph::~RoutingGraph()
{

}

RoutingGraph::RoutingGraph(RoutingGraph * orig)
{
     building = orig->building;
     vertexes = orig->vertexes;

     map<int, Vertex>::iterator itv;
     map<int, Edge>::iterator ite;
     map<int, ExitDistance>::iterator ited;

     for(itv = orig->vertexes.begin(); itv != orig->vertexes.end(); itv++) {
          // set the right edge->src and edge->dest pointers
          for(ite = itv->second.edges.begin(); ite != itv->second.edges.end(); ite++) {
               vertexes[itv->first].edges[ite->first].src = &vertexes[itv->first];
               vertexes[itv->first].edges[ite->first].dest = &vertexes[ite->second.dest->id];
          }

          // set the right ExitDistance->edge pointer
          for(ited = itv->second.distances.begin(); ited != itv->second.distances.end(); ited++) {
               if(ited->second.exit_edge)
                    vertexes[itv->first].distances[ited->first].exit_edge = &vertexes[itv->first].edges[ited->second.exit_edge->dest->id];

          }

     }


}



/**
 * GetNextDestination(Vertex index)
 * returns the next destination for pedestrian near NavLine with Nav_line_index (UID)
 */
ExitDistance RoutingGraph::GetNextDestination(int nav_line_index, Pedestrian * p)
{
     ExitDistance dist = vertexes[nav_line_index].getShortestExit();
     return dist;
}


/**
 * RoutingGraph::GetNextDestination(Pedestrian * p)
 * return: next destination for pedestrian in a subrrom.
 * just use this function, if the pedestrian did not pass an other crossing
 */
ExitDistance  RoutingGraph::GetNextDestination(Pedestrian * p)
{
     double act_shortest_dist = INFINITY;
     NavLine * return_line = NULL;
     SubRoom * sub = building->GetRoom(p->GetRoomID())->GetSubRoom(p->GetSubRoomID());
     ExitDistance ed;

     //collecting all lines to check
     vector<NavLine*> lines;
     lines.insert(lines.end(), sub->GetAllCrossings().begin(), sub->GetAllCrossings().end());
     lines.insert(lines.end(), sub->GetAllTransitions().begin(), sub->GetAllTransitions().end());
     lines.insert(lines.end(), sub->GetAllHlines().begin(), sub->GetAllHlines().end());

     for(unsigned int i = 0; i < lines.size(); i++) {
          //  check if  the exit exists in the routing graph and  the line is visible for the pedestrian
          if(GetVertex(lines[i]->GetUniqueID()) && checkVisibility(p, lines[i], sub )) {
               ed = GetVertex(lines[i]->GetUniqueID())->getShortestExit();
               Hline * hline  = dynamic_cast<Hline*>(lines[i]);
               //check if it is a hline OR it is an exit OR the exit is not thorugh the same subroom
               if(hline || GetVertex(lines[i]->GetUniqueID())->exit || p->GetRoomID() != ed.GetSubRoom()->GetRoomID() || p->GetSubRoomID() != ed.GetSubRoom()->GetSubRoomID()) {
                    // check if the distance is shorter
                    double distance = (lines[i]->GetCentre()-p->GetPos()).Norm() + ed.distance;
                    if(act_shortest_dist > distance ) {
                         act_shortest_dist  = distance;
                         return_line = lines[i];
                    }
               }
          }

     }
     ExitDistance return_dist;

     if(!return_line) {
          char tmp[CLENGTH];
          sprintf(tmp,
                  "ERROR: \t Pedestrian  [%d] can't find a exit.at X=%f Y = %f Removed Pedestrian.",
                  p->GetID(), p->GetPos().GetX(), p->GetPos().GetY());
          Log->Write(tmp);
          return return_dist;
     }

     ed = GetVertex(return_line->GetUniqueID())->getShortestExit();

     return_dist.distance = act_shortest_dist;
     //
     return_dist.exit_edge = NULL;

     if(ed.exit_vertex->id != return_line->GetUniqueID()) {
          return_dist.exit_edge = new Edge();
          return_dist.exit_edge->dest = GetVertex(return_line->GetUniqueID());
          return_dist.exit_edge->sub = sub;
          return_dist.exit_edge->src = NULL;

     }


     return_dist.exit_vertex = ed.exit_vertex;



     return return_dist;
}


/**
 * RoutingGraph::BuildGraph()
 * Building Graph from transitions and crossing
 */

RoutingGraph * RoutingGraph::BuildGraph()
{
     // Get all Rooms from the building object
     const vector<Room*> * rooms = & building->GetAllRooms();
     for(unsigned int i = 0; i < rooms->size(); i++) {
          // go through all subroom and retrieve all nav lines
          const vector<SubRoom*> * subrooms = & (*rooms)[i]->GetAllSubRooms();
          for(unsigned int k = 0; k < subrooms->size(); k++) {
               processSubroom((*subrooms)[k]);
          }
     }

     map<int, Vertex>::iterator it;
     //calculate the distances for Exits!
     for(it = vertexes.begin(); it != vertexes.end(); it++) {

          if(it->second.exit) {
               calculateDistancesForExit(&it->second);
          }
     }
     //print();
     return this;
};

/**
 * RoutingGraph::addVertex(NavLine * nav_line)
 * adds a new vertex with the given NavLine
 * returns the insert id
 */

int RoutingGraph::addVertex(NavLine * nav_line, bool exit)
{
     if(!nav_line) return -1;
     int id = nav_line->GetUniqueID();
     vertexes[id].nav_line = nav_line;
     vertexes[id].id = id;
     vertexes[id].exit = exit;
     return id;
}

void RoutingGraph::removeVertex(Vertex * remove_vertex)
{
     if(!remove_vertex)
          return;

     // remove all edges
     map<int, Edge>::iterator it_e;

     for(it_e = remove_vertex->edges.begin(); it_e != remove_vertex->edges.end(); it_e++) {
          it_e->second.dest->edges.erase(remove_vertex->id);
     }

     // remove vertex
     vertexes.erase(remove_vertex->id);

     // calculate new distance, maybe some distance changed because the door is closed.
     map<int, Vertex>::iterator it;
     //calculate the distances for Exits!
     for(it = vertexes.begin(); it != vertexes.end(); it++) {

          it->second.distances.clear();
     }

     for(it = vertexes.begin(); it != vertexes.end(); it++) {

          if(it->second.exit) {
               calculateDistancesForExit(&it->second);
          }
     }



}


void RoutingGraph::closeDoor(int id)
{
     removeVertex(GetVertex(id));
     return;
}
/**
 * RoutingGraph::processSubroom
 * adds all Crossings Transition and Hlines in a Subroom and the associated edges
 */
void RoutingGraph::processSubroom(SubRoom * sub)
{
     vector<int> goals;
     // add all crossings, transitions, hlines as vertex
     // save the insert id to add all edges in a subroom
     for(unsigned int i = 0; i < sub->GetAllCrossings().size(); i++) {
          goals.push_back(addVertex(sub->GetAllCrossings()[i]));
     }
     for(unsigned int i = 0; i < sub->GetAllTransitions().size(); i++) {
          if(sub->GetAllTransitions()[i]->IsOpen())
               goals.push_back(addVertex(sub->GetAllTransitions()[i], sub->GetAllTransitions()[i]->IsExit()));
     }
     for(unsigned int i = 0; i < sub->GetAllHlines().size(); i++) {
          goals.push_back(addVertex(sub->GetAllHlines()[i]));

     }

     // now add all edges between all goals
     // check if they visible by each other
     if(goals.size() > 1) {
          for(unsigned int k = 0; k < goals.size()-1; k++) {
               for(unsigned int i = k+1; i < goals.size(); i++) {
                    addEdge(GetVertex(goals[k]), GetVertex(goals[i]), sub);
                    addEdge(GetVertex(goals[i]), GetVertex(goals[k]), sub);
               }
          }
     }
}

/**
 * adds a crossing from v1 to v2, if v2 is visible from v1.
 * calculates the distance between both
 */

void RoutingGraph::addEdge(Vertex * v1, Vertex * v2, SubRoom* sub)
{
     Edge new_edge;

     // do not add loops!
     if(v1->id == v2->id)
          return;


     //TODO: sometime checkVisibility is not commutative (cV(v1,v2) != cV(v2,v1))
     if(checkVisibility(v1->nav_line, v2->nav_line, sub) ||  checkVisibility(v2->nav_line, v1->nav_line, sub)) {

          new_edge.dest = v2;
          new_edge.src = v1;

          // attention!! this is not the shortest distance.
          new_edge.distance = (v1->nav_line->GetCentre() - v2->nav_line->GetCentre()).Norm();
          new_edge.sub = sub;
          v1->edges[v2->id] = new_edge;
     }

};

bool RoutingGraph::checkVisibility(Line* l1, Line* l2, SubRoom* sub)
{
     return sub->IsVisible(l1,l2,true);
}


bool RoutingGraph::checkVisibility(Point& p1, Point& p2, SubRoom* sub)
{
     return sub->IsVisible(p1,p2,true);
}


//this function differs from just checking visibility
//checks if a pedestrian is standing in a hline
bool RoutingGraph::checkVisibility(Pedestrian * p, NavLine * l, SubRoom* sub)
{
     // connecting lines to check
     const int num_lines = 3;
     Line cl[num_lines] = { Line(p->GetPos(), l->GetCentre()), Line(p->GetPos(), l->GetPoint1()), Line(p->GetPos(), l->GetPoint2()) };
     bool temp[num_lines] = { true, true, true };

     //check for intersection with walls
     for(unsigned int i = 0; i <  sub->GetAllWalls().size(); i++) {
          for(int j = 0; j < num_lines; j++) {
               if(temp[j] && cl[j].IntersectionWith(sub->GetAllWalls()[i])) {
                    temp[j] = false;
               }
          }
     }
     //check for intersection with hlines
     for(unsigned int i = 0; i <  sub->GetAllHlines().size(); i++) {
          for(int j = 0; j < num_lines; j++) {
               if(temp[j]
                         && l->GetUniqueID() != sub->GetAllHlines()[i]->GetUniqueID()
                         && !l->IsInLineSegment(p->GetPos())
                         &&  cl[j].IntersectionWith((*sub->GetAllHlines()[i]))
                 ) {
                    temp[j] = false;
               }
          }
     }


     for(unsigned int i = 0; i < sub->GetAllObstacles().size(); i++) {
          Obstacle * obs = sub->GetAllObstacles()[i];
          for(unsigned int k = 0; k<obs->GetAllWalls().size(); k++) {
               Wall w = obs->GetAllWalls()[k];
               for(int j = 0; j < num_lines; j++) {
                    if(temp[j] && cl[j].IntersectionWith(w))
                         temp[j] = false;
               }
          }
     }
     //at the moment no check for intersection with obstacles
     return temp[0] || temp[1] || temp[2];
}


/**
 * calculates all distances to a certain exit
 */
void RoutingGraph::calculateDistancesForExit(Vertex *act_vertex)
{
     if(!act_vertex->exit) return;

     act_vertex->distances[act_vertex->id].distance = 0;
     act_vertex->distances[act_vertex->id].exit_edge = NULL;
     act_vertex->distances[act_vertex->id].exit_vertex = act_vertex;


     map<int, Edge>::iterator it;


     for(it = act_vertex->edges.begin(); it != act_vertex->edges.end(); it++) {
          calculateDistances(act_vertex, act_vertex, it->second.dest, it->second.distance);
     }
}
/**
 * recursive function to calculate distances from one vertex to all others
 * implementation like Dijkstra
 */
void RoutingGraph::calculateDistances(Vertex * exit, Vertex * last_vertex, Vertex * act_vertex, double act_distance)
{
     // Don't change the exit way, if the other route is shorter.
     if(act_vertex->distances[exit->id].distance && act_vertex->distances[exit->id].distance < act_distance) {
          return;
     }
     act_vertex->distances[exit->id].distance = act_distance;
     act_vertex->distances[exit->id].exit_edge = & act_vertex->edges[last_vertex->id];
     act_vertex->distances[exit->id].exit_vertex = exit;


     map<int, Edge>::iterator it;

     for(it = act_vertex->edges.begin(); it != act_vertex->edges.end(); it++) {
          //if the next vertex is an exit here is nothing to do.
          if(!it->second.dest->exit) {


               // if the edge is through the same room then the exit here is nothing to do, except it is a hline
               Hline * hline1 = dynamic_cast<Hline*>(act_vertex->nav_line);
               if(hline1 || !(act_vertex->edges[last_vertex->id].sub->GetRoomID() == it->second.sub->GetRoomID() && act_vertex->edges[last_vertex->id].sub->GetSubRoomID() == it->second.sub->GetSubRoomID())) {
                    calculateDistances(exit, act_vertex, it->second.dest, it->second.distance + act_distance);
               }
          }
     }
}

/**
 * debug printing function
 */
void RoutingGraph::print()
{
     map<int, Vertex>::iterator it;
     map<int, ExitDistance>::iterator it2;
     std::cout << vertexes.size() << std::endl;
     for(it=vertexes.begin(); it != vertexes.end(); it++ ) {
          std::cout << "\n\nvertex: " << (*it).second.nav_line->GetUniqueID() << std::endl;

          Crossing * crossing = dynamic_cast<Crossing*>(it->second.nav_line);

          if(crossing) {
               std::cout << "  from: " << crossing->GetSubRoom1()->GetRoomID() << "-" << crossing->GetSubRoom1()->GetSubRoomID() << " to:";

               if(crossing->GetSubRoom2())
                    std::cout <<  crossing->GetSubRoom2()->GetRoomID() << "-" << crossing->GetSubRoom2()->GetSubRoomID() << std::endl;
               else
                    std::cout << "exit" << std::endl;
          } else {
               std::cout << "hline" << std::endl;
          }
          std::cout << " edges to   ";
          map<int, Edge>::iterator it_edges;

          for(it_edges = it->second.edges.begin(); it_edges != (*it).second.edges.end(); it_edges++) {
               if(it_edges->second.dest)
                    std::cout << it_edges->second.dest->nav_line->GetUniqueID() << "(distance " << it_edges->second.distance <<") ";
               else
                    std::cout << "NULL" << "-" ;
          }
          std::cout << std::endl << std::endl;
          std::cout << "exit distances: \n ";

          for(it2 = (*it).second.distances.begin(); it2 !=  (*it).second.distances.end(); it2++) {
               std::cout << (*it2).first << " (" << (*it2).second.distance << ")" ;
               if(it2->second.exit_edge) {
                    std::cout <<"subroom "<<  it2->second.GetSubRoom()->GetRoomID() << "-" << (*it2).second.GetSubRoom()->GetSubRoomID() << "next vertex: " << (*it2).second.GetDest()->id <<" \n";
               } else {
                    std::cout << std::endl;
               }
          }
     }
}

/**
 * Getter and Setter
 */

map <int, Vertex> * RoutingGraph::GetAllVertexes()
{
     return  &vertexes;
};


Vertex * RoutingGraph::GetVertex(int id)
{
     map<int, Vertex>::iterator it;
     it = vertexes.find(id);
     if(it != vertexes.end()) {
          return &it->second;
     } else {
          return NULL;
     }
};

/****************************************
 * Class Vertex
 ***************************************/

ExitDistance Vertex::getShortestExit()
{
     map<int, ExitDistance>::iterator it;
     ExitDistance return_var;

     return_var.distance = INFINITY;
     for(it = distances.begin(); it != distances.end(); it++) {
          if((*it).second.distance < return_var.distance) {
               return_var = it->second;
          }
     }
     return return_var;
}


/****************************************
 * Class ExitDistance
 ***************************************/

SubRoom * ExitDistance::GetSubRoom() const
{
     if(exit_edge)
          return exit_edge->sub;
     else
          return NULL;
}

Vertex * ExitDistance::GetDest() const
{
     if(exit_edge && exit_edge->dest) {
          return exit_edge->dest;
     } else {
          if(exit_vertex)
               return exit_vertex;
          else
               return NULL;
     }

}

Vertex * ExitDistance::GetSrc() const
{
     if(exit_edge) {
          // std::cout<< "edge " << exit_edge << std::endl;
          if(exit_edge->src) {
               // std::cout<< " src " << exit_edge->src << std::endl;
               return exit_edge->src;
          } else {
               return NULL;
          }
     } else
          return NULL;

}

ExitDistance::~ExitDistance()
{

     // if(!GetSrc() && exit_edge != NULL)
     // {
     //      delete exit_edge;
     // }
}

ExitDistance::ExitDistance()
{
     distance = 0.0;
     exit_edge = NULL;
     exit_vertex = NULL;

}
