/**
 * \file        StartDistribution.h
 * \date        Apr 15, 2015
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

#include <boost/math/distributions.hpp>

#include <vector>
#include <string>
#include <random>

//Forward declarations
class AgentsParameters;
class Pedestrian;
class Building;
class Point;

class StartDistribution
{
private:
     int _roomID;
     int _nPeds;
     int _groupID;
     int _goalID;
     int _routerID;
     int _routeID;
     int _subroomID;
     int _subroomUID;
     //demographic parameters
     //TODO: should also follow a distribution, see _premovement
     std::string _gender;
     std::string _positions_dir; // directory containing >=1 file(s) used to setup starting positions
     std::string _unit_traj; //unit of traj from file
     int _age;
     double _height;
     double _patience;

     //force model parameters
     AgentsParameters* _groupParameters;

     //string motivation;// low, normal, high
     double _startX; //only valid when _nPeds=1
     double _startY; //only valid when _nPeds=1
     double _startZ; //only valid when _nPeds=1

     //bounds for distributing in a square
     double _xMin;
     double _xMax;
     double _yMin;
     double _yMax;
     //pre movement time distribution
     mutable std::normal_distribution<double> _premovementTime;

     //risk tolerance distribution
     std::string _distribution_type;
     mutable std::normal_distribution<double> _riskTolerance;
     mutable boost::math::beta_distribution<> _risk_beta_dist;

     static bool seeded;
     //random number generator engine
     mutable std::default_random_engine _generator;

	 //judge whether sigma>0
	 const double judge = 10000;

public:
     StartDistribution(int seed);
     virtual ~StartDistribution();
     std::default_random_engine GetGenerator();
     int GetAgentsNumber() const;
     void SetRoomID(int id);
     void SetAgentsNumber(int N);
     int GetAge() const;
     void SetAge(int age);
     const std::string& GetGender() const;
     void SetGender(const std::string& gender);
     int GetGoalId() const;
     void SetGoalId(int goalId);
     int GetGroupId() const;
     void SetGroupId(int groupId);
     double GetHeight() const;
     void SetHeight(double height);
     int GetRoomId() const;
     void SetRoomId(int roomId);
     int GetSubroomID() const;
     void SetSubroomID(int subroomID);
     int GetSubroomUID() const;
     void SetSubroomUID(int subroomUID);
     int GetRouteId() const;
     void SetRouteId(int routeId);
     int GetRouterId() const;
     void SetRouterId(int routerId);
     void SetStartPosition(double x, double y, double z);
     Point GetStartPosition() const;
     double GetPatience() const;
     void SetPatience(double patience);
     void SetBounds(double xMin, double xMax, double yMin, double yMax);
     void Getbounds(double bounds[4]);
     void Setbounds(double bounds[4]);
     AgentsParameters* GetGroupParameters();
     void SetGroupParameters(AgentsParameters* groupParameters);
     void InitPremovementTime(double mean, double stdv);
     double GetPremovementTime() const;
     void InitRiskTolerance(std::string distribution_type, double para1, double para2);
     double GetRiskTolerance();
     void SetPositionsDir(const std::string& dir);
     const std::string& GetPositionsDir()  const;
     void SetUnitTraj(const std::string& unit);
     const std::string& GetUnitTraj()  const;

     Pedestrian* GenerateAgent(Building* building, int* pid, std::vector<Point>& positions);
};
