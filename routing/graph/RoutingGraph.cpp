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
RoutingGraph::RoutingGraph(Building * b) : building(b)
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
 * returns the next destination for pedestrian near NavLine with Nav_line_index (UID)
 */
NavLine * RoutingGraph::GetNextDestination(int nav_line_index)
{
    exit_distance dist = vertexes[nav_line_index].getShortestExit();
    return dist.last_vertex->nav_line;
}


/**
 * RoutingGraph::GetNextDestination(Pedestrian * p)
 * return: next destination for pedestrian in a subrrom. 
 * just use this function, if the pedestrian did not pass an other crossing
 */
NavLine * RoutingGraph::GetNextDestination(Pedestrian * p)
{
    double act_shortest_dist = INFINITY;
    NavLine * nav_line = NULL;
    SubRoom * sub = building->GetRoom(p->GetRoomID())->GetSubRoom(p->GetSubRoomID());
    for(unsigned int i = 0; i < sub->GetAllTransitions().size(); i++) {
	if(sub->GetAllTransitions()[i]->IsExit()) return sub->GetAllTransitions()[i];
	double distance = sub->GetAllTransitions()[i]->DistTo(p->GetPos()) + 
	    GetVertex(sub->GetAllTransitions()[i]->GetUniqueID())->getShortestExit().distance;
	if(act_shortest_dist > distance) {
	    act_shortest_dist  = distance; 
	    nav_line = sub->GetAllTransitions()[i];
	}
    }
    for(unsigned int i = 0; i < sub->GetAllCrossings().size(); i++) {
	double distance = sub->GetAllCrossings()[i]->DistTo(p->GetPos()) + 
	    GetVertex(sub->GetAllCrossings()[i]->GetUniqueID())->getShortestExit().distance;
	if(act_shortest_dist > distance) {
	    act_shortest_dist  = distance; 
	    nav_line = sub->GetAllCrossings()[i];
	}
    }
    for(unsigned int i = 0; i < sub->GetAllHlines().size(); i++) {
	double distance = sub->GetAllHlines()[i]->DistTo(p->GetPos()) + 
	    GetVertex(sub->GetAllHlines()[i]->GetUniqueID())->getShortestExit().distance;
	if(act_shortest_dist > distance) {
	    act_shortest_dist  = distance; 
	    nav_line = sub->GetAllHlines()[i];
	}
    }
    return nav_line;
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

    map<int, vertex>::iterator it; 
    //calculate the distances for Exits!
    for(it = vertexes.begin(); it != vertexes.end(); it++) {

	if(it->second.exit) {
	    calculateDistancesForExit(&it->second);
        }
    }
     print();
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
	for(int k = 0; k < goals.size()-1; k++) {
	    for(int i = k+1; i < goals.size(); i++) {
		addEdge(GetVertex(goals[k]), GetVertex(goals[i]), sub);
		addEdge(GetVertex(goals[i]), GetVertex(goals[k]), sub);
	    }
	}
    }
};

/**
 * adds a crossing from v1 to v2, if v2 is visible from v1.
 * calculates the distance between both
 */
void RoutingGraph::addEdge(vertex * v1, vertex * v2, SubRoom* sub)
{
    edge new_edge;
    
    // do not add loops!
    if(v1->id == v2->id)
        return;
    
    //TODO: check visibility
    if(checkVisibility(v1->nav_line, v2->nav_line, sub)) {
	new_edge.next_vertex = v2;
	// attention!! this is not the shortest distance.
	new_edge.distance = (v1->nav_line->GetCentre() - v2->nav_line->GetCentre()).Norm();
	new_edge.sub = sub; 
	v1->edges.push_back(new_edge);
    }
};

bool RoutingGraph::checkVisibility(Line* l1, Line* l2, SubRoom* sub)
{
    // generate certain connection lines
    // connecting p1 mit p1, p1 mit p2, p2 mit p1, p2 mit p2 und center mit center
    Line cl[5];
    cl[0] = Line(l1->GetPoint1(), l2->GetPoint1());
    cl[1] = Line(l1->GetPoint1(), l2->GetPoint2());
    cl[2] = Line(l1->GetPoint2(), l2->GetPoint1());
    cl[3] = Line(l1->GetPoint2(), l2->GetPoint2());
    cl[4] = Line(l1->GetCentre(), l2->GetCentre());
    bool temp[5] = {true, true, true, true, true};
    //check intersection with Walls
    for(unsigned int i = 0; i < sub->GetAllWalls().size(); i++) {
	for(int k = 0; k < 5; k++) {
	    if(temp[k] && cl[k].IntersectionWith(sub->GetAllWalls()[i]) && (cl[k].NormalVec() != sub->GetAllWalls()[i].NormalVec() ||  l1->NormalVec() != l2->NormalVec()))
		temp[k] = false;
	}
    }

    //check intersection with obstacles
    for(unsigned int i = 0; i < sub->GetAllObstacles().size(); i++) {
	Obstacle * obs = sub->GetAllObstacles()[i];
	for(unsigned int k = 0; k<obs->GetAllWalls().size(); k++){
	    Wall w = obs->GetAllWalls()[k];
	    for(int j = 0; j < 5; j++) {
		if(temp[j] && cl[j].IntersectionWith(w))
		    temp[j] = false;
	    }
	}
    }

    // check intersection with other hlines in room
    for(unsigned int i = 0; i < sub->GetAllHlines().size(); i++) {
	if(l1 != sub->GetAllHlines()[i] && l2 != sub->GetAllHlines()[i]) {
	    for(int k = 0; k < 5; k++) {
		if(temp[k] && cl[k].IntersectionWith((*sub->GetAllHlines()[i])))
		    temp[k] = false;
	    }
	}
    }
    return temp[0] || temp[1] || temp[2] || temp[3] || temp[4];
}


/**
 * calculates all distances to a certain exit
 */
void RoutingGraph::calculateDistancesForExit(vertex *act_vertex)
{
    for(unsigned int k = 0; k < act_vertex->edges.size(); k++) {
        calculateDistances(act_vertex, act_vertex, k, act_vertex->edges[k].distance);
    }
}
/** 
 * recursive function to calculate distances from one vertex to all others
 * implementation like Dijkstra 
 */
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
    for(unsigned int k = 0; k < act_vertex->edges.size(); k++) {
        //if the next vertex is an exit here is nothing to do.
        if(!act_vertex->edges[k].next_vertex->exit) {
            // if the edge is through the same room then the exit here is nothing to do, except it is a hline
	    Hline * hline1 = dynamic_cast<Hline*>(act_vertex->nav_line);
            if(hline1 || !(act_vertex->distances[exit->id].exit_subroom->GetRoomID() == act_vertex->edges[k].sub->GetRoomID() && act_vertex->distances[exit->id].exit_subroom->GetSubRoomID() == act_vertex->edges[k].sub->GetSubRoomID())) {
                calculateDistances(exit, act_vertex, k, act_vertex->edges[k].distance + act_distance);
            }
        }
    }
}

/**
 * debug printing function
 */
void RoutingGraph::print()
{
    map<int, vertex>::iterator it;
    map<int, exit_distance>::iterator it2;
    std::cout << vertexes.size() << std::endl; 
    for(it=vertexes.begin(); it != vertexes.end(); it++ ) {
        std::cout << "\n\nvertex: " << (*it).second.nav_line->GetUniqueID();
	
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
        for(unsigned int i = 0; i < (*it).second.edges.size(); i++) {
            if((*it).second.edges[i].next_vertex)
                std::cout << (*it).second.edges[i].next_vertex->nav_line->GetUniqueID() << "(distance " << (*it).second.edges[i].distance <<" - ";
            else
                std::cout << "NULL" << "-" ;
        }
        std::cout << std::endl << std::endl; 
        std::cout << "exit distances: \n ";

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


vertex * RoutingGraph::GetVertex(int id)
{
    return & vertexes[id];
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

//  LocalWords:  subroom
