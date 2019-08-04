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
#pragma once

#include "JPSfire/generic/Knot.h"
#include "IO/OutputHandler.h"

#include <vector>
#include <string>

//log output
extern OutputHandler* Log;

using Matrix = std::vector<std::vector<Knot> >;

class FireMesh
{
public:
    FireMesh();
    FireMesh(const double &xmin, const double &ymin, const double &xmax, const double &ymax, const double &cellsize);
    FireMesh(const std::string &filename);
    ~FireMesh();

    void SetUpMesh(const double &xmin, const double &ymin, const double &xmax, const double &ymax, const double &cellsize);
    const Matrix &GetMesh() const;

    double GetKnotValue(const double &x, const double &y) const;
    void SetKnotValuesFromFile(const std::string &filename);


    bool statusMesh() const;

private:
    Matrix _matrix;
    double _cellsize;
    double _xmin;
    double _xmax;
    double _ymin;
    double _ymax;
    bool _statMesh;

};
