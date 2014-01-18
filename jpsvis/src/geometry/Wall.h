/**
 * File:   Wall.h
 *
 * Created on 16. November 2010, 12:55
 *
 * @section LICENSE
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * @section DESCRIPTION
 *
 *
 *
 */

#ifndef _WALL_H
#define	_WALL_H

#include "Line.h"

class Wall : public Line {

public:
    Wall();
    Wall(const Point& p1, const Point& p2);
    Wall(const Wall& orig);

    /**
     *  Debug output from the object
     */
    void WriteToErrorLog() const;

    /**
     * @return a nicely formated string of the object
     */
    virtual std::string Write() const;
};


#endif	/* _WALL_H */

