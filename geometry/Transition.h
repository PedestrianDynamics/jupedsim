/**
 * \file        Transition.h
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
 

#ifndef _TRANSITION_H
#define _TRANSITION_H

#include "Crossing.h"
#include <string>

class Room;

class Transition : public Crossing {
private:
     Room* _room2;
     bool _isOpen;
     std::string _type;
     // number of agents that passed that exit
     int _doorUsage;
     double _lastPassingTime;
     std::string _flowAtExit;

public:
     /**
      * Cosntructor
      */
     Transition();

     /**
      * Destructor
      */
     virtual ~Transition();

     /**
      * Set/Get the type of the transition
      */
     void SetType(std::string s);

     /**
      * Set/Get the second room associated with this transition.
      * The first one is set in the crossing class.
      */
     void SetRoom2(Room* ID);

     /**
      * Increment the number of persons that used that exit
      * @param number, how many person have passed the door
      * @param time, at which time
      */
     void IncreaseDoorUsage(int number, double time);

     /**
      * @return the number of pedestrians that used that exit.
      */
     int GetDoorUsage() const;


     /**
      * @return the flow curve for this exit
      */
     const std::string & GetFlowCurve() const;

     /**
      * @return the last time this door was crossed
      */
     double GetLastPassingTime() const;

     /**
      * Set/Get the type of the transition
      */
     std::string GetType() const;

     /**
      * Set/Get the second room associated with this transition.
      * The first one is set in the crossing class.
      */
     Room* GetRoom2() const;


     /**
      * @return the other room.
      */
     Room* GetOtherRoom(int room_id) const;

     // virtual functions
     virtual bool IsOpen() const;
     virtual bool IsExit() const;
     virtual bool IsTransition() const;
     virtual bool IsInRoom(int roomID) const;
     virtual SubRoom* GetOtherSubRoom(int roomID, int subroomID) const;


     virtual void WriteToErrorLog() const;
     virtual std::string GetDescription() const; // TraVisTo Ausgabe
};

#endif  /* _TRANSITION_H */

