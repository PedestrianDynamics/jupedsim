/**
 * \file        Wall.h
 * \date        Nov 16, 2010
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
 *
 *
 **/
#pragma once

#include "Line.h"

class Wall : public Line {

public:
     /**
      * Constructor
      */
     Wall();

     /**
      * Constructor
      */
     Wall(const Point& p1, const Point& p2, const std::string& type="internal");

     /**
      * Destructor
      */
     ~Wall() override = default;

     /**
      * set/get the wall type. Values are external and internal
      */
     const std::string& GetType() const;

     /**
      * set/get the wall type. Values are external and internal
      */
     void SetType(const std::string& type);

     /**
      *  Debug output from the object
      */
     void WriteToErrorLog() const;

     /**
      * @return a nicely formated string of the object
      */
     std::string Write() const override;

private:
     std::string _type;
};