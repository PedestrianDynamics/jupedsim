/**
 * \file        advancing_front.h
 * \date        Jul 4, 2014
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


#ifndef ADVANCED_FRONT_H
#define ADVANCED_FRONT_H

#include "../common/shapes.h"

namespace p2t {

struct Node;

// Advancing front node
struct Node {
     Point* point;
     Triangle* triangle;

     Node* next;
     Node* prev;

     double value;

     Node(Point& p) : point(&p), triangle(NULL), next(NULL), prev(NULL), value(p.x) {
     }

     Node(Point& p, Triangle& t) : point(&p), triangle(&t), next(NULL), prev(NULL), value(p.x) {
     }

};

// Advancing front
class AdvancingFront {
public:

     AdvancingFront(Node& head, Node& tail);
     // Destructor
     ~AdvancingFront();

     Node* head();
     void set_head(Node* node);
     Node* tail();
     void set_tail(Node* node);
     Node* search();
     void set_search(Node* node);

     /// Locate insertion point along advancing front
     Node* LocateNode(const double& x);

     Node* LocatePoint(const Point* point);

private:

     Node* head_, *tail_, *search_node_;

     Node* FindSearchNode(const double& x);
};

inline Node* AdvancingFront::head()
{
     return head_;
}
inline void AdvancingFront::set_head(Node* node)
{
     head_ = node;
}

inline Node* AdvancingFront::tail()
{
     return tail_;
}
inline void AdvancingFront::set_tail(Node* node)
{
     tail_ = node;
}

inline Node* AdvancingFront::search()
{
     return search_node_;
}

inline void AdvancingFront::set_search(Node* node)
{
     search_node_ = node;
}

}

#endif
