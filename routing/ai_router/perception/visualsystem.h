#pragma once

#include "geometry/Point.h"
#include "general/Macros.h"

#include <boost/geometry/geometry.hpp>

#include <memory>
#include <vector>
#include <unordered_map>


class NavLine;
class Pedestrian;
class Building;
class VisibleEnvironment;
class Transition;
class Room;
class SubRoom;
class Sign;

using ptrNavLine = std::unique_ptr<const NavLine>;
using ptrFloor = const Room*;
typedef boost::geometry::model::linestring<Point> Linestring;
typedef boost::geometry::model::polygon<Point> BoostPolygon;


class VisualSystem
{
public:
    VisualSystem();
    VisualSystem(const Building* b, const Pedestrian* ped, const VisibleEnvironment* env);

    const std::unordered_map<ptrFloor,BoostPolygon>& UpdateSeenEnv();
    const BoostPolygon& UpdateCurrentEnvironment();
    std::vector<NavLine> GetPossibleNavLines(const std::vector<Point> &envPolygon) const;

    const std::unordered_map<ptrFloor,BoostPolygon>& GetSeenEnvironment() const;
    const BoostPolygon& GetCurrentEnvironment() const;

    std::vector<Point> GetSeenEnvironmentAsPointVec(const ptrFloor& room) const;
    std::vector<Point> GetCurrentEnvironmentAsPointVec() const;

    //Transitions in currentEnv?
    std::vector<NavLine> FindTransitions() const;
    const SubRoom* IsInSubRoom(const Point& point) const;
    const Room *IsInRoom(const Point& point) const;
    const Transition* IsTransition(const NavLine* navLine) const;

    //Signage
    void GetVisibleSigns();
    // sign not to far away in consideration of observation angle (see Xie et. al 2007)
    bool SignLegible(const Sign& sign) const;
    //Heading direction of ped compared to direction of beeline to sign
    double HeadDirCompToSign(const Sign& sign) const;
    //double GetAngleFactor(const double& angle) const;
    //calculate probability to detect sign in current time step
    double GetProbabilityToDetectSign(double headDir, bool hardlyVisSign);
    std::vector<const Sign*> TryToDetectSigns();
    bool TryToDetectSign(const Sign& sign, double headDir, bool hardlyVisSign);

    //Herding
    // walking direction of other pedestrians are returned / only an idea! not working!
    std::vector<std::pair<const Point &, double> > PerceiveDirsOfOtherPedestrians() const;

private:

    const Building* _building;
    const Pedestrian* _ped;
    const VisibleEnvironment* _env;
    std::unordered_map<ptrFloor,BoostPolygon> _seenEnvironment;
    BoostPolygon _currentEnv;
    // in vector: sign,vector:time in visibility to sign, spent probability to detect sign,
    //last angle sector to sign, spent probability to detect sign in current angle sector
    std::vector<const Sign*> _visibleSigns;
    std::vector<const Sign*> _newVisibleSigns;
    std::vector<const Sign*> _hardlyVisibleSigns;
    std::vector<const Sign*> _detectedSigns;
    int _signPerceived;
};

double LegibleDistance(double phi);
