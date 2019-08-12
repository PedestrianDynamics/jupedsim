#include "cognitivemap.h"

#include "pedestrian/Pedestrian.h"
#include "router/ai_router/perception/visualsystem.h"

#include <chrono>
#include <random>
#include <algorithm>
#include <cmath>
#include <fstream>


AICognitiveMap::AICognitiveMap()
{

}


AICognitiveMap::AICognitiveMap(const Pedestrian *ped, const VisualSystem* perceptionalAbilities)
{
    _ped=ped;

    _perceptAbis=perceptionalAbilities;
    //Destination and regions
    _currentRegion=nullptr;
    //Find maindestination in cogmapstorage

    _nextTarget=nullptr;
    _lastBestChoice=NavLine(Line(Point(0,0),Point(0,0)));
    _lastMinAngleDiff=FLT_MAX;


//    Region newRegion(Point(0.0));
//    newRegion.SetA(20.0);
//    newRegion.SetB(20.0);
//    newRegion.SetId(1);

//    Landmark finalDest(Point(5.0,5.0),0.1,0.1,1);
//    finalDest.SetPosInMap(Point(15.0,8.0));
//    finalDest.SetType("main");

//    newRegion.AddLandmark(finalDest);
//    AddRegion(newRegion);

//    InitLandmarkNetworksInRegions();
//    FindMainDestination();

}


void AICognitiveMap::UpdateMap()
{
    FindCurrentRegion();

    CheckIfLandmarksReached();

}



void AICognitiveMap::AddRegions(const AIRegions &regions)
{
    for (const AIRegion& region:regions)
    {
       _regions.push_back(region);
    }

}

void AICognitiveMap::AddRegion(const AIRegion& region)
{
    _regions.push_back(region);
}

const AIRegion *AICognitiveMap::GetRegionByID(int regionID) const
{
    for (const AIRegion& region:_regions)
    {
        if (region.GetId()==regionID)
        {
            return &region;
        }
    }
    return nullptr;
}


std::vector<const AILandmark *> AICognitiveMap::TriggerAssociations(const std::vector<const AILandmark *> &landmarks)
{
    std::vector<const AILandmark *> associatedlandmarks;
    for (const AILandmark* landmark:landmarks)
    {
        AIAssociations associations = landmark->GetAssociations();
        for (const AIAssociation& association:associations)
        {
            if (association.GetLandmarkAssociation(landmark)!=nullptr)
            {
                associatedlandmarks.push_back(association.GetLandmarkAssociation(landmark));

            }

        }
    }
    return associatedlandmarks;
}


std::vector<const NavLine*> AICognitiveMap::SortConLeastAngle(const AILandmark* landmark, const std::vector<NavLine> &navLines, const NavLine &focus)
{
    std::vector<const NavLine*> sortedNavLines;
    std::vector<const NavLine*> cNavLines;
    NavLine focusTo = focus;
    for (const NavLine& navline:navLines)
    {
        cNavLines.push_back(&navline);
    }



    //check if lastChoice still exists
    bool statusLastChoice=false;
    if (_lastBestChoice.GetPoint1()==Point(0,0) && _lastBestChoice.GetPoint2()==Point(0,0))
    {
        _lastBestChoice=*(cNavLines.front());
        statusLastChoice=true;
        focusTo=_lastBestChoice;
        //return cNavLines;
    }
    else
    {

        for (const NavLine* navline:cNavLines)
        {

            Point pCloser = GetCloserPoint(_ped->GetPos(),navline->GetPoint1(),navline->GetPoint2());
            Point pCloserBestChoice = GetCloserPoint(_ped->GetPos(),_lastBestChoice.GetPoint1(),_lastBestChoice.GetPoint2());

            if ((pCloserBestChoice-pCloser).Norm()<0.125)
            {
                statusLastChoice=true;
                _lastBestChoice=*navline;
                break;
            }
        }
    }

    // check if ped already in landmark (ellipse); if so, random choice *******************
    if (landmark)
    {
    Point vecPedLCenter = _ped->GetPos()-landmark->GetPosInMap();
        double angleVecPedLCenter = std::atan2(vecPedLCenter._y,vecPedLCenter._x);

        Point onEllipse = Point(landmark->GetPosInMap()._x+landmark->GetA()*std::cos(angleVecPedLCenter),landmark->GetPosInMap()._y+landmark->GetB()*std::sin(angleVecPedLCenter));

        if ((onEllipse-landmark->GetPosInMap()).Norm()>vecPedLCenter.Norm())
        {
            landmark=nullptr;
            //std::cout << "Hier" << std::endl;
        }
    }
    //****************************************

    if (!landmark)
    {
        if (statusLastChoice)
        {
            sortedNavLines.push_back(&_lastBestChoice);
            for (const NavLine* navLine:cNavLines)
            {
                if (navLine!=&_lastBestChoice)
                    sortedNavLines.push_back(navLine);
            }
            return sortedNavLines;
        }
        else
        {


            //prefer navLines that are in the agent's current heading direction
            double cMinAngle=M_PI/4.0;
            std::vector<const NavLine*> navLinesBehindMe;
            //const NavLine* navLineInHeadDir=nullptr;
            Point vec1 = focusTo.GetCentre()-_ped->GetPos();
            double heading = std::atan2(vec1._y,vec1._x);
            for (auto it=cNavLines.begin(); it!=cNavLines.end(); ++it)
            {
               Point vec2 = (*it)->GetCentre()-_ped->GetPos();
               double angle = std::fabs(std::atan2(vec2._y,vec2._x)-heading);
               if (angle>M_PI)
                   angle=2*M_PI-angle;
               // if crossing is behind agent put it to the bottom of the list
               if (angle > cMinAngle)
               {
                   //cNavLines.erase(std::find(cNavLines.begin(), cNavLines.end(), navLine));
                   cNavLines.erase(it);
                   navLinesBehindMe.push_back(*it);
                   --it;
               }
            }

            cNavLines.insert(cNavLines.end(),navLinesBehindMe.begin(),navLinesBehindMe.end());
            _lastBestChoice=*(cNavLines.front());

            return cNavLines;

        }
    }





    Point beelineToLandmark = ShortestBeeLine(_ped->GetPos(),landmark);
    double alpha1=std::atan2(beelineToLandmark._y,beelineToLandmark._x);

    double anglediff;
    std::vector<double> vecMinAngleDiff;
    double currentMinAngleDiff;
    const NavLine* bestChoice=nullptr;

    while (cNavLines.size()>0)
    {
        bestChoice=nullptr;
        currentMinAngleDiff=FLT_MAX;
        for (auto it=cNavLines.begin(); it!=cNavLines.end(); ++it)
        {

            double alpha2=std::atan2(((*it)->GetCentre()-_ped->GetPos())._y,((*it)->GetCentre()-_ped->GetPos())._x);

            anglediff=std::fabs(alpha1-alpha2);


            if (anglediff>M_PI)
                anglediff=2*M_PI-anglediff;


            if (anglediff<currentMinAngleDiff)
            {
                currentMinAngleDiff=anglediff;
                bestChoice=*it;

            }

        }

        if (*bestChoice==_lastBestChoice)
        {
            _lastMinAngleDiff=currentMinAngleDiff;
        }

        vecMinAngleDiff.push_back(currentMinAngleDiff);
        sortedNavLines.push_back(bestChoice);
        cNavLines.erase(std::find(cNavLines.begin(), cNavLines.end(), bestChoice));
    }


//    //prefer navLines that are in the agent's current heading direction******************

//    double cMinAngle=M_PI/3.0;
//    std::vector<const NavLine*> navLinesBehindMe;
//    //const NavLine* navLineInHeadDir=nullptr;
//    Point vec1 = focusTo.GetCentre()-_ped->GetPos();
//    double heading = std::atan2(vec1._y,vec1._x);
//    for (auto it=sortedNavLines.begin(); it!=sortedNavLines.end(); ++it)
//    {
//       Point vec2 = (*it)->GetCentre()-_ped->GetPos();
//       double angle = std::fabs(std::atan2(vec2._y,vec2._x)-heading);
//       if (angle>M_PI)
//           angle=2*M_PI-angle;
//       // if crossing is behind agent put it to the bottom of the list
//       if (angle > cMinAngle)
//       {
//           //cNavLines.erase(std::find(cNavLines.begin(), cNavLines.end(), navLine));
//           sortedNavLines.erase(it);
//           navLinesBehindMe.push_back(*it);
//           --it;
//       }
//    }

//    sortedNavLines.insert(sortedNavLines.end(),navLinesBehindMe.begin(),navLinesBehindMe.end());
//    _lastBestChoice=*(cNavLines.front());

    //*********************************

    if (_lastMinAngleDiff<vecMinAngleDiff.front()+M_PI/3.0 && statusLastChoice)
    {
        //erase the best choice navline from the vector and put it on the front
        for (auto it=sortedNavLines.begin(); it!=sortedNavLines.end(); ++it)
        {
            if (*(*it)==_lastBestChoice)
            {
                sortedNavLines.erase(it);
                break;
            }
        }
        sortedNavLines.insert(sortedNavLines.begin(),&_lastBestChoice);
    }
    else
    {

        _lastBestChoice=*(sortedNavLines.front());
        _lastMinAngleDiff=vecMinAngleDiff.front();

    }


    return sortedNavLines;

}



Point AICognitiveMap::ShortestBeeLine(const Point &pos, const AILandmark *landmark)
{
    Point pointOnShortestRoute = landmark->PointOnShortestRoute(pos);
    Point beeLineVec = pointOnShortestRoute-pos;
    return beeLineVec;
}


double AICognitiveMap::MakeItFuzzy(double mean, double std)
{
    using myClock = std::chrono::high_resolution_clock;
    myClock::duration d = myClock::now().time_since_epoch();

    auto seed = d.count();

    std::default_random_engine generator(seed);
    std::normal_distribution<double> distribution(mean,std);

    double number = distribution(generator);

    return number;
}



std::vector<const AILandmark*> AICognitiveMap::GetLandmarksConnectedWith(const AILandmark* landmark) const
{
    const AIRegion* cRegion = GetRegionContaining(landmark);

    if (cRegion!=nullptr)
    {
        return cRegion->ConnectedWith(landmark);
    }
    else
    {
        return std::vector<const AILandmark*>();
    }
}

const AIRegion* AICognitiveMap::GetRegionContaining(const AILandmark *landmark) const
{
    for (const AIRegion& region:_regions)
    {
        if (region.ContainsLandmark(landmark))
            return &region;
    }
    return nullptr;
}

void AICognitiveMap::FindCurrentRegion()
{

    //for test purposes. has to be changed
    if (_regions.empty())
        return;

    //needs to be fixed
    _currentRegion=&(_regions.back());
    return;

    for (const AIRegion& region:_regions)
    {
        if (region.Contains(_ped->GetPos()))
        {
            _currentRegion=&region;
            return;
        }
    }
    //path searching to region
    _currentRegion=nullptr;
}

void AICognitiveMap::CheckIfLandmarksReached()
{

    BoostPolygon boostPolygon=_perceptAbis->GetCurrentEnvironment();

    if (_currentRegion!=nullptr)
    {
        for (const AILandmark& landmark:_currentRegion->GetLandmarks())
        {
		if (boost::geometry::intersects(landmark.GetRealPos(),boostPolygon))
		{
		    if (std::find(_landmarksRecentlyVisited.begin(), _landmarksRecentlyVisited.end(), &landmark) == _landmarksRecentlyVisited.end() )
		    {
			_landmarksRecentlyVisited.push_back(&landmark);

			_mainDestination=nullptr;
			_nextTarget=nullptr;
		    }
		}

        }
    }


}

const AILandmark *AICognitiveMap::FindConnectionPoint(const AIRegion *currentRegion, const AIRegion *targetRegion) const
{
    if (currentRegion!=nullptr && targetRegion!=nullptr)
    {
        for (const AILandmark& landmarka:currentRegion->GetLandmarks())
        {
            for (const AILandmark& landmarkb:targetRegion->GetLandmarks())
            {
                if (&landmarka==&landmarkb)
                {
                    return &landmarka;
                }
            }
        }
    }

    return nullptr;

}

void AICognitiveMap::FindMainDestination()
{
    std::vector<const AILandmark*> possibleMainTargets;
    for (const AIRegion& region:_regions)
    {
        for (const AILandmark& landmark:region.GetLandmarks())
        {
            if (landmark.GetType()=="main" && std::find(_landmarksRecentlyVisited.begin(),_landmarksRecentlyVisited.end(),&landmark)==_landmarksRecentlyVisited.end())
            {
                possibleMainTargets.push_back(&landmark);
                _targetRegion=&region;

            }
        }
    }
    if (possibleMainTargets.empty())
    {
        _mainDestination=nullptr;
        _targetRegion=nullptr;
        return;
    }
    else
        _mainDestination=GetNearestMainTarget(possibleMainTargets);

}

void AICognitiveMap::FindNextTarget()
{

    _nextTarget=nullptr;

    // if not already in the region of the maindestination
    if (_currentRegion==nullptr || _targetRegion==nullptr)
    {
        return;
    }

    if (_targetRegion!=_currentRegion)
    {
        _nextTarget=FindConnectionPoint(_currentRegion,_targetRegion);
        // if connection point does not exist: Path searching to region
        if (_nextTarget==nullptr)
        {
            //Region is target
            _nextTarget=_targetRegion->GetRegionAsLandmark();
            return;
        }

    }
    else //destination is in current region
    {
       _nextTarget=_mainDestination;
    }

    // Function considers that nearLandmark can be the target itself if no nearer was found.
    const AILandmark* nearLandmark = FindNearLandmarkConnectedToTarget(_nextTarget);

    _nextTarget=nearLandmark;
    //Direct way to target much shorter than via near Landmark?

}

void AICognitiveMap::FindShortCut()
{

}

const AILandmark* AICognitiveMap::FindNearLandmarkConnectedToTarget(const AILandmark* target)
{

    std::vector<const AILandmark*> landmarksConnectedToTarget = FindLandmarksConnectedToTarget(target);

    //if target has no connections return nullptr
    if (landmarksConnectedToTarget.empty())
        return target;

    //look for nearest located landmark

    //look for landmarks within a circle with the radius searchlimit
    // if no landmarks were found radius will be enlarged
    // if radius = distance(Pos->target) return target

    Point vector=target->GetPosInMap()-_ped->GetPos();//_YAHPointer.GetPos();


    double distanceToTarget=vector.Norm();
    int divisor = 24;
    double searchlimit=distanceToTarget/divisor;
    std::vector<const AILandmark*> nearLandmarks;

    while (searchlimit<distanceToTarget && nearLandmarks.empty())
    {
        for (const AILandmark* landmark:landmarksConnectedToTarget)
        {

            vector=landmark->GetPosInMap()-_ped->GetPos();

            double distance = vector.Norm();


            if (distance<=searchlimit)
            {
                nearLandmarks.push_back(landmark);
            }
        }
        searchlimit+=searchlimit;

    }

    if (nearLandmarks.empty())
        return target;

    // select best route to target from one of the nearLandmarks

    return FindBestRouteFromOneOf(nearLandmarks);


}

std::vector<const AILandmark*> AICognitiveMap::FindLandmarksConnectedToTarget(const AILandmark* target)
{

    std::vector<const AILandmark*> connectedLandmarks;

    // landmarks directly connected to target
    std::vector<const AILandmark*> firstCandidates = GetLandmarksConnectedWith(target);

    for (const AILandmark* candidate:firstCandidates)
    {
        if(std::find(_landmarksRecentlyVisited.begin(), _landmarksRecentlyVisited.end(), candidate) == _landmarksRecentlyVisited.end())
        {
            connectedLandmarks.push_back(candidate);
	}
    }

    //Landmarks connected to landmarks connected to target
    std::vector<const AILandmark*> furtherCandidates;

    for (size_t i=0; i<connectedLandmarks.size(); ++i)
    {
        furtherCandidates=GetLandmarksConnectedWith(connectedLandmarks[i]);

        for (const AILandmark* candidate : furtherCandidates)
        {
            // if candidate not already taken into account, not visited before or target itself
            if(std::find(connectedLandmarks.begin(), connectedLandmarks.end(), candidate) == connectedLandmarks.end()
                    && candidate!=target)
            {
                connectedLandmarks.push_back(candidate);

            }
        }
    }
    return connectedLandmarks;
}

const AILandmark* AICognitiveMap::FindBestRouteFromOneOf(const std::vector<const AILandmark*> &nearLandmarks)
{
    const AILandmark* bestChoice = nullptr;
    double minDistance = FLT_MAX;
    double cDistance;
    for (const AILandmark* landmark:nearLandmarks)
    {
        cDistance=(_ped->GetPos()-landmark->GetRandomPoint()).Norm()+_currentRegion->PathLengthFromLandmarkToTarget(landmark, _nextTarget).second;

        if (cDistance<minDistance)
        {
            minDistance=cDistance;

            bestChoice=landmark;
        }
    }
    std::vector<const AILandmark*> landmarksOnShortestPath = _currentRegion->PathLengthFromLandmarkToTarget(bestChoice, _nextTarget).first;

    for (int i=landmarksOnShortestPath.size()-1; i>=0; --i)
    {
        if (std::find(_landmarksRecentlyVisited.begin(),_landmarksRecentlyVisited.end(),landmarksOnShortestPath[i])==_landmarksRecentlyVisited.end())
        {
            return landmarksOnShortestPath[i];
        }

    }
    return nullptr;
}

const AILandmark* AICognitiveMap::GetNearestMainTarget(const std::vector<const AILandmark*> &mainTargets)
{
    const AILandmark* nearest = nullptr;
    double dNearest = FLT_MAX;

    if (mainTargets.size()==1)
        return mainTargets[0];

    for (const AILandmark* mainDest:mainTargets)
    {
        _nextTarget=mainDest;
        const AILandmark* cLandmark = FindNearLandmarkConnectedToTarget(mainDest);
        double cDistance=(_ped->GetPos()-cLandmark->GetRandomPoint()).Norm()+_currentRegion->PathLengthFromLandmarkToTarget(cLandmark,mainDest).second;

        if (cDistance<dNearest)
        {
            dNearest=cDistance;
            nearest=mainDest;
        }
    }

    return nearest;
}

void AICognitiveMap::SetBestChoice(const NavLine &navLine)
{
    _lastBestChoice=navLine;
}

const NavLine* AICognitiveMap::GetBestChoice() const
{
    if (_lastBestChoice.GetPoint1()==Point(0,0) && _lastBestChoice.GetPoint2()==Point(0,0))
        return nullptr;
    else
        return &_lastBestChoice;
}

void AICognitiveMap::InitLandmarkNetworksInRegions()
{
    for (AIRegion& region:_regions)
    {
        region.InitLandmarkNetwork();
    }
}

const AILandmark* AICognitiveMap::GetNextTarget() const
{
    return _nextTarget;
}


Point GetCloserPoint(const Point &origin, const Point &target1, const Point& target2)
{

    Point vec1 = target1-origin;
    Point vec2 = target2-origin;

    if (vec1.Norm()<vec2.Norm())
        return target1;
    else
        return target2;
}