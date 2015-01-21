/**
 * \file        Knowledge.h
 * \date        Sep 18, 2012
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

#ifndef KNOWLEDGE_H_
#define KNOWLEDGE_H_

#include <iostream>

class Knowledge
{
public:
     /**
      * Constructor
      */
     Knowledge();

     /**
      * Destructor
      */
     ~Knowledge();

     void Dump();
     void SetState(int id, bool is_closed, double time);
     bool GetState() const;
     double GetQuality() const;
     double GetTime() const;

private:
     /// information quality in [0..1]. 1 is very reliable information
     double _quality; //
     /// last time the state was recorded.
     double _time;
     /// state 0=open, 1=close
     bool _isClosed;
     /// id of the door
     int _id;

};
#endif /* KNOWLEDGE_H_ */

