/**
 * \file        NavMesh.h
 * \date        Oct 29, 2012
 * \version     v0.6
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

#ifndef NAVMESH_H_
#define NAVMESH_H_

//#define _CGAL

// CGAL libs
#ifdef _CGAL
#include "ConvexDecomp.h"
#include "Triangulation.h"
#endif

#include "../geometry/Point.h"
#include "cmath"
#include "../geometry/Building.h"
#include <string>
#include <vector>
#include <iostream>

class NavMesh
{

public:
     class JVertex
     {
     public:
          Point pPos;
          int id;bool operator==(const JVertex& v) const
          {
               return v.pPos == pPos;
          }
          bool operator<(const JVertex& v) const
          {
               return v.pPos._x < pPos._x;
          }
          bool operator>(const JVertex& v) const
          {
               return v.pPos._x > pPos._x;
          }

     };

     class JNode
     {
     public:
          std::string pGroup;
          Point pCentroid;
          int id;
          std::vector<JVertex> pHull;
          std::vector<int> pObstacles;
          std::vector<int> pPortals;
          double pNormalVec[3];

          bool operator()(JNode*a, JNode* b)
          {
               return a->pGroup < b->pGroup;
          }

          bool operator==(const JNode& other)
          {

               if (pCentroid != other.pCentroid)
                    return false;
               if (pObstacles.size() != other.pObstacles.size())
                    return false;
               if (pPortals.size() != other.pPortals.size())
                    return false;

               // the obstacles and the portals are sorted
               for (unsigned int s = 0; s < pPortals.size(); s++) {
                    if (pPortals[s] != other.pPortals[s])
                         return false;
               }
               for (unsigned int s = 0; s < pObstacles.size(); s++) {
                    if (pObstacles[s] != other.pObstacles[s])
                         return false;
               }
               return true;
          }

          ///http://stackoverflow.com/questions/471962/how-do-determine-if-a-polygon-is-complex-convex-nonconvex
          bool IsConvex()
          {

               unsigned int hsize = pHull.size();
               unsigned int pos = 0;
               unsigned int neg = 0;

               for (unsigned int i = 0; i < hsize; i++) {
                    Point vecAB = pHull[(i + 1) % hsize].pPos
                              - pHull[i % hsize].pPos;
                    Point vecBC = pHull[(i + 2) % hsize].pPos
                              - pHull[(i + 1) % hsize].pPos;
                    double det = vecAB.Det(vecBC);
                    if (fabs(det) < J_EPS)
                         det = 0.0;

                    if (det < 0.0) {
                         neg++;
                    } else if (det > 0.0) {
                         pos++;
                    } else {
                         pos++;
                         neg++;
                    }

               }

               if ((pos == hsize) || (neg == hsize)) {
                    return true;
               }
               return false;
          }

          ///http://stackoverflow.com/questions/9473570/polygon-vertices-clockwise-or-counterclockwise/
          bool IsClockwise()
          {
               if (pHull.size() < 3) {
                    std::cerr << "You need at least 3 vertices. JNode ID ["
                              << id << " ]" << std::endl;
                    exit(EXIT_FAILURE);
               }

               Point vecAB = pHull[1].pPos - pHull[0].pPos;
               Point vecBC = pHull[2].pPos - pHull[1].pPos;

               double det = vecAB.Det(vecBC);
               if (fabs(det) < J_EPS)
                    det = 0.0;

               return (det <= 0.0);
          }

     };

     class JEdge
     {
     public:
          int id;
          JVertex pStart;
          JVertex pEnd;
          int pNode0;
          int pNode1;
          JEdge()
          {
               id = -1;
               pNode0 = -1;
               pNode1 = -1;
          }
     };

     class JObstacle
     {
     public:
          int id;
          JVertex pStart;
          JVertex pEnd;
          int pNode0;
          int pNextObst;

          int GetCommonVertex(JObstacle* obst)
          {
               if (obst->pEnd.id == pEnd.id)
                    return pEnd.id;
               if (obst->pEnd.id == pStart.id)
                    return pStart.id;
               if (obst->pStart.id == pStart.id)
                    return pStart.id;
               if (obst->pStart.id == pEnd.id)
                    return pEnd.id;
               return -1;
          }
     };

private:
     std::vector<JVertex*> _vertices;
     std::vector<JEdge*> _edges;
     std::vector<JObstacle*> _obst;
     std::vector<JNode*> _nodes;
     Building* _building;

     std::vector<JNode*> new_nodes;
     std::vector<int> problem_nodes;

     //this is used for subroom with obstacles
     std::map<int, std::pair<int, int> > map_node_to_subroom;

     // Check the created navmesh for convex polygons
     // convexify the created nav mesh
     void Convexify();

     // Add the additional surrounding world JObstacle
     // and triangulate
     void Finalize();

     void FinalizeAlphaShape();

     /**
      * @return true if there is a visibility line between the segment and a portion of the circle
      */
     bool IsCircleVisibleFromLine(const Point& center, double radius,
               const Line& segment);

     void ComputePlaneEquation(SubRoom* sub, double* coefficents);

     void ComputeStairsEquation();
     void ComputePlanesEquation();

     // Triangulate a subroom possibly with obstacles
     void Triangulate(SubRoom* sub);
     void Triangulate(JNode* JNode);

     /// Return the id of the JEdge
     int IsPortal(Point& p1, Point& p2);
     /// Return the id of the JObstacle
     int IsObstacle(Point& p1, Point& p2);

public:

     NavMesh(Building* b);
     virtual ~NavMesh();
     void BuildNavMesh();
     void WriteToFile(std::string fileName);
     void WriteToString(std::string& output);
     void WriteToFileTraVisTo(std::string fileName);
     void WriteToFileTraVisTo(std::string fileName,
               const std::vector<Point>& points);
     void WriteToFileTraVisTo(std::string fileName, JNode* node);

     int AddVertex(JVertex* v);
     int AddEdge(JEdge* e);
     int AddObst(JObstacle* o);
     int AddNode(JNode* n);

     ///return the JVertex with the corresponding point
     JVertex* GetVertex(const Point& p);
     void DumpNode(int id);
     void DumpEdge(int id);
     void DumpObstacle(int id);

     /// Write the simulation scenario for the
     /// MENGE simulator
     void WriteScenario();
     void WriteBehavior();
     void WriteViewer();
     void WriteStartPositions();

     void UpdateEdges();
     void UpdateObstacles();
     void UpdateNodes();

     void Test();

     const std::vector<NavMesh::JEdge*>& GetEdges() const;
     const std::vector<NavMesh::JNode*>& GetNodes() const;
     const std::vector<NavMesh::JObstacle*>& GetObst() const;
     const std::vector<NavMesh::JVertex*>& GetVertices() const;

     // need to access vertices and nodes
     friend class TrajectoriesVTK;

};

#endif /* NAVMESH_H_ */
