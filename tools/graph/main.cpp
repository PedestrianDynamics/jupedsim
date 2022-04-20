#include <boost/array.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/named_function_params.hpp>
#include <boost/graph/properties.hpp>
#include <boost/pending/property.hpp>
#include <boost/property_map/property_map.hpp>
#include <iostream>
#include <vector>

class Graph
{
};

class GraphBuilder
{
};

using BoostGraph = boost::adjacency_list<
    boost::vecS,
    boost::vecS,
    boost::undirectedS,
    boost::no_property,
    boost::property<boost::edge_weight_t, int>>;
using Edge = std::pair<int, int>;

void print_path(const std::vector<BoostGraph::vertex_descriptor> & paths, int start, int goal)
{
    int current = start;
    while(current != goal) {
        std::cout << current << std::endl;
        current = paths[current];
    }
    std::cout << current << std::endl;
}

int main()
{
    std::vector<Edge> edges{{1, 2}, {2, 3}, {2, 4}, {3, 5}, {4, 5}};
    std::vector<int> weights{1, 1, 2, 1, 1};
    BoostGraph graph{std::begin(edges), std::end(edges), std::begin(weights), 5};
    std::vector<BoostGraph::vertex_descriptor> predecessors(num_vertices(graph));
    boost::dijkstra_shortest_paths(
        graph,
        5,
        boost::predecessor_map(boost::make_iterator_property_map(
            predecessors.begin(), get(boost::vertex_index, graph))));
    print_path(predecessors, 1, 5);
    return 0;
}
