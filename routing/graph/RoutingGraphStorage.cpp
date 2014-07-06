/**
 * \file        RoutingGraphStorage.cpp
 * \date        Sep 10, 2012
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


#include "RoutingGraphStorage.h"

using namespace std;


RoutingGraphStorage::RoutingGraphStorage()
{
}


RoutingGraphStorage::~RoutingGraphStorage()
{
     map<set<int>, RoutingGraph*>::iterator it;

     for(it = graphs.begin(); it != graphs.end(); it++) {
          delete it->second;

     }

}


void RoutingGraphStorage::init(Building * b)
{
     //create the first RoutingGraph with empty set as Key (no door closed right at the beginning

     graphs[empty_set] = new RoutingGraph(b);
}


RoutingGraph * RoutingGraphStorage::GetGraph(set<int> closed_doors)
{
     map<set<int>, RoutingGraph*>::iterator it;
     // check if the graph with closed doors already exists
     it = graphs.find(closed_doors);
     if(it != graphs.end()) {
          return it->second;
     }

     // find the biggest existing subset

     GenerateNewGraph(closed_doors);

     return graphs[closed_doors];
}

void RoutingGraphStorage::GenerateNewGraph(set<int> iset, set<int> from)
{
     set<int>::iterator it;
     graphs[iset] = new RoutingGraph(GetGraph(from));

     for(it = iset.begin(); it != iset.end(); it++) {
          //check if UniqueID is already closed in the "from" graph
          if(from.find(*it) == from.end()) {
               graphs[iset]->closeDoor(*it);

          }

     }
     return;

}

/**
 * Returns the biggest subset for which a graph already exists
 * not finished yet ;)
 */
set<int> RoutingGraphStorage::GetBiggestSubSet(set<int> iset) const
{
     set<int> oset;
     for(int i = iset.size(); i > 1; i--) {
          //check all subsets of size i

     }
     return oset;

}
