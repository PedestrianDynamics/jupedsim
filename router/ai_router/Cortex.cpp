#include "Cortex.h"

#include "pedestrian/Pedestrian.h"

#include <cstdlib>
#include <cmath>

Cortex::Cortex()
{

}

Cortex::Cortex(const Building *b, const Pedestrian *ped, const VisibleEnvironment *env, std::unordered_map<const SubRoom *, ptrIntNetwork> *roominternalNetworks)
{
    _b=b;
    _ped=ped;
    _wholeEnvironment=env;
    _intNetworks=roominternalNetworks;

    _perceptionAbilities=VisualSystem(_b,_ped,_wholeEnvironment);
    _cMap=AICognitiveMap(_ped,&_perceptionAbilities);
    _lastBestVisNavLine=NavLine(Line(Point(0,0),Point(0,0)));

    if (_b->GetConfig()->GetRandomNumberGenerator()->GetRandomRealBetween0and1()<=0.11)
        _statPreferToGoDown=true;
    else
        _statPreferToGoDown=false;


}

AICognitiveMap &Cortex::GetCognitiveMap()
{
    return _cMap;
}

VisualSystem &Cortex::GetPerceptionAbilities()
{
    return _perceptionAbilities;
}

const NavLine* Cortex::GetNextNavLine(const NavLine* nextTarget)
{
    const SubRoom* currentSubRoom = _b->GetSubRoomByUID(_ped->GetSubRoomUID());

    _currentIntNetwork=*(_intNetworks->operator [](currentSubRoom));

    return _currentIntNetwork.GetNextNavLineOnShortestPathToTarget(_ped->GetPos(),nextTarget);
}

bool Cortex::HlineReached() const
{
    const SubRoom* currentSubRoom = _b->GetSubRoomByUID(_ped->GetSubRoomUID());

    for (Hline* hline:currentSubRoom->GetAllHlines())
    {
        // if distance hline to ped lower than 1 m
        if (hline->DistTo(_ped->GetPos())<1)
        {
            return true;
        }
    }

    return false;
}


std::vector<const NavLine*> Cortex::SortConGeneralKnow(const std::vector<const NavLine*> &navLines)
{

    //std::cout << dis(_randomEngine) << std::endl;

    std::vector<const NavLine*> sortedNavLines;
    std::vector<const NavLine*> sortedNavLinesOutside;
    std::vector<const NavLine*> sortedNavLinesLobbies;
    std::vector<const NavLine*> sortedNavLinesCirculs;
    std::vector<const NavLine*> sortedNavLinesCommons;
    std::vector<const NavLine*> sortedNavLinesStairsUp;
    std::vector<const NavLine*> sortedNavLinesStairs;
    std::vector<const NavLine*> sortedNavLinesStairsDown;
    std::vector<const NavLine*> sortedNavLinesTrains;

    for (const NavLine* navLine:navLines)
    {

        // navLine a transition?
        const Transition* transition = _perceptionAbilities.IsTransition(navLine);

        if (!transition)
        {

            if (_ped->GetRoomID()==11)
            {
                if (IsInsideRectangle(navLine->GetCentre(),Point(7.52,52.19),Point(11.31,46.0))
                                    || IsInsideRectangle(navLine->GetCentre(),Point(20.25,68.38),Point(27.2,64.91))
                                        || IsInsideRectangle(navLine->GetCentre(),Point(4.48,79.5),Point(13.78,73.14))
                                               || IsInsideRectangle(navLine->GetCentre(),Point(-7.06,54.69),Point(-1.48,50.48)))
                {
                    sortedNavLinesStairsDown.push_back(navLine);
                    continue;
                }
            }


            // is line on same floor? If yes: subRoom!=nullptr
            const SubRoom* subRoom=_perceptionAbilities.IsInSubRoom(navLine->GetCentre());
            if (!subRoom)
                sortedNavLinesStairsDown.push_back(navLine);
            else if (subRoom->GetType()=="Lobby")
                sortedNavLinesLobbies.push_back(navLine);
            else if (subRoom->GetType()=="Corridor")
                sortedNavLinesCirculs.push_back(navLine);
            else if (subRoom->GetType()=="train")
                sortedNavLinesTrains.push_back(navLine);
            else
                sortedNavLinesCommons.push_back(navLine);
        }
        else if (transition->IsExit())
        {
            sortedNavLinesOutside.push_back(transition);
        }

        else
        {
            if (transition->GetOtherRoom(_ped->GetRoomID())->GetZPos()>_b->GetRoom(_ped->GetRoomID())->GetZPos())
            {
                sortedNavLinesStairsUp.push_back(transition);
            }

            else
            {
                sortedNavLinesStairsDown.push_back(transition);

            }

        }
    }



    //settings for underground station!
    sortedNavLines.insert(std::end(sortedNavLines), std::begin(sortedNavLinesOutside), std::end(sortedNavLinesOutside));
    sortedNavLines.insert(std::end(sortedNavLines), std::begin(sortedNavLinesLobbies), std::end(sortedNavLinesLobbies));
    sortedNavLines.insert(std::end(sortedNavLines), std::begin(sortedNavLinesStairsUp), std::end(sortedNavLinesStairsUp));
    sortedNavLines.insert(std::end(sortedNavLines), std::begin(sortedNavLinesStairs), std::end(sortedNavLinesStairs));
    sortedNavLines.insert(std::end(sortedNavLines), std::begin(sortedNavLinesCirculs), std::end(sortedNavLinesCirculs));
    sortedNavLines.insert(std::end(sortedNavLines), std::begin(sortedNavLinesCommons), std::end(sortedNavLinesCommons));
    sortedNavLines.insert(std::end(sortedNavLines), std::begin(sortedNavLinesStairsDown), std::end(sortedNavLinesStairsDown));
    sortedNavLines.insert(std::end(sortedNavLines), std::begin(sortedNavLinesTrains), std::end(sortedNavLinesTrains));


    return sortedNavLines;
}



const NavLine Cortex::FindApprCrossing(const std::vector<NavLine>& navLines)
{

    // suffle navLines randomly -> if no factor influences ranking of navLines a specific navLine is chosen randomly.
    std::vector<NavLine> shuffledNavLines=navLines;

    std::shuffle(shuffledNavLines.begin(),shuffledNavLines.end(),_b->GetConfig()->GetRandomNumberGenerator()->GetRandomEngine());

    // knowledge generalized <- signage  <- Cognitive Map <- Random shuffle
    const NavLine* nextNvLine=SortConGeneralKnow(SortConSignage(_cMap.SortConLeastAngle(_cMap.GetNextTarget(),shuffledNavLines,_lastBestVisNavLine))).front();

    _cMap.SetBestChoice(*nextNvLine);
    if (!nextNvLine)
      throw(std::invalid_argument("No NavLine found. Ped is trapped"));
    else
        return *nextNvLine;

}

const NavLine Cortex::FindApprVisibleCrossing(const NavLine& navLine, const std::vector<NavLine> &navLines)
{
    // if transition or outside is in sight
    Point normal = Point(-(navLine.GetPoint2()-navLine.GetPoint1())._y,(navLine.GetPoint2()-navLine.GetPoint1())._x);
    normal=normal/normal.Norm();
    Point dirVec1 = navLine.GetCentre()+normal*0.5-_ped->GetPos();
    Point dirVec2 = navLine.GetCentre()-normal*0.5-_ped->GetPos();

    std::vector<int> transIDs= _b->GetRoom(_ped->GetRoomID())->GetAllTransitionsIDs();

    for (int transID: transIDs)
    {
        if (*(_b->GetTransitionByUID(transID))==navLine)
        {
            Line targetToHead(Point(0.0,0.0),Point(0.0,0.0));
            if (dirVec1.Norm() > dirVec2.Norm())
                targetToHead=Line(navLine.GetPoint1()+normal*0.5,navLine.GetPoint2()+normal*0.5);
            else
                targetToHead=Line(navLine.GetPoint1()-normal*0.5,navLine.GetPoint2()-normal*0.5);

            const NavLine visibleNavLine(targetToHead);
            return visibleNavLine;
        }
    }

    // not a transition
    if (_lastBestVisNavLine.GetPoint1()==Point(0,0) && _lastBestVisNavLine.GetPoint2()==Point(0,0))
    {
        double minDistance=FLT_MAX;
        for (const NavLine& candidate:navLines)
        {
            if (candidate.DistTo(navLine.GetCentre())<=minDistance)
            {
                minDistance=candidate.DistTo(navLine.GetCentre());
                _lastBestVisNavLine=candidate;
            }
        }
        return TargetToHead(&_lastBestVisNavLine);
    }

    bool statFoundBestChoice=false;

    for (const NavLine& candidate:navLines)
    {
        Point vec1=candidate.GetPoint1()-_lastBestVisNavLine.GetPoint1();
        Point vec2=candidate.GetPoint2()-_lastBestVisNavLine.GetPoint2();
        Point vec3=candidate.GetPoint2()-_lastBestVisNavLine.GetPoint1();
        Point vec4=candidate.GetPoint1()-_lastBestVisNavLine.GetPoint2();
        if (vec1.Norm()<0.1 || vec2.Norm()<0.1 || vec3.Norm()<0.1 || vec4.Norm()<0.1)
        {
            _lastBestVisNavLine=candidate;
            statFoundBestChoice=true;
        }
    }

    double pedAlpha = std::atan2(_ped->GetV()._x,_ped->GetV()._y);
    bool statFound=false;
    if (statFoundBestChoice)
    {


        double minDistance=_lastBestVisNavLine.DistTo(navLine.GetCentre());
        for (const NavLine& candidate:navLines)
        {
            Point vecPedNavLine = candidate.GetCentre()-_ped->GetPos();
            double navAlpha=std::atan2(vecPedNavLine._x,vecPedNavLine._y);
            double diffAlpha=std::fabs(navAlpha-pedAlpha);
            if (diffAlpha>M_PI)
                diffAlpha=2*M_PI-diffAlpha;
            if (diffAlpha<M_PI/3.0 && candidate.DistTo(navLine.GetCentre())<=minDistance*0.8)
            {
                minDistance=candidate.DistTo(navLine.GetCentre());
                _lastBestVisNavLine=candidate;
                statFound=true;
            }
        }


        if (!statFound)
        {
            for (const NavLine& candidate:navLines)
            {
                if (candidate.DistTo(navLine.GetCentre())<=minDistance*0.2)
                {
                    minDistance=candidate.DistTo(navLine.GetCentre());
                    _lastBestVisNavLine=candidate;
                }
            }
        }
    }
    else
    {
        double minDistance=FLT_MAX;
        for (const NavLine& candidate:navLines)
        {
            Point vecPedNavLine = candidate.GetCentre()-_ped->GetPos();
            double navAlpha=std::atan2(vecPedNavLine._x,vecPedNavLine._y);
            //Log->Write(std::to_string(std::fabs(navAlpha-pedAlpha)*180/M_PI));
            double diffAlpha=std::fabs(navAlpha-pedAlpha);
            if (diffAlpha>M_PI)
                diffAlpha=2*M_PI-diffAlpha;
            if (diffAlpha<M_PI/3.0 && candidate.DistTo(navLine.GetCentre())<=minDistance)
            {
                minDistance=candidate.DistTo(navLine.GetCentre());
                _lastBestVisNavLine=candidate;
                statFound=true;
            }
        }

        if (!statFound)
        {
            for (const NavLine& candidate:navLines)
            {
                if (candidate.DistTo(navLine.GetCentre())<=minDistance)
                {
                    minDistance=candidate.DistTo(navLine.GetCentre());
                    _lastBestVisNavLine=candidate;
                }
            }
        }
    }


    return TargetToHead(&_lastBestVisNavLine);
}

const NavLine Cortex::TargetToHead(const NavLine* visibleCrossing) const
{
    Point P1 = visibleCrossing->GetPoint1();
    Point P2 = visibleCrossing->GetPoint2();
    Point dirVector=P2-P1;
    dirVector=dirVector/dirVector.Norm();
    Point normalVector = Point(-dirVector._y,dirVector._x);

    Point distVec1=P1-_ped->GetPos();
    Point distVec2=P2-_ped->GetPos();

    Point NavLineP1;
    Point NavLineP2;
    if (distVec1.Norm()<distVec2.Norm())
    {
        NavLineP1=P1;
        NavLineP2=P1-normalVector*1.5;
    }
    else
    {
        NavLineP1=P2;
        NavLineP2=P2-normalVector*1.5;
    }

    const NavLine navLine(Line(NavLineP1,NavLineP2));

    return navLine;
}


std::vector<const NavLine *> Cortex::SortConSignage(const std::vector<const NavLine *> &navLines)
{

    std::vector<const Sign*> perceivedSigns = _perceptionAbilities.TryToDetectSigns();
    std::vector<const Sign*> cSigns;

    for (const Sign* sign:perceivedSigns)
    {
        if (std::find(_notUsedSigns.begin(),_notUsedSigns.end(),sign)==_notUsedSigns.end())
            cSigns.push_back(sign);
    }

    if (cSigns.empty())
        return navLines;


    std::vector<const NavLine*> sortedNavLines;
    //determine decisive sign (sign which has the recent instruction)

    const Sign* decisiveSign=nullptr;
    if (cSigns.size()>1)
        decisiveSign=DetermineDecisiveSign(cSigns);
    else
        decisiveSign=cSigns.front();

    if (!decisiveSign)
        return navLines;

    Point vecToNavLine;
    double angleVecToNavLine;
    double anglediff;

    // find navLine which is most appropriately to follow sign instruction

    for (const NavLine* navLine:navLines)
    {

        vecToNavLine = navLine->GetCentre()-decisiveSign->GetPos();
        angleVecToNavLine = std::atan2(vecToNavLine._y,vecToNavLine._x)*180.0/M_PI;

        anglediff=std::fabs(angleVecToNavLine-decisiveSign->GetAlphaPointing());
        if (anglediff>180.0)
            anglediff=360.0-anglediff;


        if (anglediff<90.0)
        {
            sortedNavLines.push_back(navLine);
        }
    }

    if (sortedNavLines.empty())
        return navLines;

    const NavLine* bestChoice=nullptr;

    if (sortedNavLines.size()>1)
    {
        std::vector<double> orthogonalDistances;
        for (const NavLine* navLine:sortedNavLines)
        {
            orthogonalDistances.push_back(OrthogonalDistanceFromTo(navLine,*decisiveSign));
        }
        size_t index=std::min_element(orthogonalDistances.begin(),orthogonalDistances.end())-orthogonalDistances.begin();

        bestChoice = *(sortedNavLines.begin()+index);
        sortedNavLines.clear();
        sortedNavLines.push_back(bestChoice);
    }
    else
        bestChoice=sortedNavLines.front();




    for (const NavLine* navLine:navLines)
    {
        if (navLine!=bestChoice)
        {
            sortedNavLines.push_back(navLine);
        }
    }

    return sortedNavLines;
}

const Sign* Cortex::DetermineDecisiveSign(const std::vector<const Sign*> &signs)
{

    std::vector<const Sign*> cSigns=signs;

    if (cSigns.size()==1)
        return cSigns.front();


    std::vector<double> angles;
    std::vector<const Sign*> signsNotPointing;

    for (size_t i=0; i<cSigns.size(); ++i)
    {
       std::vector<double> cAngles;

       for (size_t j=0; j<cSigns.size(); ++j)
       {
           if (i!=j)
           {
               Point vec_ij=cSigns[j]->GetPos()-cSigns[i]->GetPos();
               cAngles.push_back(std::fabs(cSigns[i]->GetAlphaPointing()-std::atan2(vec_ij._y,vec_ij._x)*180.0/M_PI));
           }
       }
       size_t index=std::min_element(cAngles.begin(),cAngles.end())-cAngles.begin();

       if (cAngles.at(index)>45.0)
           signsNotPointing.push_back(cSigns[i]);
       angles.push_back(cAngles.at(index));
    }

    //how many angles are greater than 45 degrees or rather how many signs do not point to another?
    if (signsNotPointing.size()>1)
    {
        std::shuffle(signsNotPointing.begin(),signsNotPointing.end(),_b->GetConfig()->GetRandomNumberGenerator()->GetRandomEngine());
        for (auto it=signsNotPointing.begin();it!=signsNotPointing.end();++it)
        {
            if (it==signsNotPointing.begin())
                continue;
            _notUsedSigns.push_back(*it);
        }
        return signsNotPointing.front();
    }
    else
    {

        size_t index=std::max_element(angles.begin(),angles.end())-angles.begin();
        return cSigns.at(index);
    }

}

double Cortex::OrthogonalDistanceFromTo(const Sign &sign1, const Sign &sign2) const
{
    // line created with dir Vector of sign2

    double a;
    double b;

    if (sign2.GetAlpha()==90.0 || sign2.GetAlpha()==270.0)
    {
        a=0.0;
        b=1.0;
    }
    else
    {
        a=1.0;
        b = std::tan(sign2.GetAlpha()*M_PI/180.);
    }

    Point pa = sign2.GetPos();
    Point dirVec = Point(a,b);

    Line signLine = Line(pa-dirVec*1000,pa+dirVec*1000);

    return signLine.DistTo(sign1.GetPos());
}

double Cortex::OrthogonalDistanceFromTo(const NavLine* navLine, const Sign& sign) const
{
    // line created with dir Vector of sign

    double a;
    double b;
    //sign1->GetAlpha();
    if (sign.GetAlphaPointing()==90.0 || sign.GetAlphaPointing()==270.0)
    {
        a=0.0;
        b=1.0;
    }
    else
    {
        a=1.0;
        b = std::tan(sign.GetAlphaPointing()*M_PI/180.);
    }

    Point pa = sign.GetPos();
    Point dirVec = Point(a,b);

    Line signLine = Line(pa-dirVec*1000,pa+dirVec*1000);

    return signLine.DistTo(navLine->GetCentre());
}

double Cortex::OrthogonalDistanceFromTo(const NavLine* navLine, const Point &pos, const double &angle) const
{
    // line created with dir Vector of sign

    double a;
    double b;

    if (angle==90.0 || angle==270.0)
    {
        a=0.0;
        b=1.0;
    }
    else
    {
        a=1.0;
        b = std::tan(angle*M_PI/180.);
    }

    Point dirVec = Point(a,b);

    Line line = Line(pos-dirVec*1000,pos+dirVec*1000);

    return line.DistTo(navLine->GetCentre());
}



bool IsInsideRectangle(const Point &point, const Point &leftUp, const Point &rightDown)
{
    if (point._x> leftUp._x && point._x< rightDown._x
            && point._y>rightDown._y && point._y<leftUp._y)
        return true;
    else
        return false;
}
