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
    SubRoom * sub;
};

struct exit_distance 
{
    double distance;
    vertex* last_vertex;
    SubRoom * exit_subroom;
};

struct vertex
{
    Crossing* crossing;
    int id;
    vector<edge> edges;
    map<int, exit_distance> distances;

    //public functions
    exit_distance getShortestExit() {
	map<int, exit_distance>::iterator it;
	exit_distance return_var;
	//TODO INF einstzen
	return_var.distance = INFINITY;
	std::cout << "num distances" << distances.size() << std::endl; 
	std::cout << "id" << this->crossing << std::endl; 
	for(it = distances.begin(); it != distances.end(); it++) {
	    std::cout << (*it).second.distance << " asdasdas- " << return_var.distance << std::endl; 
	    std::cout << (*it).second.exit_subroom->GetSubRoomID()<< std::endl;
	    if((*it).second.distance < return_var.distance) {
		return_var.distance = (*it).second.distance;
		return_var.last_vertex = (*it).second.last_vertex;
	    }
	}

	return return_var;
    }
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
	     
		calculateDistances(act_vertex, act_vertex, k, act_vertex->edges[k].distance);
	    }

	}
    }
    print();
    
    return this;
};


Crossing * RoutingGraph::GetNextDestination(int crossing_index, SubRoom * sub) {
    exit_distance dist = vertexes[crossing_index].getShortestExit();
    std::cout << vertexes.size() << std::endl;
    std::cout << "ID " <<   vertexes[crossing_index].id<< std::endl;
    std::cout << "ID exit " <<   dist.last_vertex << std::endl;
    return dist.last_vertex->crossing;
}
void RoutingGraph::calculateDistances(vertex * exit, vertex * last_vertex, int edge_index, double act_distance) 
{
    vertex * act_vertex = last_vertex->edges[edge_index].next_vertex;
    //std::cout << act_vertex->distances[exit->id].distance << " " << act_vertex->crossing->GetIndex() << std::endl;
    // Don't change the exit way, if the other route is shorter.
    if(act_vertex->distances[exit->id].distance && act_vertex->distances[exit->id].distance < act_distance) {

	//std::cout << "asdmasdmasd";
	return;
    }
    // Don't change the exit way, if the rout i through the same subroom! (keine Umwege durch einen Subroom)
    
    // if ((act_vertex->distances[exit->id].distance && act_vertex->distances[exit->id].exit_subroom->GetRoomID() == last_vertex->edges[edge_index].sub->GetRoomID() && act_vertex->distances[exit->id].exit_subroom->GetSubRoomID() == last_vertex->edges[edge_index].sub->GetSubRoomID()) && act_vertex->distances[exit->id].distance < 1.1* act_distance ) {
    // 	return;
    // }
    // if(act_vertex->id == 21 || last_vertex->id == 21) {
    // 	std::cout << "last: " << last_vertex->id << " act: " << act_vertex->id <<  std::endl;
    // }
    act_vertex->distances[exit->id].distance = act_distance;
    act_vertex->distances[exit->id].last_vertex = last_vertex;
    act_vertex->distances[exit->id].exit_subroom = last_vertex->edges[edge_index].sub;
    for(int k = 0; k < act_vertex->edges.size(); k++) {
	//std::cout << act_vertex->crossing->GetIndex() << "\t" <<  k << "\t" << act_distance << std::endl;
	if(!act_vertex->edges[k].next_vertex->crossing->IsExit() ) {

	    if(!(act_vertex->distances[exit->id].exit_subroom->GetRoomID() == act_vertex->edges[k].sub->GetRoomID() && act_vertex->distances[exit->id].exit_subroom->GetSubRoomID() == act_vertex->edges[k].sub->GetSubRoomID())) {
		calculateDistances(exit, act_vertex, k, act_vertex->edges[k].distance + act_distance);
		//std::cout << "asdsd" << std::endl;
	    }
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
	    processNewCrossingEdge(sub, sub->GetAllTransitions()[j], crossing);
	}
	for(unsigned int j = 0; j < sub->GetAllCrossings().size(); j++) {
	    processNewCrossingEdge(sub, sub->GetAllCrossings()[j], crossing);
	}
    }
};

void RoutingGraph::processNewCrossingEdge(SubRoom* sub, Crossing * new_crossing, Crossing * act_crossing)
{
    edge new_edge;
    if(act_crossing->GetIndex() == new_crossing->GetIndex()) 
	return;

    new_edge.next_vertex = & vertexes[new_crossing->GetIndex()];
    // attention!! this is not the shortest distance.
    new_edge.distance = (act_crossing->GetCentre() - new_crossing->GetCentre()).Norm();
    new_edge.sub = sub; 
    this->vertexes[act_crossing->GetIndex()].edges.push_back(new_edge);
};

void RoutingGraph::print()
{
    map<int, vertex>::iterator it;
    map<int, exit_distance>::iterator it2;

    for(it=vertexes.begin(); it != vertexes.end(); it++ ) {
	std::cout << "\n\nvertex: " << (*it).second.crossing->GetIndex()<< "  from: " << (*it).second.crossing->GetSubRoom1()->GetRoomID() << "-" << (*it).second.crossing->GetSubRoom1()->GetSubRoomID() << " to:";

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
	    std::cout << (*it2).first << " (" << (*it2).second.distance << ")" << "subroom "<< (*it2).second.exit_subroom->GetSubRoomID() << "next vertex: " << (*it2).second.last_vertex->id <<" \n";
	}
    }
}

Crossing * RoutingGraph::GetVertexCrossing(int index) {
    return vertexes[index].crossing;
}
