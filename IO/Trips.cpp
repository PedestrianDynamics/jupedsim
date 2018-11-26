//
// Created by Tobias Schrödter on 23.11.18.
//

#include "Trips.h"
//#include <random>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/discrete_distribution.hpp>

std::ostream& operator<<(std::ostream& os, const Trips& trips)
{
     for (auto v : boost::make_iterator_range(boost::vertices(trips.trips))){
          os << "Vertex " << v << " " << trips.trips[v] << "\n";
     }

     for (auto e : boost::make_iterator_range(boost::edges(trips.trips))){
          os << "Edge " << e << " " << trips.trips[e] << "\n";
     }

     return os;
}

void Trips::addGoal(int id)
{
     VertexItr vi, vi_end;
     boost::tie(vi, vi_end) = boost::vertices(trips);
     vi = getGoal(id);

     // check if goal with id already exists
     if (vi == vi_end) {
          boost::add_vertex(VertexProperty{id}, trips);
          return;
     }
}

VertexItr Trips::getGoal(int id)
{
     VertexItr vi, vi_end;
     for (boost::tie(vi, vi_end) = boost::vertices(trips); vi != vi_end; ++vi) {
          if(trips[*vi].getID() == id) return vi;
     }
     return vi_end;}

void Trips::addConnection(int sourceId, int destinationId, EdgeProperty& weight)
{
     VertexItr vSource, vDestination, vEnd;
     boost::tie(vSource, vEnd) = boost::vertices(trips);

     // Check if vertex with sourceId exists, if not add it
     vSource = getGoal(sourceId);
     if (vSource == vEnd){
          addGoal(sourceId);
     }

     // Check if vertex with destinationId exists, if not add it
     boost::tie(vSource, vEnd) = boost::vertices(trips);
     vDestination = getGoal(destinationId);
     if (vDestination == vEnd){
          addGoal(destinationId);
     }

     // Get updated iterator to source and destination, needed if one vertex had to be added
     vSource = getGoal(sourceId);
     vDestination = getGoal(destinationId);

     // Finally add the edge
     boost::add_edge(*vSource, *vDestination, weight, trips);
}

int Trips::getNextGoal(int id)
{
     VertexItr vCurrent = getGoal(id);
     VertexItr vEnd = boost::vertices(trips).second;

     // Check if edge exists, if not there is no next goal (should never happen, check the ini file)
     if (vCurrent == vEnd){
          return -1;
     }

     // Create vector with possible targets and correspondig weights
     std::vector<double> weights;
     std::vector<long> targets;

     for (auto e : boost::make_iterator_range(boost::out_edges(*vCurrent, trips))){
          weights.push_back(trips[e].getProbabilty());
          targets.push_back(e.m_target);
     }

     // Random number generator
//     std::random_device rd;
//     std::mt19937 gen(rd());
//
//     std::default_random_engine generator;
//     std::discrete_distribution<double> distribution (weights.begin(), weights.end());

     boost::mt19937 gen;
     boost::random::discrete_distribution<> dist(weights.begin(), weights.end());

     // Map random number to target
     return targets[(int)dist(gen)];
}