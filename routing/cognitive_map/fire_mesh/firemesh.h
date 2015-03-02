/**
 * \file        firemesh.h
 * \date        Jan 1, 2015
 * \version     v0.7
 * \copyright   <2009-2015> Forschungszentrum Jülich GmbH. All rights reserved.
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
 * 2D-Mesh. Division of the geometry into a cartesian equidistant mesh
 *
 *
 **/

#ifndef FIREMESH_H
#define FIREMESH_H

#include <vector>
#include <string>
#include "knot.h"

//using Matrix = std::array<std::array<T, COL>, ROW>;


using Matrix = std::vector<std::vector<knot>>;

class FireMesh
{
public:
    FireMesh();
    FireMesh(const double &xmin, const double &ymin, const double &xmax, const double &ymax, const int &cellsize);
    FireMesh(const std::string &filename);
    ~FireMesh();

    void setUpMesh(const double &xmin, const double &ymin, const double &xmax, const double &ymax, const int &cellsize);
    const Matrix &getMesh() const;

    double getKnotValue(const double &x, const double &y) const;
    void setKnotValuesFromFile(const std::string &filename);

    bool statusMesh() const;


private:
    Matrix _matrix;
    int _cellsize;
    double _xmin;
    double _xmax;
    double _ymin;
    double _ymax;

    //bool statHeaderRead;
    bool statMesh;

};

#endif // FIREMESH_H
