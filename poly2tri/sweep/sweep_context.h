/**
 * \file        sweep_context.h
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


#ifndef SWEEP_CONTEXT_H
#define SWEEP_CONTEXT_H

#include <list>
#include <vector>
#include <cstddef>

namespace p2t {

// Inital triangle factor, seed triangle will extend 30% of
// PointSet width to both left and right.
const double kAlpha = 0.3;

struct Point;
class Triangle;
struct Node;
struct Edge;
class AdvancingFront;

class SweepContext {
public:

     /// Constructor
     SweepContext(std::vector<Point*> polyline);
     /// Destructor
     ~SweepContext();

     void set_head(Point* p1);

     Point* head();

     void set_tail(Point* p1);

     Point* tail();

     int point_count();

     Node& LocateNode(Point& point);

     void RemoveNode(Node* node);

     void CreateAdvancingFront(std::vector<Node*> nodes);

     /// Try to map a node to all sides of this triangle that don't have a neighbor
     void MapTriangleToNodes(Triangle& t);

     void AddToMap(Triangle* triangle);

     Point* GetPoint(const int& index);

     Point* GetPoints();

     void RemoveFromMap(Triangle* triangle);

     void AddHole(std::vector<Point*> polyline);

     void AddPoint(Point* point);

     AdvancingFront* front();

     void MeshClean(Triangle& triangle);

     std::vector<Triangle*> GetTriangles();
     std::list<Triangle*> GetMap();

     std::vector<Edge*> edge_list;

     struct Basin {
          Node* left_node;
          Node* bottom_node;
          Node* right_node;
          double width;
          bool left_highest;

          Basin() : left_node(NULL), bottom_node(NULL), right_node(NULL), width(0.0), left_highest(false) {
          }

          void Clear() {
               left_node = NULL;
               bottom_node = NULL;
               right_node = NULL;
               width = 0.0;
               left_highest = false;
          }
     };

     struct EdgeEvent {
          Edge* constrained_edge;
          bool right;

          EdgeEvent() : constrained_edge(NULL), right(false) {
          }
     };

     Basin basin;
     EdgeEvent edge_event;

private:

     friend class Sweep;

     std::vector<Triangle*> triangles_;
     std::list<Triangle*> map_;
     std::vector<Point*> points_;

     // Advancing front
     AdvancingFront* front_;
     // head point used with advancing front
     Point* head_;
     // tail point used with advancing front
     Point* tail_;

     Node *af_head_, *af_middle_, *af_tail_;

     void InitTriangulation();
     void InitEdges(std::vector<Point*> polyline);

};

inline AdvancingFront* SweepContext::front()
{
     return front_;
}

inline int SweepContext::point_count()
{
     return points_.size();
}

inline void SweepContext::set_head(Point* p1)
{
     head_ = p1;
}

inline Point* SweepContext::head()
{
     return head_;
}

inline void SweepContext::set_tail(Point* p1)
{
     tail_ = p1;
}

inline Point* SweepContext::tail()
{
     return tail_;
}

}

#endif
