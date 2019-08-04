/**
 * \file        Knowledge.h
 * \date        Sep 18, 2012
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

     /**
      * Initialize the knowledge
      * @param id
      * @param is_closed
      * @param time
      * @param quality
      * @param refuse
      */
     void SetState(int id, bool is_closed, double time, double quality, double latency, bool refuse=false);

     /**
      * @return the state of the object. close or open
      */
     bool GetState() const;

     /**
      * transmitted knowledge have a lower quality than those gathered by oneself.
      * @return the quality of the knowledge
      */
     double GetQuality() const;

     /**
      * transmitted knowledge have a lower quality than those gathered by oneself.
      * @return the quality of the knowledge
      */
     void SetQuality(double quality);

     /**
      *  When was the event recorded
      */
     double GetTime() const;

     /**
      * @return whether this knowledge has been refused before
      */
     bool HasBeenRefused() const;

     /**
      * store but refuse the knowledge.
      * @param state
      */
     void Refuse(bool state);

     /**
      * print the content of the knowledge as string
      */
     std::string Dump() const;

     /**
      * @return true, if the information can be forwarded
      */
     bool CanBeForwarded() const;

     /**
      * Set/Get the latency time before forwarding the information
      */
     void SetLatency(double latency);

     /**
      * Set/Get the latency time before forwarding the information
      */
     double GetLatency() const;

     /**
      * Update the latency by reducing the specified time.
      * The latency will eventually be 0 and the information will be forwarded.
      * @param minus
      */
     void DecreaseLatency(double minus);

private:
     /// information quality in [0..1]. 1 is very reliable information
     double _quality; //
     /// last time the state was recorded.
     double _time;
     /// state 0=open, 1=close
     bool _isClosed;
     /// id of the door
     int _id;
     /// whether I already accepted or refused that information
     bool _hasBeenRefusedOnce;
     /// time to retain this information before forwarding
     double _latency;

};
