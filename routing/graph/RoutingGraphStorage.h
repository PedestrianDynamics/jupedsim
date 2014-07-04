/**
 * \file        RoutingGraphStorage.h
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


#ifndef ROUTINGGRAPHSTORAGE_H_
#define ROUTINGGRAPHSTORAGE_H_

#include "RoutingGraph.h"
#include <map>
#include <set>


class RoutingGraphStorage {

public:
     RoutingGraphStorage();
     ~RoutingGraphStorage();
     void init(Building * b);
     RoutingGraph * GetGraph(std::set<int> closed_doors);



private:
     std::map<std::set<int>, RoutingGraph*> graphs;
     std::set<int> empty_set;
     // do not use this function now
     std::set<int> GetBiggestSubSet(std::set<int> iset) const;

     /**
      * @param iset
      * @param from
      */
     void GenerateNewGraph(std::set<int> iset, std::set<int> from = std::set<int>());



};
#endif /* ROUTINGGRAPHSTORAGE_H_ */
