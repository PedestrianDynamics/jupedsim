#include "landmarknetwork.h"
#include "region.h"

//    In the following example, we construct a graph and apply dijkstra_shortest_paths().
//            The complete source code for the example is in examples/dijkstra-example.cpp.
//            Dijkstra's algorithm computes the shortest distance from the starting vertex to every other vertex in the graph.

//    Dijkstra's algorithm requires that a weight property is associated with each edge and a distance property with each vertex.
//            Here we use an internal property for the weight and an external property for the distance.
//            For the weight property we use the property class and specify int as the type used to represent
//            weight values and edge_weight_t for the property tag (which is one of the BGL predefined property tags).
//            The weight property is then used as a template argument for adjacency_list.

//    The listS and vecS types are selectors that determine the data structure used inside the adjacency_list
//            (see Section Choosing the Edgelist and VertexList). The directedS type specifies that the graph
//            should be directed (versus undirected). The following code shows the specification of the graph
//            type and then the initialization of the graph. The edges and weights are passed to the graph
//            constructor in the form of iterators (a pointer qualifies as a RandomAccessIterator).


//    For the external distance property we will use a std::vector for storage.
//    BGL algorithms treat random access iterators as property maps,
//    so we can just pass the beginning iterator of the distance
//    vector to Dijkstra's algorithm. Continuing the above example,
//    the following code shows the creation of the distance vector,
//    the call to Dijkstra's algorithm (implicitly using the
//    internal edge weight property), and then the output of the results.

//    typedef adjacency_list<listS, vecS, directedS,
//                           no_property, property<edge_weight_t, int> > Graph;
//    typedef graph_traits<Graph>::vertex_descriptor Vertex;
//    typedef std::pair<int,int> E;

//    const int num_nodes = 5;
//    E edges[] = { E(0,2),
//                  E(1,1), E(1,3), E(1,4),
//                  E(2,1), E(2,3),
//                  E(3,4),
//                  E(4,0), E(4,1) };
//    int weights[] = { 1, 2, 1, 2, 7, 3, 1, 1, 1};

//    Graph G(edges, edges + sizeof(edges) / sizeof(E), weights, num_nodes);


//    // vector for storing distance property
//    std::vector<int> d(num_vertices(G));

//    // get the first vertex
//    Vertex s = *(vertices(G).first);
//    // invoke variant 2 of Dijkstra's algorithm
//    dijkstra_shortest_paths(G, s, distance_map(&d[0]));

//    std::cout << "distances from start vertex:" << std::endl;
//    graph_traits<Graph>::vertex_iterator vi;
//    for(vi = vertices(G).first; vi != vertices(G).second; ++vi)
//      std::cout << "distance(" << index(*vi) << ") = "
//                << d[*vi] << std::endl;
//    std::cout << std::endl;


AILandmarkNetwork::AILandmarkNetwork()
{
    _graph=Graph();
}

AILandmarkNetwork::AILandmarkNetwork(const AIRegion* region, const AILandmarks &landmarks, const std::vector<AIConnection> &connections)
{
    _region=region;
    _graph=Graph();

    for (const AILandmark& landmark:landmarks)
    {
        AddLandmark(&landmark);
    }


    for (const AIConnection& connection:connections)
    {
        AddConnection(&connection);

    }
}


AILandmarkNetwork::~AILandmarkNetwork()
{

}

void AILandmarkNetwork::AddLandmark(const AILandmark *landmark)
{

    Vertex v = boost::add_vertex(_graph);
    //std::make_pair<int,int>(1,1);
    _landmarks.emplace(landmark,v);

}

void AILandmarkNetwork::RemoveLandmark(const AILandmark *landmark)
{


    for (auto it=_landmarks.begin(); it!=_landmarks.end(); ++it)
    {
        if (it->first==landmark)
        {
            boost::clear_vertex(it->second,_graph);
            RemoveAdjacentEdges(it->second);
            boost::remove_vertex(it->second,_graph);
            _landmarks.erase(it);
            break;
        }
    }



}

void AILandmarkNetwork::AddConnection(const AIConnection *connection)
{
    //find indeces of vertices(landmarks) in graph
    const AILandmark* landmarkA = _region->GetLandmarkByID(connection->GetLandmarkIds().first);
    const AILandmark* landmarkB = _region->GetLandmarkByID(connection->GetLandmarkIds().second);
    Vertex A = _landmarks[landmarkA];
    Vertex B = _landmarks[landmarkB];

//    for (auto it=_landmarks.begin(); it!=_landmarks.end(); ++it)
//    {
//        int counter=0;
//        if (it->first==landmarkA)
//        {
//            A = it->second;
//            counter++;
//            if (counter==2)
//                break;
//        }
//        else if (it->first==landmarkB)
//        {
//            B = it->second;
//            counter++;
//            if (counter==2)
//                break;
//        }
//    }

    Point vector = landmarkA->GetRandomPoint()-landmarkB->GetRandomPoint();//->GetRandomPoint()-landmarkB->GetRandomPoint();
    double distance = vector.Norm();
    _connections.push_back(std::pair<Edge,Weight>(Edge(A,B),distance));

    boost::add_edge(A,B,distance,_graph);
    boost::add_edge(B,A,distance,_graph);
}

std::pair<std::vector<const AILandmark*>,double> AILandmarkNetwork::LengthofShortestPathToTarget(const AILandmark *landmark, const AILandmark *target) const
{

    if (landmark==target)
        return std::make_pair<std::vector<const AILandmark*>,double>(std::vector<const AILandmark*>{landmark},0.0);

    auto startVertex = _landmarks.at(landmark);
    auto targetVertex = _landmarks.at(target);

    // vector for storing distance property
    std::vector<Vertex> p(boost::num_vertices(_graph));
    std::vector<double> d(boost::num_vertices(_graph));

//    for (auto it =_connections.begin(); it!=_connections.end(); ++it)
//    {
//        edgeWeights.push_back(it->second);
//    }

    // invoke variant 2 of Dijkstra's algorithm
    //boost::dijkstra_shortest_paths(_graph, startVertex, boost::distance_map(&d[0]));
    boost::dijkstra_shortest_paths(_graph, startVertex,
                              boost::predecessor_map(boost::make_iterator_property_map(p.begin(), get(boost::vertex_index, _graph))).
                              distance_map(boost::make_iterator_property_map(d.begin(), get(boost::vertex_index, _graph))));

   //std::cout << "distances and parents:" << std::endl;
   Vertex vi = targetVertex;


   std::vector<const AILandmark*> landmarksOnShortestPath;

   while (vi!=startVertex)
   {
        for (auto it=_landmarks.begin(); it!=_landmarks.end(); ++it)
        {
            if (it->second==vi)
            {
                landmarksOnShortestPath.push_back(it->first);

                break;
            }
            //Log->Write(std::to_string(vi));

        }
        vi=p[vi];

      //std::cout << d[targetVertex] << std::endl;
      //std::cout << "parent(" << name[*vi] << ") = " << name[p[*vi]] << std::endl;
    }
    landmarksOnShortestPath.push_back(landmark);
    //std::cout << "distance from start vertex to target:" << std::endl;
//    boost::graph_traits<Graph>::vertex_iterator vi;
//    for(vi = boost::vertices(_graph).first; vi != boost::vertices(_graph).second; ++vi)
//        std::cout << "distance = "  << d[*vi] << std::endl;

    //Log->Write(std::to_string(landmarksOnShortestPath.size()));
    return std::make_pair(landmarksOnShortestPath,d[targetVertex]);
}

void AILandmarkNetwork::RemoveAdjacentEdges(const Vertex &vertex)
{
    for (auto it=_connections.begin(); it!=_connections.end(); ++it)
    {
        if (it->first.first == vertex || it->first.second == vertex)
        {
            _connections.remove(*it);
        }
    }
}

