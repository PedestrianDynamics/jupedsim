/**
 * \file        PedDistributor.h
 * \date        Oct 12, 2010
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


#ifndef _PEDDISTRIBUTOR_H
#define _PEDDISTRIBUTOR_H

#include <vector>
#include <string>

#include "../routing/Router.h"
#include "../geometry/Building.h"
#include "AgentsParameters.h"
#include "../general/ArgumentParser.h"

typedef vector< Point > tPoints;
typedef vector< tPoints >  GridPoints;

/************************************************************
 StartDistributionRoom
************************************************************/
class StartDistributionRoom {
private:
     int _roomID;
     int _nPeds;
     int _groupID;
     int _goalID;
     int _routerID;
     int _routeID;
     //demographic parameters
     //TODO: should also follow a distribution, see _premovement
     std::string _gender;
     int _age;
     int _height;
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
     std::normal_distribution<double> _premovementTime;
     std::default_random_engine _generator;


public:
    StartDistributionRoom(int seed);
    virtual ~StartDistributionRoom();

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
    int GetHeight() const;
    void SetHeight(int height);
    int GetRoomId() const;
    void SetRoomId(int roomId);
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
    void InitPremovementTime(double mean, double stv);
    double GetPremovementTime();
};

//TODO merge the two classes and set the _subRoomID=-1
class StartDistributionSubroom : public StartDistributionRoom {
private:
     int _subroomID;

public:
     StartDistributionSubroom(unsigned int seed);
     virtual ~StartDistributionSubroom();

     int GetSubroomID() const;
     void SetSubroomID(int i);
};


/************************************************************
 PedDistributor
************************************************************/
class PedDistributor {
private:
     std::vector<StartDistributionRoom*> _start_dis; // ID startraum, subroom und Anz
     std::vector<StartDistributionSubroom*> _start_dis_sub; // ID startraum, subroom und Anz
     //std::string _projectFilename; // store the file for later user
     //std::map<int, AgentsParameters*> _agentsParameters;
     bool InitDistributor(const string&, const std::map<int, AgentsParameters*>&, unsigned int);
     static std::vector<Point> PositionsOnFixX(double max_x, double min_x, double max_y, double min_y,
            const SubRoom& r, double bufx, double bufy, double dy);
     static std::vector<Point> PositionsOnFixY(double max_x, double min_x, double max_y, double min_y,
            const SubRoom& r, double bufx, double bufy, double dx);

public:
     /**
      * constructor
      */
     PedDistributor(const string& fileName, const std::map<int, AgentsParameters*>& agentPars, unsigned int seed);

     /**
      * desctructor
      */
     virtual ~PedDistributor();

     // sonstige Funktionen
     static vector<Point >  PossiblePositions(const SubRoom& r);
     void DistributeInSubRoom(SubRoom* r, int N, std::vector<Point>& positions, int* pid, StartDistributionSubroom* parameters,Building* building) const;


     bool Distribute(Building* building) const;
};

#endif  /* _PEDDISTRIBUTOR_H */

