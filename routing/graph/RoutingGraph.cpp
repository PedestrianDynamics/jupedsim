/*
 * RoutingGraph.cpp
 *
 *  Created on: Aug 20, 2012
 *      Author: David Haensel
 */

#include "RoutingGraph.h"




/**
 * Constructors & Destructors
 */
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
 * GetNextDestination(vertex index)
 * returns the next destination for pedestrian near crossing with id index
 */
Crossing * RoutingGraph::GetNextDestination(int crossing_index)
{
    exit_distance dist = vertexes[crossing_index].getShortestExit();
    return dist.last_vertex->crossing;
}

/**
 * RoutingGraph::GetNextDestination(Pedestrian * p)
 * return: next destination for pedestrian in a subrrom. 
 * just use this function, if the pedestrian did not pass an other crossing
 */
Crossing * RoutingGraph::GetNextDestination(Pedestrian * p)
{
    map<int,vertex>::iterator it;
    double act_shortest_dist = INFINITY;
    Crossing * crossing = NULL;
    for(it = vertexes.begin(); it != vertexes.end(); it++) {
	
	if(it->second.crossing->IsInRoom(p->GetRoomID()) && it->second.crossing->IsInSubRoom(p->GetSubRoomID())) {
	    double distance = it->second.crossing->DistTo(p->GetPos());
	    if(!it->second.crossing->IsExit()) {
		//TODO: look if there is an other exit in the subroom
		return it->second.crossing; 
	    }
	    if(act_shortest_dist > distance) {
		act_shortest_dist  = distance; 
		crossing = it->second.crossing;
	    }
	}
    }
    
    return crossing;
}


/**
 * RoutingGraph::BuildGraph()
 * Building Graph from transitions and crossing
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

void RoutingGraph::calculateDistances(vertex * exit, vertex * last_vertex, int edge_index, double act_distance) 
{
    vertex * act_vertex = last_vertex->edges[edge_index].next_vertex;
    // Don't change the exit way, if the other route is shorter.
    if(act_vertex->distances[exit->id].distance && act_vertex->distances[exit->id].distance < act_distance) {
	return;
    }
    act_vertex->distances[exit->id].distance = act_distance;
    act_vertex->distances[exit->id].last_vertex = last_vertex;
    act_vertex->distances[exit->id].exit_subroom = last_vertex->edges[edge_index].sub;
    for(int k = 0; k < act_vertex->edges.size(); k++) {
	//if the next vertex is an exit here is nothing to do.
	if(!act_vertex->edges[k].next_vertex->crossing->IsExit() ) {
	    // if the edge is through the same room then the exit here is nothing to do
	    if(!(act_vertex->distances[exit->id].exit_subroom->GetRoomID() == act_vertex->edges[k].sub->GetRoomID() && act_vertex->distances[exit->id].exit_subroom->GetSubRoomID() == act_vertex->edges[k].sub->GetSubRoomID())) {
		calculateDistances(exit, act_vertex, k, act_vertex->edges[k].distance + act_distance);
	    }
	}
    }
}


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

/**
 * Getter and Setter
 */

map <int, vertex> * RoutingGraph::GetAllVertexes()
{ 
    return  &vertexes; 
};

/****************************************
 * Class vertex
 ***************************************/

exit_distance vertex::getShortestExit() {
    map<int, exit_distance>::iterator it;
    exit_distance return_var;

    return_var.distance = INFINITY;
    for(it = distances.begin(); it != distances.end(); it++) {
	if((*it).second.distance < return_var.distance) {
	    return_var.distance = (*it).second.distance;
	    return_var.last_vertex = (*it).second.last_vertex;
	}
    }
    return return_var;
}
