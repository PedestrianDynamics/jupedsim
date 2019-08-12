#include "visualsystem.h"

#include "visibleenvironment.h"
#include "geometry/SubRoom.h"
#include "pedestrian/Pedestrian.h"

#include <chrono>
#include <iostream>

VisualSystem::VisualSystem()
{

}

VisualSystem::VisualSystem(const Building *b, const Pedestrian *ped, const VisibleEnvironment *env)
{
    _building=b;
    _ped=ped;
    _env=env;
    _signPerceived=0;

}

const std::unordered_map<ptrFloor, BoostPolygon> &VisualSystem::UpdateSeenEnv()
{


    //SubRoom * sub_room = _building->GetRoom(_ped->GetRoomID())->GetSubRoom(_ped->GetSubRoomID());


    //BoostPolygon roomP;

//    for (Point point:sub_room->GetPolygon())
//        boost::geometry::append(roomP,point);

//    std::vector<BoostPolygon> intersectPolygon;
//    boost::geometry::intersection(roomP,newPolygon,intersectPolygon);


    //BoostPolygon currentEnv = GetCurrentEnvironment();



    //limiting seen Environment
    BoostPolygon rectangle;

    boost::geometry::append(rectangle,Point(_ped->GetPos()._x-35,_ped->GetPos()._y-35));
    boost::geometry::append(rectangle,Point(_ped->GetPos()._x-35,_ped->GetPos()._y+35));
    boost::geometry::append(rectangle,Point(_ped->GetPos()._x+35,_ped->GetPos()._y+35));
    boost::geometry::append(rectangle,Point(_ped->GetPos()._x+35,_ped->GetPos()._y-35));


    try {
        std::vector<BoostPolygon> output2;
        boost::geometry::intersection(_seenEnvironment[_building->GetRoom(_ped->GetRoomID())],rectangle,output2);
        if (!output2.empty())
            _seenEnvironment.at(_building->GetRoom(_ped->GetRoomID()))=output2.front();


    //union of current and seen Env

        std::vector<BoostPolygon> output;
        boost::geometry::union_(_seenEnvironment[_building->GetRoom(_ped->GetRoomID())],_currentEnv,output);
        if (!output.empty())
            _seenEnvironment.at(_building->GetRoom(_ped->GetRoomID()))=output.front();

        boost::geometry::correct(_seenEnvironment.at(_building->GetRoom(_ped->GetRoomID())));

    }
    catch (...)
    {
       // Code that handles another exception type
       // ...
       //cerr << e.what();
        _seenEnvironment.at(_building->GetRoom(_ped->GetRoomID())) = _currentEnv;
    }

    std::ofstream myfile2;
    std::string str2 = "./isovists/seen_env_"+std::to_string(_ped->GetID())+"_"+std::to_string((int)(std::round(_ped->GetGlobalTime()*100)))+".txt";
    myfile2.open (str2);
    for(auto it1 = boost::begin(boost::geometry::exterior_ring(_seenEnvironment.at(_building->GetRoom(_ped->GetRoomID()))));
            it1 != boost::end(boost::geometry::exterior_ring(_seenEnvironment.at(_building->GetRoom(_ped->GetRoomID()))));
            ++it1)
    {
        myfile2 << it1->_x << " " << it1->_y << std::endl;
        //seenEnvPolygon.push_back(*it1);

    }
    myfile2.close();

    std::ofstream myfile3;
    std::string str3 = "./isovists/real_pos"+std::to_string(_ped->GetID())+"_"+std::to_string((int)(std::round(_ped->GetGlobalTime()*100)))+".txt";
    myfile3.open (str3);
    myfile3 << std::to_string(_ped->GetPos()._x) << " " << std::to_string(_ped->GetPos()._y) << std::endl;
    myfile3.close();


    return _seenEnvironment;

}

const BoostPolygon &VisualSystem::UpdateCurrentEnvironment()
{
//    VisiLibity::Visibility_Polygon
//    my_visibility_polygon(VisiLibity::Point(_ped->GetPos()._x, _ped->GetPos()._y), (*_env->GetVisEnv()),0.01);
//   //std::cout << "The visibility polygon is \n" << my_visibility_polygon << std::endl;



    // find the face of the query point
    // (usually you may know that by other means)
    Point_2 q(_ped->GetPos()._x,_ped->GetPos()._y);
    Arrangement_2::Face_const_handle *face;
    //only visEnv of actual room
    const Arrangement_2& arr = _env->GetCGALGeometry().GetVisEnvCGAL(_building->GetRoom(_ped->GetRoomID()));
    CGAL::Arr_naive_point_location<Arrangement_2> pl(arr);
    CGAL::Arr_point_location_result<Arrangement_2>::Type obj = pl.locate(q);
    // The query point locates in the interior of a face
    face = boost::get<Arrangement_2::Face_const_handle> (&obj);


    //visibility query
    Arrangement_2 output_arr;
    Arrangement_2::Face_handle fh = _env->GetCGALGeometry().GetTEV(_building->GetRoom(_ped->GetRoomID()))->compute_visibility(q,*face,output_arr);//query_point, he, output_arr);

    //print out the visibility region.
//    std::cout << "Regularized visibility region of q has "
//        << output_arr.number_of_edges()
//        << " edges." << std::endl;

    //std::cout << "Boundary edges of the visibility region:" << std::endl;
    Arrangement_2::Ccb_halfedge_circulator curr = fh->outer_ccb();

    BoostPolygon currentEnv;

    //std::cout << "[" << curr->source()->point() << " -> " << curr->target()->point() << "]" << std::endl;
    boost::geometry::append(currentEnv,Point(CGAL::to_double(curr->source()->point().x()) ,CGAL::to_double(curr->source()->point().y())));
    while (++curr != fh->outer_ccb())
    {
        boost::geometry::append(currentEnv,Point(CGAL::to_double(curr->source()->point().x()) ,CGAL::to_double(curr->source()->point().y())));
        //std::cout << "[" << curr->source()->point() << " -> " << curr->target()->point() << "]" << std::endl;
    }

    //delete face;
//   //std::vector<Point> visPolygon;


//   for (size_t i=0; i<my_visibility_polygon.n(); ++i)
//   {
//       //check for small holes
//       for (size_t j=i+1; j<my_visibility_polygon.n(); ++j)
//       {
//            Point vec = Point(my_visibility_polygon[i].x(),my_visibility_polygon[i].y())-Point(my_visibility_polygon[j].x(),my_visibility_polygon[j].y());
//            if (vec.Norm()<=0.1)
//            {
//                i=j+1;
//            }
//       }
//       if (i==my_visibility_polygon.n())
//           break;

//       else
//       {
//           visPolygon.push_back(Point(my_visibility_polygon[i].x(),my_visibility_polygon[i].y()));
//           boost::geometry::append(currentEnv,visPolygon[i]);
//       }

//   }

    std::ofstream myfile;
    std::string str = "./isovists/current_env_"+std::to_string(_ped->GetID())+"_"+std::to_string((int)(std::round(_ped->GetGlobalTime()*100)))+".txt";
    myfile.open (str);
    for(auto it1 = boost::begin(boost::geometry::exterior_ring(currentEnv));
            it1 != boost::end(boost::geometry::exterior_ring(currentEnv));
            ++it1)
    {
        myfile << it1->_x << " " << it1->_y << std::endl;
        //seenEnvPolygon.push_back(*it1);

    }
    myfile.close();

    boost::geometry::correct(currentEnv);

    _currentEnv=currentEnv;

    //PerceiveSigns();

    return _currentEnv;

}

std::vector<NavLine> VisualSystem::GetPossibleNavLines(const std::vector<Point>& envPolygon) const
{
    //check if transitions are in the visible surroundings
    std::vector<NavLine> navLines=FindTransitions();
    //std::vector<ptrNavLine> navLines;
//    if (!navLines.empty())
//        return navLines;


    std::vector<Point> visPolygon = envPolygon;// UpdateSeenEnv();
    //visPolygon.push_back(visPolygon.front());


    for (size_t i=1; i<visPolygon.size(); ++i)
    {

//    for(auto it1 = boost::begin(boost::geometry::exterior_ring(_seenEnvironment)++);
//        it1 != boost::end(boost::geometry::exterior_ring(_seenEnvironment));
//        ++it1)
    //{
        bool intersect=false;
//        bool intersectsSeenEnv=false;
        Point center = (visPolygon[i-1]+visPolygon[i])/2.0;//

//        std::cout << boost::geometry::distance(center,_seenEnvironment) << std::endl;
        //if (boost::geometry::within(center,_seenEnvironment))
        //{
//            bool status=false;
//            for (ptrNavLine navLine:_oldNavLines)
//            {
//                Linestring old_line;

//                boost::geometry::append(old_line, navLine->GetPoint1());
//                boost::geometry::append(old_line, navLine->GetPoint2());
//                if (boost::geometry::distance(center,navLine->GetCentre())<1.0)
//                {
//                    status=true;
//                }
//            }
//            if (!status)
//                continue;
           // intersectsSeenEnv=true;
        //}
        for (Linestring lineS: _env->GetAllWallsOfEnv(_building->GetRoom(_ped->GetRoomID())))
        {
            //Point vecWall = lineS.at(1)-lineS.at(0);
            //Point vecVis = visPolygon[i]+visPolygon[i-1];
            if (boost::geometry::distance(center,lineS)<=0.15)// && boost::geometry::distance(visPolygon[i],lineS)<=0.15)
            {
               //std::cout << "Distance" << boost::geometry::distance(visPolygon[i-1],lineS) << "  " << boost::geometry::distance(visPolygon[i],lineS) << std::endl;
               intersect=true;
               break;
            }
        }
        // if not wall and in current room (no transition)
        if (intersect==false) //&& (IsInSubRoom(visPolygon[i-1])!=nullptr || IsInSubRoom(visPolygon[i])!=nullptr))
        {
            if (Line(visPolygon[i-1],visPolygon[i]).GetLength()>0.4)
            {

//                Point dirVector=visPolygon[i-1]-visPolygon[i];
//                dirVector=dirVector/dirVector.Norm();
//                Point normalVector = Point(-dirVector._y,dirVector._x);

                //if (!intersectsSeenEnv)
                const NavLine navLine(Line(visPolygon[i-1],visPolygon[i]));
                navLines.push_back(navLine);//Line(center+normalVector*3.0,center)));
                //else
                //    navLinesSecondChoice.push_back(std::make_shared<const NavLine>(Line(center+normalVector*3.0,center)));
//                Log->Write("Center:");
//                Log->Write(std::to_string((center-normalVector*3.0)._x));
//                Log->Write(std::to_string((center-normalVector*3.0)._y));
//                Log->Write(" ");
            }
        }

    }

//    std::cout << navLines.size() << std::endl;
    if (navLines.empty())
    {

        navLines.push_back(NavLine(Line(_ped->GetPos(),_ped->GetPos()+_ped->GetV())));


////        if (navLinesSecondChoice.empty())
////        {
//        Log->Write("ERROR:\t Pedestrian seems to be trapped. No way out^^\n");
//        Log->Write(std::to_string(_ped->GetID()));
//        Log->Write("\n");
//        std::cout << _ped->GetPos()._x << " " << _ped->GetPos()._y << std::endl;
//        for(auto it1 = boost::begin(boost::geometry::exterior_ring(_currentEnv));
//                it1 != boost::end(boost::geometry::exterior_ring(_currentEnv));
//                ++it1)
//        {
//            std::cout << it1->_x << " " << it1->_y << std::endl;
//            //seenEnvPolygon.push_back(*it1);

//        }
        //exit(EXIT_FAILURE);
//            //return navLines;
////        }
////        return navLinesSecondChoice;
    }
   // std::cout << navLines.size() << std::endl;

    //_oldNavLines=navLines;
    return navLines;


}

const std::unordered_map<ptrFloor, BoostPolygon> &VisualSystem::GetSeenEnvironment() const
{
    return _seenEnvironment;
}

const BoostPolygon &VisualSystem::GetCurrentEnvironment() const
{
    return _currentEnv;
}

std::vector<Point> VisualSystem::GetSeenEnvironmentAsPointVec(const ptrFloor& floor) const
{
    std::vector<Point> seenEnvPolygon;
    for(auto it = boost::begin(boost::geometry::exterior_ring(_seenEnvironment.at(floor)));
            it != boost::end(boost::geometry::exterior_ring(_seenEnvironment.at(floor)));
            ++it)
    {
        seenEnvPolygon.push_back(*it);
    }



    for(size_t j=0; j<_seenEnvironment.at(floor).inners().size(); ++j)
    {
        auto innerPolygon = _seenEnvironment.at(floor).inners()[j];
        for (size_t i=0; i<innerPolygon.size(); ++i)
        {
            seenEnvPolygon.push_back(innerPolygon[i]);
        }
    }


    return seenEnvPolygon;
}

std::vector<Point> VisualSystem::GetCurrentEnvironmentAsPointVec() const
{
    std::vector<Point> currentEnvPolygon;
    for(auto it = boost::begin(boost::geometry::exterior_ring(_currentEnv));
            it != boost::end(boost::geometry::exterior_ring(_currentEnv));
            ++it)
    {
        currentEnvPolygon.push_back(*it);

    }

    for(size_t j=0; j<_currentEnv.inners().size(); ++j)
    {
        auto innerPolygon = _currentEnv.inners()[j];
        for (size_t i=0; i<innerPolygon.size(); ++i)
        {
            currentEnvPolygon.push_back(innerPolygon[i]);
        }
    }

    return currentEnvPolygon;
}

std::vector<NavLine> VisualSystem::FindTransitions() const
{

    std::vector<Transition*> transitions;
    for (int transID:_building->GetRoom(_ped->GetRoomID())->GetAllTransitionsIDs())
    {
        Point normal = Point(-(_building->GetTransitionByUID(transID)->GetPoint2()-_building->GetTransitionByUID(transID)->GetPoint1())._y,
                             (_building->GetTransitionByUID(transID)->GetPoint2()-_building->GetTransitionByUID(transID)->GetPoint1())._x);
        normal=normal/normal.Norm();

        if (boost::geometry::within(_building->GetTransitionByUID(transID)->GetCentre()+normal*0.5,_currentEnv) ||
                boost::geometry::within(_building->GetTransitionByUID(transID)->GetCentre()-normal*0.5,_currentEnv))
        {
            transitions.push_back(_building->GetTransitionByUID(transID));
        }
    }
    std::vector<NavLine> transNavLines;
    for (const Transition* transition:transitions)
    {
        //Point dirVec = transition->GetCentre()-_ped->GetPos();
        //dirVec=dirVec/dirVec.Norm();
        NavLine navLine(Line(transition->GetPoint1(),transition->GetPoint2()));

        transNavLines.push_back(navLine);

    }
    return transNavLines;

}

const SubRoom* VisualSystem::IsInSubRoom(const Point &point) const
{

    for (auto it2 = _building->GetRoom(_ped->GetRoomID())->GetAllSubRooms().begin(); it2 != _building->GetRoom(_ped->GetRoomID())->GetAllSubRooms().end(); ++it2)
    {

        BoostPolygon boostP;
        for (Point vertex:it2->second->GetPolygon())
            boost::geometry::append(boostP, vertex);

        boost::geometry::correct(boostP);

        if (boost::geometry::within(point, boostP))
            return it2->second.get();

    }
    return nullptr;


}

const Room* VisualSystem::IsInRoom(const Point &point) const
{

    std::vector<int> transIDs= _building->GetRoom(_ped->GetRoomID())->GetAllTransitionsIDs();
    std::vector<const Room*> adjacentRooms;

    for (int transID:transIDs)
    {
        if (_building->GetTransitionByUID(transID)->GetOtherRoom(_ped->GetRoomID()))
            adjacentRooms.push_back(_building->GetTransitionByUID(transID)->GetOtherRoom(_ped->GetRoomID()));
    }

    for (const Room* room:adjacentRooms)
    {
        for (auto it2 = room->GetAllSubRooms().begin(); it2 != room->GetAllSubRooms().end(); ++it2)
        {

            BoostPolygon boostP;
            for (Point vertex:it2->second->GetPolygon())
            {
                boost::geometry::append(boostP, vertex);
            }
            boost::geometry::correct(boostP);

            if (boost::geometry::within(point, boostP))
                return _building->GetRoom(room->GetID());

        }
    }


    return nullptr;

}

const Transition* VisualSystem::IsTransition(const NavLine *navLine) const
{
    std::vector<int> transIDs= _building->GetRoom(_ped->GetRoomID())->GetAllTransitionsIDs();

    for (int transID:transIDs)
    {
        if (navLine->GetCentre()==_building->GetTransitionByUID(transID)->GetCentre())
            return _building->GetTransitionByUID(transID);
    }

    return nullptr;
}

void VisualSystem::GetVisibleSigns()
{

    _newVisibleSigns.clear();

    const std::vector<Sign>* signsInCurrentFloor = _env->GetSignsOfFloor(_building->GetAllRooms().at(_ped->GetRoomID()).get());

    if (!signsInCurrentFloor)
    {
       _visibleSigns.clear();
       _detectedSigns.clear();
       return;
    }

    for (const Sign& sign:*signsInCurrentFloor)
    {
        // if sign is in visible range
        if (SignLegible(sign))
        {
            // sign in visibility range for the first time
            if (std::find(_visibleSigns.begin(),_visibleSigns.end(),&sign)==_visibleSigns.end())
            {
                _newVisibleSigns.push_back(&sign);
                _visibleSigns.push_back(&sign);
            }
	}
        // if sign not anymore in field of view
        else if (std::find(_visibleSigns.begin(),_visibleSigns.end(),&sign)!=_visibleSigns.end())
        {
            _visibleSigns.erase(std::find(_visibleSigns.begin(),_visibleSigns.end(),&sign));

            if (std::find(_detectedSigns.begin(),_detectedSigns.end(),&sign)!=_detectedSigns.end())
                _detectedSigns.erase(std::find(_detectedSigns.begin(),_detectedSigns.end(),&sign));

        }
    }

}

bool VisualSystem::SignLegible(const Sign &sign) const
{
    //following Signage Legibility Distances as a Function of Observation Angle by Xie et al. 2007


    Point vec1 = _ped->GetPos()-sign.GetPos();
    Point vec2 = sign.GetAlpha()*M_PI/180.0;

    double phi1=std::atan2(vec1._y,vec1._x);
    double phi2=std::atan2(vec2._y,vec2._x);
    double phi = std::fabs(phi2-phi1);

    if(vec1.Norm()>LegibleDistance(phi))
        return false;

    //checking if sign is in not hided by walls or obstacles
    if (boost::geometry::within(sign.GetPos(),_currentEnv))
        return true;


    return false;

}

double VisualSystem::HeadDirCompToSign(const Sign &sign) const
{
    Point pedVec=_ped->GetV();
    double pedAlpha=std::atan2(pedVec._y,pedVec._x);

    Point pedToSignVec=sign.GetPos()-_ped->GetPos();
    double pedToSignAlpha=std::atan2(pedToSignVec._y,pedToSignVec._x);

    return std::fabs(pedAlpha-pedToSignAlpha);

}


double VisualSystem::GetProbabilityToDetectSign(double /*headDir*/, bool hardlyVisSign)
{

    double P=1.0;
    double kappa_alpha=1.0;

    double zeta=0.0;//0.16;
    double eta=0.0;//0.375;

    if (_signPerceived>=1)
        P+=_signPerceived*zeta;
    else if (_signPerceived<0)
        P+=_signPerceived*eta;

    if (hardlyVisSign)
    {
        kappa_alpha=0.5;
        P=(1-kappa_alpha)/(1-P*kappa_alpha)*P;
    }

//    if (headDir>M_PI/4.0)
//    {
//        kappa_alpha=0.5;
//    }
//    else if (headDir>M_PI/2.0)
//    {
//        kappa_alpha=0.0;
//    }
//    else
//    {
//        kappa_alpha=1.0;
//    }

    P*=kappa_alpha;


    if (P<0)
        P=0;
    else if (P>1)
        P=1;

    return P;





}

std::vector<const Sign*> VisualSystem::TryToDetectSigns()
{
    //Visible:= sign in visible range
    //Detect:= Instruction of sign will be followed
    GetVisibleSigns();

    for (const Sign* sign:_newVisibleSigns)
    {
        double headDir=HeadDirCompToSign(*sign);

        TryToDetectSign(*sign, headDir,false);
	//if hard to detect signs shall be considered:
//        if (!TryToDetectSign(*sign, headDir,false))// && headDir>M_PI/4.0)
//        {
//            if(headDir>M_PI/4.0)
//            {
//                _hardlyVisibleSigns.push_back(sign);
//            }
//        }
    }

//    for (const Sign* sign:_hardlyVisibleSigns)
//    {
//        double headDir=HeadDirCompToSign(*sign);

//        if (headDir<M_PI/4.0)
//        {
//            _hardlyVisibleSigns.erase(std::find(_hardlyVisibleSigns.begin(),_hardlyVisibleSigns.end(),sign));
//            TryToDetectSign(*sign,headDir,true);
//        }
//    }

    return _detectedSigns;

}

bool VisualSystem::TryToDetectSign(const Sign &sign, double headDir, bool hardlyVisSign)
{

    double P=GetProbabilityToDetectSign(headDir,hardlyVisSign);
    std::discrete_distribution<int> distribution {1.0-P,P};

    int number = distribution(_building->GetConfig()->GetRandomNumberGenerator()->GetRandomEngine());


    if (number)
    {
        _detectedSigns.push_back(&sign);
        if (_signPerceived<0)
            _signPerceived=1;
        else
            ++_signPerceived;

        return true;
    }
    else
    {
        //if (headDir<M_PI/4.0)
        //{
        if (_signPerceived<=0)
            --_signPerceived;
        //}
        //Log->Write("ROUTEINFO: Sign not detected\n");
        return false;
    }
}

std::vector<std::pair<const Point&,double>> VisualSystem::PerceiveDirsOfOtherPedestrians() const
{
    std::vector<std::pair<const Point&,double>> dirVectors;

    if (_ped->GetV()._x==0.0 && _ped->GetV()._y==0.0)
        return dirVectors;


    const std::vector<Pedestrian*> pedestrians= _building->GetAllPedestrians();

    for (const Pedestrian* ped:pedestrians)
    {
        if (ped!=_ped)
        {
            if (boost::geometry::within(ped->GetPos(),_currentEnv))
            {
                double angle = std::atan2(ped->GetV()._y,ped->GetV()._x)*180/M_PI;
                dirVectors.push_back(std::make_pair<const Point&,double&>(ped->GetPos(),angle));
            }
        }
    }

    return dirVectors;
}


double LegibleDistance(double phi) // based on Xie et al. 2007
{
   return -0.003335*phi*phi+0.07863*phi+19.79;

}
