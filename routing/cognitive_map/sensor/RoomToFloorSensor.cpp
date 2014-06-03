/**
 * @file RoomToFloorSensor.cpp
 * @author David H. <d.haensel@fz-juelich.de>
 * @date 03, 2014
 * @brief
 */

#include "RoomToFloorSensor.h"
#include "../NavigationGraph.h"
#include "../../../geometry/Building.h"
#include "../CognitiveMap.h"
#include "../../../pedestrian/Pedestrian.h"


RoomToFloorSensor::~RoomToFloorSensor()
{
}

std::string RoomToFloorSensor::GetName() const
{
    return "RoomToFloorSensor";
}

void RoomToFloorSensor::execute(const Pedestrian * pedestrian, CognitiveMap * cognitive_map) const
{
    // NavigationGraph::VerticesContainer * vertices = cognitive_map->GetAllVertices();

    // //iterating over all Vertices
    // for(NavigationGraph::VerticesContainer::iterator it_vertices = vertices->begin(); it_vertices != vertices->end(); ++it_vertices) {
    //     GraphVertex::EdgesContainer * edges = (it_vertices->second)->GetAllEdges();
    //     //iterating over all edges
    //     for(GraphVertex::EdgesContainer::iterator it_edges = edges->begin(); it_edges != edges->end(); ++it_edges) {
    //         if((*it_edges)->GetDest() == NULL || (*it_edges)->GetDest()->GetSubRoom()->GetType() == (*it_edges)->GetSrc()->GetSubRoom()->GetType()) {
    //             (*it_edges)->SetFactor(1.0, GetName());
    //         } else {
    //             if((*it_edges)->GetDest()->GetSubRoom()->GetType() == "floor")
    //                 (*it_edges)->SetFactor(.5 , GetName());
    //             else
    //                 (*it_edges)->SetFactor(3.0 , GetName());
    //         }
    //     }
    // }

    SubRoom * sub_room = building->GetRoom(pedestrian->GetRoomID())->GetSubRoom(pedestrian->GetSubRoomID());
    GraphVertex * vertex = (*cognitive_map->GetNavigationGraph())[sub_room];
    const GraphVertex::EdgesContainer * edges = vertex->GetAllOutEdges();
    for(GraphVertex::EdgesContainer::iterator it_edges = edges->begin(); it_edges != edges->end(); ++it_edges) {
        if((*it_edges)->GetDest() == NULL || (*it_edges)->GetDest()->GetSubRoom()->GetType() == (*it_edges)->GetSrc()->GetSubRoom()->GetType()) {
            (*it_edges)->SetFactor(1.0, GetName());
        } else {
            if((*it_edges)->GetDest()->GetSubRoom()->GetType() == "floor")
                (*it_edges)->SetFactor(.3 , GetName());
            else
                (*it_edges)->SetFactor(5.0 , GetName());
        }
    }
}
