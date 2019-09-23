/**
 * \file        QuickestPathRouter.h
 * \date        Apr 20, 2011
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

#include "router/global_shortest/GlobalRouter.h"
#include "router/global_shortest/AccessPoint.h"

#include "router/Router.h"
#include "geometry/Building.h"
#include "pedestrian/Pedestrian.h"

#include <cfloat>

enum ExitState
{
     FREE_EXIT = 0,
     REF_PED_FOUND = 1,
     UNREACHEABLE_EXIT = 2
};

enum RefSelectionMode
{
     SINGLE = 1,
     ALL =2
};

enum DefaultStrategy
{
     LOCAL_SHORTEST=1,
     GLOBAL_SHORTEST=2
};

//log output
extern OutputHandler* Log;
/*!
 * \class QuickestPathRouter
 *
 * \brief Quickest path router
 *
 *\ingroup Router
 *
 * We will derive from the <Router> class to fit the interface.
 *
 * \author Ulrich Kemloh
 */
class QuickestPathRouter: public GlobalRouter{


public:
     QuickestPathRouter();

     QuickestPathRouter(int id, RoutingStrategy s):GlobalRouter(id, s) { }

     virtual ~QuickestPathRouter();

     virtual int FindExit(Pedestrian* ped);

     virtual bool Init(Building* building);

private:

     /**
      * find the next suitable destination for the pedestrian
      * @param ped
      * @return the index of the destination
      */
     int FindNextExit(Pedestrian* ped);

     /**
      * compute the best route for the pedestrian
      *
      * @param ped the pedestrian
      * @param nearestAP the nearest decision point
      * @return the best route found
      */

     int GetQuickestRoute(Pedestrian* ped);

     /**
      * compute the cost benefit analysis obtained by changing from value g1 to value g2
      * @param g1, the reference value
      * @param g2, the value to compare to
      * @return the gain
      */
     double CBA (double ref_g1, double comp_g2);

     /**
      * compute the gain related to this travelling time
      */
     double gain(double time);


     /**
      * \brief determines if a pedestrian is within (in a centre) of a jam.
      *
      * This function calculates the degree of congestion of a pedestrian.
      * -1 is returned if the ped has no choice than to follow his actual goal,
      * otherwise a value [0 1] is returned. 0 means free and 1 mean fully stuck
      * in jam.
      *
      * @param pedIndex, the index of the pedestrian
      * @return the degree of congestion
      */
     int isCongested(Pedestrian* ped);

     /**
      * redirect a pedestrian based on the actual traffic conditions:
      * i.e if IsCongested returned false
      * @see isCongested
      *
      * @param ped the pedestrian to be redirected
      */
     virtual void Redirect(Pedestrian* ped);

     /**
      * redirect a pedestrian based on the actual jamming conditions
      *
      * @param pedindex
      * @param actualexit
      * @return
      */
     int redirect(int pedindex,int actualexit=-1);


     /**
      * select a reference pedestrian for an exit.
      * In the case NULL is returned, check the flag to see why.
      * Reasons are:
      * flag=0: a proper reference was found
      * flag=1: the exit is free, no reference
      * flag=2: I can't see the exit, nor references, too crowded, too many crossing pedes
      */
     bool SelectReferencePedestrian(Pedestrian* me, Pedestrian** myref, double JamThreshold, int exitID, int* flag);

     /**
      * return the queue at the specified exit within the specified radius
      * if subroomToConsider == -1 then the two side of the crossing will be considered
      *
      */
     void GetQueueAtExit(Hline* hline, double minVel,
               double radius, std::vector<Pedestrian*>& queue, int subroomToConsider=-1);

     /**
      * Check if two pedestrian can see each other
      *
      * @param ped, the pedestrian being investigated
      * @param ref, the reference pedestrian
      * @return true if both can see each other
      */
     bool IsDirectVisibilityBetween(Pedestrian* ped, Pedestrian* ref);

     /**
      * Check if a pedestrian can see a crossing
      *
      * @param ped, the pedestrian being investigated
      * @param ref, the crossing/exit
      * @return true if there is a direct visibility
      */
     bool IsDirectVisibilityBetween(Pedestrian* ped, Hline* ref);

     /**
      * returns the number of obstacles between the points p1 and p2 with respect
      * to the crossing crossing. The pedestrians ped1 and ped2 are not accounted in the obstacles.
      *
      * @param p1  the start point
      * @param p2  the end point
      * @param crossing
      * @param ignore_ped1
      * @param ignore_ped2
      * @return the number of obstacles
      */
     unsigned int GetObstaclesCountBetween(const Point& p1, const Point& p2, Hline* hline, int ignore_ped1,
             int ignore_ped2);

     /**
      *
      * @param ped the pedestrian doing the estimation
      * @param exitID the exit ID
      * @return the estimated travel time via the given exit
      */
     double GetEstimatedTravelTimeVia(Pedestrian* ped, int exitID);

     /**
      * compute the best default exit when there is no information
      * @param ped
      * @return
      */
     virtual int GetBestDefaultRandomExit(Pedestrian* ped);

     /**
      * Parse extra routing information for the quickest path
      * @return
      */
     virtual bool ParseAdditionalParameters();

private:
     // [m/s] maximum speed to be considered in a queue while looking for a reference in a new room
     double _queueVelocityNewRoom=0.7;
     // [m/s] maximum speed to be considered in a queue while looking for a reference in a jam situation
     double _queueVelocityFromJam=0.2;
     // Threshold for the cost benefit analysis
     double _cbaThreshold=0.15;
     // obstructions
     unsigned int _visibilityObstruction=4;
     //congestion factor before attempting a change
     double _congestionRation=0.8;
     // reference pedestrian selection mode
     RefSelectionMode _refPedSelectionMode= RefSelectionMode::SINGLE;
     // defauult initial strategy for the pedestrians
     DefaultStrategy _defaultStrategy=DefaultStrategy::GLOBAL_SHORTEST;
};
