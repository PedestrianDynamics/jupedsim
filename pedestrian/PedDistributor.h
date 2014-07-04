/**
 * \file        PedDistributor.h
 * \date        Oct 12, 2010
 * \version     v0.5
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

#include "../math/Distribution.h"
#include "../routing/Router.h"
#include "../geometry/Building.h"



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
     int _age;
     int _height;
     double _patience;
     //string motivation;// low, normal, high
     double _startX; //only valid when _nPeds=1
     double _startY; //only valid when _nPeds=1
     double _startZ; //only valid when _nPeds=1

     //distributing in a square
     double _xMin;
     double _xMax;
     double _yMin;
     double _yMax;

     std::string _gender;

public:
     StartDistributionRoom();
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
     void SetStartPosition(double x,double y, double z);
     Point GetStartPosition() const;
     double GetPatience() const;
     void SetPatience(double patience);
     void SetBounds(double xMin, double xMax, double yMin, double yMax);
     void Getbounds(double bounds[4]);
     void Setbounds(double bounds[4]);
};

//TODO merge the two classes and set the _subRoomID=-1
class StartDistributionSubroom : public StartDistributionRoom {
private:
     int _subroomID;

public:
     StartDistributionSubroom();
     virtual ~StartDistributionSubroom();

     int GetSubroomID() const;
     void SetSubroomID(int i);
};

/************************************************************
 PedDistributor
************************************************************/
class PedDistributor {
private:
     Distribution* _v0; // Gauss - Verteilung für v0
     Distribution* _Bmax;
     Distribution* _Bmin;
     Distribution* _Atau;
     Distribution* _Amin;
     Distribution* _Tau;
     std::vector<StartDistributionRoom*> _start_dis; // ID startraum, subroom und Anz
     std::vector<StartDistributionSubroom*> _start_dis_sub; // ID startraum, subroom und Anz
     std::string _projectFilename; // store the file for later user

     // find aped in a subroom and delete him
     bool FindPedAndDeleteFromRoom(Building* building,Pedestrian*ped) const;

public:
     // Konstruktoren
     PedDistributor();
     PedDistributor(double v0mu, double v0sigma, double BmaxMu, double BmaxSigma,
                    double BminMu, double BminSigma, double AtauMu, double AtauSigma, double AminMu,
                    double AminSigma, double tauMu, double tauSigma);
     //PedDistributor(const PedDistributor& orig);
     virtual ~PedDistributor();
     // Getter-Funktionen
     Distribution* GetV0() const;
     Distribution* GetBmax() const;
     Distribution* GetBmin() const;
     Distribution* GetAtau() const;
     Distribution* GetAmin() const;
     Distribution* GetTau() const;

     // sonstige Funktionen
     std::vector<Point> PositionsOnFixX(double max_x, double min_x, double max_y, double min_y,
                                        SubRoom* r, double bufx, double bufy, double dy) const;
     std::vector<Point> PositionsOnFixY(double max_x, double min_x, double max_y, double min_y,
                                        SubRoom* r, double bufx, double bufy, double dx) const;
     std::vector<Point> PossiblePositions(SubRoom* r) const;
     void DistributeInSubRoom(SubRoom* r, int N, std::vector<Point>& positions, int* pid, StartDistributionSubroom* parameters,Building* building) const;
     std::string writeParameter() const;

     void InitDistributor(const std::string &start_file);
     int Distribute(Building* building) const;
};

#endif  /* _PEDDISTRIBUTOR_H */

