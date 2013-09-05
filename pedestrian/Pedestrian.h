/**
 * File:   Pedestrian.h
 *
 *Created on 30. September 2010, 15:59
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

#ifndef _PEDESTRIAN_H
#define	_PEDESTRIAN_H

#include <vector>
#include <map>
#include <set>
#include <time.h>

#include "../pedestrian/Ellipse.h"
#include "../general/Macros.h"
#include "../routing/graph/NavLineState.h"

class Building;
class NavLine;
class Router;


class Pedestrian {
private:
	/// starting with 1
    int _id;
    double _height;
    double _age;
    double _mass; // Masse: 1
    double _tau; // Reaktinszeit: 0.5
    double _deltaT; // step size

    std::string _gender;
    std::string _roomCaption;

    int _roomID;
    int _subRoomID;
    int _exitIndex; // aktueller Ausgang, Index in routing->GetGoal(index)
    int _group;

    NavLine* _navLine; // aktuelle Ausgangslinie
    std::map<int, int>_mentalMap; // map the actual room to a destination
    std::vector<int> _destHistory;
    std::vector<int> _trip;
    Point _V0; //vector V0
    Point _lastPosition;
    int _lastCellPosition;
    
    /**
     * A set with UniqueIDs of closed crossings,
     * transitions or hlines (hlines doesnt make that much sense,
     * just that they are removed from the routing graph)
     */
    std::map<int, NavLineState> _knownDoors;
    

    //routing parameters
    double _reroutingThreshold; // new orientation after 10 seconds
    double _timeBeforeRerouting; // a new orientation starts after this time
    double _timeInJam; // actual time im Jam
    double _patienceTime; // time after which the ped feels to be in jam

    int _desiredFinalDestination;
    int _oldRoomID;
    int _oldSubRoomID;
    int _newOrientationDelay; //2 seconds, in steps


    // necessary for smooth turning at sharp bend
    int _updateRate;
    double _turninAngle;
    bool _reroutingEnabled;
    bool _tmpFirstOrientation; // possibility to get rid of this variable
    bool _newOrientationFlag;
    // the Pedestrian start after a certain amount of clockticks
    int _clockTicsTillStart;

    // the current time in the simulation
    static double _globalTime;

    /// the router responsible for this pedestrian
    Router* _router;
    JEllipse _ellipse;


public:
    // Konstruktoren
    Pedestrian();
//    Pedestrian(const Pedestrian& orig);
    virtual ~Pedestrian();

    // Setter-Funktionen
    void SetID(int i);
    //TODO: use setRoom(Room*) and setSubRoom(SubRoom*)
    void SetRoomID(int i, std::string roomCaption);
    void SetSubRoomID(int i);
    void SetMass(double m);
    void SetTau(double tau);
    void SetEllipse(const JEllipse& e);
    void SetExitIndex(int i);
    void SetExitLine(NavLine* l);
    void Setdt(double dt);
    double Getdt();
    

    // Eigenschaften der Ellipse
    void SetPos(const Point& pos); // setzt x und y-Koordinaten
    void SetCellPos(int cp);
    void SetV(const Point& v); // setzt x und y-Koordinaten der Geschwindigkeit
    void SetV0Norm(double v0);
    void SetSmoothTurning(bool smt); // activate the smooth turning with a delay of 2 sec
    void SetPhiPed();
    void SetFinalDestination(int UID);
    void SetTrip(const std::vector<int>& trip);
    void SetRouter(Router* router);

    // Getter-Funktionen
    const std::vector<int>& GetTrip() const;
    int GetID() const;
    int GetRoomID() const;
    int GetSubRoomID() const;
    double GetMass() const;
    double GetTau() const;
    const JEllipse& GetEllipse() const;
    int GetExitIndex() const;
    Router* GetRouter() const;
    NavLine* GetExitLine() const;

    // Eigenschaften der Ellipse
    const Point& GetPos() const;
    int GetCellPos() const;
    const Point& GetV() const;
    const Point& GetV0() const;
    const Point& GetV0(const Point& target);
    double GetV0Norm() const;
    double GetLargerAxis() const;
    double GetSmallerAxis() const;
    double GetTimeInJam()const;
    int GetFinalDestination() const;
    void ClearMentalMap(); // erase the peds memory

    // functions for known closed Doors (needed for the Graphrouting and Rerouting)
    void AddKnownClosedDoor(int door);
    std::set<int>  GetKnownClosedDoors();
    void MergeKnownClosedDoors(std::map<int, NavLineState> * input);
    std::map<int, NavLineState> * GetKnownDoors();
    int DoorKnowledgeCount() const;
    
    

    int GetUniqueRoomID() const;
    int GetNextDestination();
    int GetLastDestination();
    int GetDestinationCount();
    double GetDistanceToNextTarget() const;
    double GetDisTanceToPreviousTarget() const;

    
	
    bool ChangedSubRoom();

    void RecordActualPosition();
    double GetDistanceSinceLastRecord();


    /**
     * Compute and update the route.
     * This method should be called at each time step;
     *
     * @return -1 if no route could be found. The ID of the
     * next target is returned otherwise.
     *
     */
    int FindRoute();

    ///write the pedestrian path (room and exit taken ) to file
    void WritePath(std::ofstream& file, Building* building=NULL);

    ///write the pedestrian path (room and exit taken ) to file
    /// in the format room1:exit1>room2:exit2
    std::string GetPath();

    //debug
    void Dump(int ID, int pa = 0); // dump pedestrians parameter, 0 for all parameters

    /**
     * observe the reference pedestrians and collect some data, e.g distance to exit
     * and average/maximal velocities
     *
     * @param maxObservationTime in sec.
     * @return false, if the observation time is over and the observation data can be retrieved
     */
    bool Observe(double maxObservationTime=-1);

    /**
     * @return true, if reference pedestrian have been selected and the observation process has started
     */
    bool IsObserving();

    /**
     * return the observation data in an array
     *
     * @param exitID, the concerned exit
     * @param data, a float array to store the data
     */
    void GetObservationData(int exitID, float* data);

    /**
     * @return true if the time spent in jam exceed the given threshold
     */
    bool IsFeelingLikeInJam();

    void ResetTimeInJam();
    void UpdateTimeInJam();
    void UpdateJamData();
    void UpdateReroutingTime();
    void RerouteIn(double time);
    bool IsReadyForRerouting();

    /**
     * clear the parameter related to the re routing
     */
    void ResetRerouting();


    //void SetMaxOberservationTime(double time);
    double GetAge() const;
    void SetAge(double age);
    std::string GetGender() const;
    void SetGender(std::string gender);
    double GetHeight() const;
    void SetHeight(double height);
    int GetGroup() const;
    void SetGroup(int group);

    //
    static double GetGlobalTime();
    static void SetGlobalTime(double time);
};

#endif	/* _PEDESTRIAN_H */

