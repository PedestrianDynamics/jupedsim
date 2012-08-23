/*
 * RoutingGraph.cpp
 *
 *  Created on: Aug 20, 2012
 *      Author: David Haensel
 */

#include "RoutingGraph.h"

struct edge 
{
  vertex* next_vertex;
  double distance;
};

struct exit_distance 
{
  double distance;
  vertex* last_vertex;
};
struct vertex
{
  Crossing* crossing;
  int id;
  vector<edge> edges;
  map<int, exit_distance> distances;
};



RoutingGraph::RoutingGraph(const vector<Transition*> &trans, const vector<Crossing*> &cross) : crossings(&cross), transitions(&trans) 
{

  BuildGraph();
  
};

RoutingGraph::RoutingGraph()
{
  
}
RoutingGraph::~RoutingGraph() 
{

};

/**
 * RoutingGraph::BuildGraph()
 * Building Graph with transitions and crossing
 */

RoutingGraph* RoutingGraph::BuildGraph() 
{
  //processing crossings
  for(int i = 0; i < (int) crossings->size(); i++) 
    {
      Crossing * crossing = (*crossings)[i];
      vertexes[crossing->GetIndex()].crossing = crossing;
      vertexes[crossing->GetIndex()].id = crossing->GetIndex();
      //add edges
      //adding transitions and crossings from 1 subroom
      processSubroom(crossing->GetSubRoom1(), vertexes, crossing);
      //now subroom2
      processSubroom(crossing->GetSubRoom2(), vertexes, crossing);
    }
  //processing transition
  //take care about EXITs!
  for(int i = 0; i < (int) transitions->size(); i++) 
    {
      Crossing * crossing = (*transitions)[i];
      vertexes[crossing->GetIndex()].crossing = crossing;
      vertexes[crossing->GetIndex()].id = crossing->GetIndex();
      //add edges
      //adding transitions and crossings from 1 subroom
      processSubroom(crossing->GetSubRoom1(), vertexes, crossing);
      //now subroom2
      processSubroom(crossing->GetSubRoom2(), vertexes, crossing);
      
      
    }

  // print();
  //calculate the distances for Exits!

  for(int i = 0; i < (int) transitions->size(); i++) 
    {
      Crossing * crossing = (*transitions)[i];
      if(crossing->IsExit()) 
	{
	  vertex * act_vertex =  & vertexes[crossing->GetIndex()];
	  for(int k = 0; k < act_vertex->edges.size(); k++) 
	    {
	     
	      calculateDistances(act_vertex, act_vertex, act_vertex->edges[k].next_vertex, act_vertex->edges[k].distance);
	    }

	}
    }
  print();
    
  return this;
};

void RoutingGraph::calculateDistances(vertex * exit, vertex * last_vertex, vertex * act_vertex, double act_distance) 
{

  //std::cout << act_vertex->distances[exit->id].distance << " " << act_vertex->crossing->GetIndex() << std::endl;
  if(act_vertex->distances[exit->id].distance && act_vertex->distances[exit->id].distance < act_distance) {
    //std::cout << "asdmasdmasd";
    return;
  }
  
  act_vertex->distances[exit->id].distance = act_distance;
  act_vertex->distances[exit->id].last_vertex = last_vertex;
  for(int k = 0; k < act_vertex->edges.size(); k++) {
    //std::cout << act_vertex->crossing->GetIndex() << "\t" <<  k << "\t" << act_distance << std::endl;
    if(! act_vertex->edges[k].next_vertex->crossing->IsExit()) {
      calculateDistances(exit, act_vertex, act_vertex->edges[k].next_vertex, act_vertex->edges[k].distance + act_distance);
      //std::cout << "asdsd" << std::endl;
    }
  }
}


void RoutingGraph::processSubroom(SubRoom * sub, map<int, vertex> & vertexes, Crossing * crossing)
{
  if(!sub) {
    // nothing to do here, check exit with Crossing::IsExit()
    return;
  } else {
    for(unsigned int j = 0; j < sub->GetAllTransitions().size(); j++) {
      processNewCrossingEdge(sub->GetAllTransitions()[j], crossing);
    }
    for(unsigned int j = 0; j < sub->GetAllCrossings().size(); j++) {
      processNewCrossingEdge(sub->GetAllCrossings()[j], crossing);
    }
  }
};


void RoutingGraph::processNewCrossingEdge(Crossing * new_crossing, Crossing * act_crossing) {
  edge new_edge;
  if(act_crossing->GetIndex() == new_crossing->GetIndex()) 
    return;

  new_edge.next_vertex = & vertexes[new_crossing->GetIndex()];
  // attention!! this is not the shortest distance. its the shortest distance to the center point of the next crossing
  new_edge.distance = act_crossing->DistTo(new_crossing->GetCentre());
  this->vertexes[act_crossing->GetIndex()].edges.push_back(new_edge);
};

void RoutingGraph::print()
{
  map<int, vertex>::iterator it;
  map<int, exit_distance>::iterator it2;

  for(it=vertexes.begin(); it != vertexes.end(); it++ ) {
    std::cout << "vertex: " << (*it).second.crossing->GetIndex()<< "  from: " << (*it).second.crossing->GetSubRoom1()->GetRoomID() << "-" << (*it).second.crossing->GetSubRoom1()->GetSubRoomID() << " to:";

    if((*it).second.crossing->GetSubRoom2())
      std::cout <<  (*it).second.crossing->GetSubRoom2()->GetRoomID() << "-" << (*it).second.crossing->GetSubRoom2()->GetSubRoomID() << std::endl;
    else
      std::cout << "exit" << std::endl;

    std::cout << " edges to   ";
    for(unsigned int i = 0; i < (*it).second.edges.size(); i++) {
      if((*it).second.edges[i].next_vertex)
        std::cout << (*it).second.edges[i].next_vertex->crossing->GetIndex() << "(distance " << (*it).second.edges[i].distance <<" - ";
      else
        std::cout << "NULL" << "-" ;
    }
    std::cout << std::endl << std::endl; 
    std::cout << "exit distacne: ";

    for(it2 = (*it).second.distances.begin(); it2 !=  (*it).second.distances.end(); it2++) {
      std::cout << (*it2).first << " (" << (*it2).second.distance << ") \n";
    }
  }
}

