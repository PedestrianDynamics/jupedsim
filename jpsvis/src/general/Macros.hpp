/**
 * \file        Macros.h
 * \date        Jun 16, 2010
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
 * useful constants
 *
 *
 **/
#pragma once

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

#define VTK_CREATE(type, name) vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define J_EPS 0.001
#define J_EPS_EVENT 0.00001 // zum pruefen des aktuellen Zeitschrittes auf events
#define J_EPS_DIST 0.05 // [m]

#define J_EPS_GOAL 0.005 /// [m] Abstand zum Ziel, damit Fußgänger immer zu einem Raum gehört
#define J_TOLERANZ 0.03 /// [m] Toleranz beim erstellen der Linien
#define J_EPS_V 0.1 /// [m/s] wenn  v<EPS_V wird mit 0 gerechnet

// Length of array
#define CLENGTH 1000

// conversion (cm <-> m)
#define FAKTOR 100

// Linked cells
#define LIST_EMPTY -1

// global functions for convenience

inline int xmltoi(const char* t, int v = 0)
{
    if(t && (*t))
        return atoi(t);
    return v;
}
inline double xmltof(const char* t, double v = 0.0)
{
    if(t && (*t))
        return atof(t);
    return v;
}
inline const char* xmltoa(const char* t, const char* v = "")
{
    if(t)
        return t;
    return v;
}

/**
 * @return true if the element is present in the vector
 */
template <typename A>
inline bool IsElementInVector(const std::vector<A>& vec, const A& el)
{
    typename std::vector<A>::const_iterator it;
    it = std::find(vec.begin(), vec.end(), el);
    if(it == vec.end()) {
        return false;
    } else {
        return true;
    }
}
