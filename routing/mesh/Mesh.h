/**
 * \file        Mesh.h
 * \date        Aug 21, 2013
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


#ifndef MESH_H_
#define MESH_H_

#include <fstream>
#include <vector>
#include "../../geometry/Point.h"
#include "../../geometry/NavLine.h"
#include "../../geometry/Crossing.h"

class MeshEdge;
class MeshCell;
class MeshCellGroup;
class Crossing;

class MeshData {
public:
     MeshData();
     ~MeshData();
     MeshData(std::vector<Point*>,std::vector<MeshEdge*>,
              std::vector<MeshEdge*>,std::vector<MeshCellGroup*>);
     std::vector<Point*> GetNodes()const {
          return _mNodes;
     }
     std::vector<MeshEdge*> GetEdges()const {
          return _mEdges;
     }
     std::vector<MeshEdge*> GetObstacles()const {
          return _mObstacles;
     }
     std::vector<MeshCellGroup*> GetCellGroups()const {
          return _mCellGroups;
     }
     unsigned int GetCellCount()const {
          return _mCellCount;
     };

     MeshCell* GetCellAtPos(int tpos)const;

     MeshCell* FindCell(Point testp,int& cell_id)const;

private:
     std::vector<Point*> _mNodes;
     std::vector<MeshEdge*> _mEdges;
     std::vector<MeshEdge*> _mObstacles;
     std::vector<MeshCellGroup*> _mCellGroups;
     unsigned int _mCellCount;

};

class MeshEdge:public Crossing {
public:
     MeshEdge(int,int,int,int,Point p1=Point(),Point p2=Point());//:Line(p1,p2);
     int GetNode1()const {
          return _n1;
     };
     int GetNode2()const {
          return _n2;
     };
     int GetCell1()const {
          return _c1;
     };
     int GetCell2()const {
          return _c2;
     };
     //friend std::istream& operator>>(std::istream& is, MeshEdge& mn);
private:
     int _n1; //ID of Node 1
     int _n2; //ID of Node 2
     int _c1; //ID of Cell 1
     int _c2; //ID of Cell 2
};

class MeshCell {
public:
     MeshCell(double,double,std::vector<int>,
              double*,std::vector<int>,std::vector<int>,int);
     ~MeshCell();
     //double get_midx(){return _midx;};
     //double get_midy(){return _midy;};
     Point GetMidpoint()const {
          return _mid;
     };
     std::vector<int> GetNodes()const {
          return _node_id;
     };
     std::vector<int> GetEdges()const {
          return _edge_id;
     };
     int GetID()const {
          return _tc_id;
     };
private:
     //double _midx;
     //double _midy;
     Point _mid;
     std::vector<int> _node_id;
     //double *_normvec;
     double _normvec[3];
     std::vector<int> _edge_id;
     std::vector<int> _wall_id;
     int _tc_id;//Cell ID unique for all cells in building
};

class MeshCellGroup {
public:
     MeshCellGroup(std::string,std::vector<MeshCell*>);
     ~MeshCellGroup();
     std::vector<MeshCell*> GetCells()const;
private:
     std::string _groupname;
     std::vector<MeshCell*> _cells;
};

unsigned int Calc_CellCount(std::vector<MeshCellGroup*> mcg);

#endif /* MESH_H_ */
