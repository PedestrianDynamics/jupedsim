/**
 * @file ForceModel.cpp
 *
 * @date 13. December 2010, 15:05
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
 * Implementation of the GCFM and the Gompertz model 
 *
 *
 */

#include "ForceModel.h"
#include "../routing/DirectionStrategy.h"
#include "../mpi/LCGrid.h"
#include "../pedestrian/Pedestrian.h" 


#ifdef _OPENMP
#include <omp.h>
#else
#define omp_get_thread_num() 0
#define omp_get_max_threads()  1
#endif

//int logped=499;

using namespace std;

ForceModel::ForceModel() {
}


ForceModel::~ForceModel() {
}

/************************************************************
 GCFM ForceModel
 ************************************************************/


inline  Point GCFMModel::ForceDriv(Pedestrian* ped, Room* room) const {
        const Point& target = _direction->GetTarget(room, ped);
        Point F_driv;
        const Point& pos = ped->GetPos();
        double dist = ped->GetExitLine()->DistTo(pos);
        
        
        if (dist > J_EPS_GOAL) {
                const Point& v0 = ped->GetV0(target);
                //printf("MC v0= [%.2f %.2f]\n", v0.GetX(), v0.GetY());
                //fprintf(stderr, "%.2f %.2f %.2f %.2f %f %f\n", v0.GetX(), v0.GetY(), pos.GetX(), pos.GetY(), target.GetX(), target.GetY());
                F_driv = ((v0 * ped->GetV0Norm() - ped->GetV()) * ped->GetMass()) / ped->GetTau();
        } else {
                const Point& v0 = ped->GetV0();
                //fprintf(stderr, "%.2f %.2f %.2f %.2f %f %f\n", v0.GetX(), v0.GetY(), pos.GetX(), pos.GetY(), target.GetX(), target.GetY());
                F_driv = ((v0 * ped->GetV0Norm() - ped->GetV()) * ped->GetMass()) / ped->GetTau();
        }
        return F_driv;
}

Point GCFMModel::ForceRepPed(Pedestrian* ped1, Pedestrian* ped2) const {

        Point F_rep;
        // x- and y-coordinate of the distance between p1 and p2
        Point distp12 = ped2->GetPos() - ped1->GetPos();
        const Point& vp1 = ped1->GetV(); // v Ped1
        const Point& vp2 = ped2->GetV(); // v Ped2
        Point ep12; // x- and y-coordinate of the normalized vector between p1 and p2
        double tmp, tmp2;
        double v_ij;
        double K_ij;
        //double r1, r2;
        double nom; //nominator of Frep
        double px; // hermite Interpolation value
        const JEllipse& E1 = ped1->GetEllipse();
        const JEllipse& E2 = ped2->GetEllipse();
        double distsq;
        double dist_eff = E1.EffectiveDistanceToEllipse(E2, &distsq);


        //          smax    dist_intpol_left      dist_intpol_right       dist_eff_max
        //       ----|-------------|--------------------------|--------------|----
        //       5   |     4       |            3             |      2       | 1

        // If the pedestrian is outside the cutoff distance, the force is zero.
        if (dist_eff >= _distEffMaxPed) {
                F_rep = Point(0.0, 0.0);
                return F_rep;
        }
        //Point AP1inE1 = Point(E1.GetXp(), 0); // ActionPoint von E1 in Koordinaten von E1
        //Point AP2inE2 = Point(E2.GetXp(), 0); // ActionPoint von E2 in Koordinaten von E2
        // ActionPoint von E1 in Koordinaten von E2 (transformieren)
        //Point AP1inE2 = AP1inE1.CoordTransToEllipse(E2.GetCenter(), E2.GetCosPhi(), E2.GetSinPhi());
        // ActionPoint von E2 in Koordinaten von E1 (transformieren)
        //Point AP2inE1 = AP2inE2.CoordTransToEllipse(E1.GetCenter(), E1.GetCosPhi(), E1.GetSinPhi());
        //r1 = (AP1inE1 - E1.PointOnEllipse(AP2inE1)).Norm();
        //r2 = (AP2inE2 - E2.PointOnEllipse(AP1inE2)).Norm();

        //%------- Free parameter --------------
        Point p1, p2; // "Normale" Koordinaten
        double mindist;


        p1 = Point(E1.GetXp(), 0).CoordTransToCart(E1.GetCenter(), E1.GetCosPhi(), E1.GetSinPhi());
        p2 = Point(E2.GetXp(), 0).CoordTransToCart(E2.GetCenter(), E2.GetCosPhi(), E2.GetSinPhi());
        distp12 = p2 - p1;
        //mindist = E1.MinimumDistanceToEllipse(E2); //ONE
        mindist = 0.5; //for performance reasons, it is assumed that this distance is about 50 cm 
        double dist_intpol_left = mindist + _intp_widthPed; // lower cut-off for Frep (modCFM)
        double dist_intpol_right = _distEffMaxPed - _intp_widthPed; //upper cut-off for Frep (modCFM)
        double smax = mindist - _intp_widthPed; //max overlapping
        double f = 0.0f, f1 = 0.0f; //function value and its derivative at the interpolation point'

        //todo: runtime normsquare?
        if (distp12.Norm() >= J_EPS) {
                ep12 = distp12.Normalized();
                
        } else {
                Log->Write("ERROR: \tin GCFMModel::forcePedPed() ep12 kann nicht berechnet werden!!!\n");
                Log->Write("ERROR:\t fix this as soon as possible");
                return F_rep; // FIXME: should never happen
                exit(0);

        }
        // calculate the parameter (whatever dist is)
        tmp = (vp1 - vp2).ScalarP(ep12); // < v_ij , e_ij >
        v_ij = 0.5 * (tmp + fabs(tmp));
        tmp2 = vp1.ScalarP(ep12); // < v_i , e_ij >

        //todo: runtime normsquare?
        if (vp1.Norm() < J_EPS) { // if(norm(v_i)==0)
                K_ij = 0;
        } else {
                double bla = tmp2 + fabs(tmp2);
                K_ij = 0.25 * bla * bla / vp1.ScalarP(vp1); //squared

                if (K_ij < J_EPS * J_EPS) {
                        F_rep = Point(0.0, 0.0);
                        return F_rep;
                }
        }
        nom = _nuPed * ped1->GetV0Norm() + v_ij; // Nu: 0=CFM, 0.28=modifCFM;
        nom *= nom;

        K_ij = sqrt(K_ij);
        if (dist_eff <= smax) { //5
                f = -ped1->GetMass() * K_ij * nom / dist_intpol_left;
                F_rep = ep12 * _maxfPed * f;
                return F_rep;
        }

        //          smax    dist_intpol_left      dist_intpol_right       dist_eff_max
        //           ----|-------------|--------------------------|--------------|----
        //       5   |     4       |            3             |      2       | 1

        if (dist_eff >= dist_intpol_right) { //2
                f = -ped1->GetMass() * K_ij * nom / dist_intpol_right; // abs(NR-Dv(i)+Sa)
                f1 = -f / dist_intpol_right;
                px = hermite_interp(dist_eff, dist_intpol_right, _distEffMaxPed, f, 0, f1, 0);
                F_rep = ep12 * px;
        } else if (dist_eff >= dist_intpol_left) { //3
                f = -ped1->GetMass() * K_ij * nom / fabs(dist_eff); // abs(NR-Dv(i)+Sa)
                F_rep = ep12 * f;
        } else {//4
                f = -ped1->GetMass() * K_ij * nom / dist_intpol_left;
                f1 = -f / dist_intpol_left;
                px = hermite_interp(dist_eff, smax, dist_intpol_left, _maxfPed*f, f, 0, f1);
                F_rep = ep12 * px;
        }
        if (F_rep.GetX() != F_rep.GetX() || F_rep.GetY() != F_rep.GetY()) {
                char tmp[CLENGTH];
                sprintf(tmp, "\nNAN return ----> p1=%d p2=%d Frepx=%f, Frepy=%f\n", ped1->GetID(),
                                ped2->GetID(), F_rep.GetX(), F_rep.GetY());
                Log->Write(tmp);
                Log->Write("ERROR:\t fix this as soon as possible");
                return Point(0,0); // FIXME: should never happen
                exit(0); 
        }
        return F_rep;
}

/* abstoßende Kraft zwischen ped und subroom
 * Parameter:
 *   - ped: Fußgänger für den die Kraft berechnet wird
 *   - subroom: SubRoom für den alle abstoßende Kräfte von Wänden berechnet werden
 * Rückgabewerte:
 *   - Vektor(x,y) mit Summe aller abstoßenden Kräfte im SubRoom
 * */

inline Point GCFMModel::ForceRepRoom(Pedestrian* ped, SubRoom* subroom) const {
        Point f = Point(0., 0.);
        //first the walls
        
        const vector<Wall>& walls = subroom->GetAllWalls();
        for (int i = 0; i < subroom->GetNumberOfWalls(); i++) {
            f = f + ForceRepWall(ped, walls[i]); 
        }

        //then the obstacles
        const vector<Obstacle*>& obstacles = subroom->GetAllObstacles();
        for(unsigned int obs=0;obs<obstacles.size();++obs){
                const vector<Wall>& walls = obstacles[obs]->GetAllWalls();
                for (unsigned int i = 0; i < walls.size(); i++) {
                        f = f + ForceRepWall(ped, walls[i]);
                }
        }

        //eventually crossings
        const vector<Crossing*>& crossings = subroom->GetAllCrossings();
        for (unsigned int i = 0; i < crossings.size(); i++) {
                //Crossing* goal=crossings[i];
                //int uid1= goal->GetUniqueID();
                //int uid2=ped->GetExitIndex();
                // ignore my transition
                //if (uid1 != uid2) {
                //      f = f + ForceRepWall(ped,*((Wall*)goal));
                //}
        }

        // and finally the closed doors or doors that are not my destination
        const vector<Transition*>& transitions = subroom->GetAllTransitions();
        for (unsigned int i = 0; i < transitions.size(); i++) {
                Transition* goal=transitions[i];
                int uid1= goal->GetUniqueID();
                int uid2=ped->GetExitIndex();
                // ignore my transition consider closed doors
                //closed doors are considered as wall

                if((uid1 != uid2) || (goal->IsOpen()==false )) {
                        f = f + ForceRepWall(ped,*((Wall*)goal));
                }
        }

        return f;
}


inline Point GCFMModel::ForceRepWall(Pedestrian* ped, const Wall& w) const {
        Point F = Point(0.0, 0.0);
        Point pt = w.ShortestPoint(ped->GetPos());
        double wlen = w.LengthSquare();

        if (wlen < 0.01) { // ignore walls smaller than 10 cm
                return F;
        }
        // Kraft soll nur orthgonal wirken
        // ???
        if (fabs((w.GetPoint1() - w.GetPoint2()).ScalarP(ped->GetPos() - pt)) > J_EPS)
        {
            return F;
        }
        //double mind = ped->GetEllipse().MinimumDistanceToLine(w);
        double mind = 0.5; //for performance reasons this distance is assumed to be constant
        double vn = w.NormalComp(ped->GetV()); //normal component of the velocity on the wall
        return  ForceRepStatPoint(ped, pt, mind, vn); //line --> l != 0
}

/* abstoßende Punktkraft zwischen ped und Punkt p
 * Parameter:
 *   - ped: Fußgänger für den die Kraft berechnet wird
 *   - p: Punkt von dem die Kaft wirkt
 *   - l: Parameter zur Käfteinterpolation
 *   - vn: Parameter zur Käfteinterpolation
 * Rückgabewerte:
 *   - Vektor(x,y) mit abstoßender Kraft
 * */
//TODO: use effective DistanceToEllipse and simplify this function.
Point GCFMModel::ForceRepStatPoint(Pedestrian* ped, const Point& p, double l, double vn) const {
        Point F_rep = Point(0.0, 0.0);
        const Point& v = ped->GetV();
        Point dist = p - ped->GetPos(); // x- and y-coordinate of the distance between ped and p
        double d = dist.Norm(); // distance between the centre of ped and point p
        Point e_ij; // x- and y-coordinate of the normalized vector between ped and p
        double K_ij;
        double tmp;
        double bla;
        Point r;
        Point pinE; // vorher x1, y1
        const JEllipse& E = ped->GetEllipse();

        if (d < J_EPS)
                return Point(0.0, 0.0);
        e_ij = dist / d;
        tmp = v.ScalarP(e_ij); // < v_i , e_ij >;
        bla = (tmp + fabs(tmp));
        if (!bla) // Fussgaenger nicht im Sichtfeld
                return Point(0.0, 0.0);
        if (fabs(v.GetX()) < J_EPS && fabs(v.GetY()) < J_EPS) // v==0)
                return Point(0.0, 0.0);
        K_ij = 0.5 * bla / v.Norm(); // K_ij
        // Punkt auf der Ellipse
        pinE = p.CoordTransToEllipse(E.GetCenter(), E.GetCosPhi(), E.GetSinPhi());
        // Punkt auf der Ellipse
        r = E.PointOnEllipse(pinE);
        //interpolierte Kraft
        F_rep = ForceInterpolation(ped->GetV0Norm(), K_ij, e_ij, vn, d, (r - E.GetCenter()).Norm(), l);
        return F_rep;
}

Point GCFMModel::ForceInterpolation(double v0, double K_ij, const Point& e, double vn, double d, double r, double l) const {
        Point F_rep;
        double nominator = _nuWall * v0 + vn;
        nominator *= nominator*K_ij;
        double f = 0, f1 = 0; //function value and its derivative at the interpolation point
        //BEGIN ------- interpolation parameter
        double smax = l - _intp_widthWall; // max overlapping radius
        double dist_intpol_left = l + _intp_widthWall; //r_eps
        double dist_intpol_right = _distEffMaxWall - _intp_widthWall;
        //END ------- interpolation parameter

        double dist_eff = d - r;

        //         smax    dist_intpol_left      dist_intpol_right       dist_eff_max
        //           ----|-------------|--------------------------|--------------|----
        //       5   |     4       |            3             |      2       | 1

        double px = 0; //value of the interpolated function
        double tmp1 = _distEffMaxWall;
        double tmp2 = dist_intpol_right;
        double tmp3 = dist_intpol_left;
        double tmp5 = smax + r;

        if (dist_eff >= tmp1) { // 1
                //F_rep = Point(0.0, 0.0);
                return F_rep;
        }

        if (dist_eff <= tmp5) { // 5
                F_rep = e * (-_maxfWall);
                return F_rep;
        }

        if (dist_eff > tmp2) { //2
                f = -nominator / dist_intpol_right;
                f1 = -f / dist_intpol_right; // nominator / (dist_intpol_right^2) = derivativ of f
                px = hermite_interp(dist_eff, dist_intpol_right, _distEffMaxWall, f, 0, f1, 0);
                F_rep = e * px;
        } else if (dist_eff >= tmp3) { //3
                f = -nominator / fabs(dist_eff); //devided by abs f the effective distance
                F_rep = e * f;
        } else { //4 d > smax FIXME
                f = -nominator / dist_intpol_left;
                f1 = -f / dist_intpol_left;
                px = hermite_interp(dist_eff, smax, dist_intpol_left, _maxfWall*f, f, 0, f1);
                F_rep = e * px;
        }
        return F_rep;
}



GCFMModel::GCFMModel(DirectionStrategy* dir, double nuped, double nuwall, double dist_effPed,
                double dist_effWall, double intp_widthped, double intp_widthwall, double maxfped,
                double maxfwall) {
        _direction = dir;
        _nuPed = nuped;
        _nuWall = nuwall;
        _intp_widthPed = intp_widthped;
        _intp_widthWall = intp_widthwall;
        _maxfPed = maxfped;
        _maxfWall = maxfwall;
        _distEffMaxPed = dist_effPed;
        _distEffMaxWall = dist_effWall;

}

GCFMModel::~GCFMModel(void) {

}

// Getter-Funktionen

DirectionStrategy* GCFMModel::GetDirection() const {
        return _direction;
}

double GCFMModel::GetNuPed() const {
        return _nuPed;
}

double GCFMModel::GetNuWall() const {
        return _nuWall;
}

double GCFMModel::GetIntpWidthPed() const {
        return _intp_widthPed;
}

double GCFMModel::GetIntpWidthWall() const {
        return _intp_widthWall;
}

double GCFMModel::GetMaxFPed() const {
        return _maxfPed;
}

double GCFMModel::GetMaxFWall() const {
        return _maxfWall;
}

double GCFMModel::GetDistEffMaxPed() const {
        return _distEffMaxPed;
}

double GCFMModel::GetDistEffMaxWall() const {
        return _distEffMaxWall;
}

string GCFMModel::writeParameter() const {
        string rueck;
        char tmp[CLENGTH];

        sprintf(tmp, "\t\tNu: \t\tPed: %f \tWall: %f\n", _nuPed, _nuWall);
        rueck.append(tmp);
        sprintf(tmp, "\t\tInterp. Width: \tPed: %f \tWall: %f\n", _intp_widthPed, _intp_widthWall);
        rueck.append(tmp);
        sprintf(tmp, "\t\tMaxF: \t\tPed: %f \tWall: %f\n", _maxfPed, _maxfWall);
        rueck.append(tmp);
        sprintf(tmp, "\t\tDistEffMax: \tPed: %f \tWall: %f\n", _distEffMaxPed, _distEffMaxWall);
        rueck.append(tmp);

        return rueck;
}


// virtuelle Funktionen

void GCFMModel::CalculateForce(double time, vector< Point >& result_acc, Building* building,
                int roomID, int subroomID) const {

        printf("CalculateForce is not working: you should not use this function");
        exit(0);
}

/**
 * implementation of Linked-cell combined with openMP
 */

void GCFMModel::CalculateForceLC(double time, double tip1, Building* building) const {
        double delta = 0.5;
        double h = tip1 - time;
        // collect all pedestrians in the simulation.
        const vector< Pedestrian* >& allPeds = building->GetAllPedestrians();

        unsigned int nSize = allPeds.size();

        int nThreads = omp_get_max_threads();

        // check if worth sharing the work
        if (nSize < 20) nThreads = 1;
        int partSize = nSize / nThreads;

#pragma omp parallel  default(shared) num_threads(nThreads)
        {
                vector< Point > result_acc = vector<Point > ();
                result_acc.reserve(2200);

        const int threadID = omp_get_thread_num();

                int start = threadID*partSize;
                int end = (threadID + 1) * partSize - 1;
                if ((threadID == nThreads - 1)) end = nSize - 1;

                for (int p = start; p <= end; ++p) {

                        Pedestrian* ped = allPeds[p];
                        Room* room = building->GetRoom(ped->GetRoomID());
                        SubRoom* subroom = room->GetSubRoom(ped->GetSubRoomID());
                        if(subroom->GetType()=="cellular") continue;

                        double normVi = ped->GetV().ScalarP(ped->GetV());
                        double tmp = (ped->GetV0Norm() + delta) * (ped->GetV0Norm() + delta);
                        if (normVi > tmp && ped->GetV0Norm() > 0) {
                                fprintf(stderr, "GCFMModel::calculateForce() WARNING: actual velocity (%f) of iped %d "
                                                "is bigger than desired velocity (%f) at time: %fs\n",
                                                sqrt(normVi), ped->GetID(), ped->GetV0Norm(), time);

                                // remove the pedestrian and abort
                                for(int p=0;p<subroom->GetNumberOfPedestrians();p++){
                                        if (subroom->GetPedestrian(p)->GetID()==ped->GetID()){
                                                subroom->DeletePedestrian(p);
                                                break;
                                        }
                                }

                                building->DeletePedFromSim(ped);
                                Log->Write("\tCRITICAL: one ped was removed due to high velocity");

                                //      continue;
                                exit(EXIT_FAILURE);
                        }

                        Point F_rep;
                        vector<Pedestrian*> neighbours;
                        building->GetGrid()->GetNeighbourhood(ped,neighbours);

                        int nSize=neighbours.size();
                        for (int i = 0; i < nSize; i++) {
                                Pedestrian* ped1 = neighbours[i];
                                //if they are in the same subroom
                                if (ped->GetUniqueRoomID() == ped1->GetUniqueRoomID()) {
                                        F_rep = F_rep + ForceRepPed(ped, ped1);
                                } else {
                                        // or in neighbour subrooms
                                        SubRoom* sb2=building->GetRoom(ped1->GetRoomID())->GetSubRoom(ped1->GetSubRoomID());
                                        if(subroom->IsDirectlyConnectedWith(sb2)){
                                                F_rep = F_rep + ForceRepPed(ped, ped1);
                                        }
                                }
                        }//for peds

                        //repulsive forces to the walls and transitions that are not my target
                        Point repwall = ForceRepRoom(allPeds[p], subroom);
                        Point fd = ForceDriv(ped, room);
                        // Point acc = (ForceDriv(ped, room) + F_rep + repwall) / ped->GetMass();
                        Point acc = (fd + F_rep + repwall) / ped->GetMass();
                        result_acc.push_back(acc);
                }

                //#pragma omp barrier
                // update
                for (int p = start; p <= end; ++p) {
                        Pedestrian* ped = allPeds[p];
                        Point v_neu = ped->GetV() + result_acc[p - start] * h;
                        Point pos_neu = ped->GetPos() + v_neu * h;

                        Room* room = building->GetRoom(ped->GetRoomID());
                        SubRoom* subroom = room->GetSubRoom(ped->GetSubRoomID());
                        if(subroom->GetType()=="cellular") continue;

                        //Jam is based on the current velocity
                        if (v_neu.Norm() >= J_EPS_V){
                                ped->ResetTimeInJam();
                        }else{
                                ped->UpdateTimeInJam();
                        }

                        ped->SetPos(pos_neu);
                        ped->SetV(v_neu);
                        ped->SetPhiPed();
                }

        }//end parallel

        //update the CA Model
        //UpdateCellularModel(building);
}

// void GCFMModel::UpdateCellularModel(Building* building) const {

//      const vector< Pedestrian* >& allPeds = building->GetAllPedestrians();

// }

/************************************************************
 Gompertz ForceModel
 ************************************************************/


Point GompertzModel::ForceDriv(Pedestrian* ped, Room* room) const {
    const Point& target = _direction->GetTarget(room, ped);
    Point F_driv;
    const Point& pos = ped->GetPos();
    double dist = ped->GetExitLine()->DistTo(pos);
        
        
    if (dist > J_EPS_GOAL) {
        const Point& v0 = ped->GetV0(target);
        //printf("MC v0= [%.2f %.2f]\n", v0.GetX(), v0.GetY());
        //fprintf(stderr, "%.2f %.2f %.2f %.2f %f %f\n", v0.GetX(), v0.GetY(), pos.GetX(), pos.GetY(), target.GetX(), target.GetY());
        F_driv = ((v0 * ped->GetV0Norm() - ped->GetV()) * ped->GetMass()) / ped->GetTau();
    } else {
        const Point& v0 = ped->GetV0();
        //fprintf(stderr, "%.2f %.2f %.2f %.2f %f %f\n", v0.GetX(), v0.GetY(), pos.GetX(), pos.GetY(), target.GetX(), target.GetY());
        F_driv = ((v0 * ped->GetV0Norm() - ped->GetV()) * ped->GetMass()) / ped->GetTau();
    }
    return F_driv;
}

Point GompertzModel::ForceRepPed(Pedestrian* ped1, Pedestrian* ped2) const {
    Point F_rep = Point(0.0, 0.0);;
    // x- and y-coordinate of the distance between p1 and p2
    Point distp12 = ped2->GetPos() - ped1->GetPos();
    //const Point& vp1 = ped1->GetV(); // v Ped1
    //const Point& vp2 = ped2->GetV(); // v Ped2
    Point ep12; // x- and y-coordinate of the normalized vector between p1 and p2
    //double K_ij;
    double B_ij, f;
    const JEllipse& E1 = ped1->GetEllipse();
    const JEllipse& E2 = ped2->GetEllipse();
    Point AP1inE1 = E1.GetCenter();
    Point AP2inE2 = E2.GetCenter();
    // ActionPoint von E1 in Koordinaten von E2 (transformieren)
    Point AP1inE2 = AP1inE1.CoordTransToEllipse(E2.GetCenter(), E2.GetCosPhi(), E2.GetSinPhi());
    // ActionPoint von E2 in Koordinaten von E1 (transformieren)
    Point AP2inE1 = AP2inE2.CoordTransToEllipse(E1.GetCenter(), E1.GetCosPhi(), E1.GetSinPhi());
    double r1 = (AP1inE1 - E1.PointOnEllipse(AP2inE1)).Norm();
    double r2 = (AP2inE2 - E2.PointOnEllipse(AP1inE2)).Norm();
    double Distance = distp12.Norm();
    
    // if(ped1->GetID() ==logped)
    // {
    //     printf("ped1=%d ped2=%d  Distance=%f\n",ped1->GetID(), ped2->GetID(), Distance);
    // }
    if (Distance >= J_EPS) {
        ep12 = distp12.Normalized();
    } 
    else {
        //printf("ERROR: \tin GompertzModel::forcePedPed() ep12 can not be calculated!!!\n");
        Log->Write("ERROR: \tin GompertzModel::forcePedPed() ep12 can not be calculated!!!\n");
        Log->Write("\t\t Pedestrians are too near to each other.");
        Log->Write("\t\t Get your model right. Going to exit.");
        return F_rep; // should never happen
        exit(0);
    }
//------------------------- check if others are behind using v0 instead of v 
    double tmp = ped1->GetV0().ScalarP(ep12); // < v^0_i , e_ij >
    
    double ped2IsBehind = exp(-exp(-5*tmp)); //step function: continuous version
    if (ped2IsBehind < J_EPS){

            // if(ped1->GetID() ==logped)
            //     printf("isBehind=%f\n", ped2IsBehind);

        return F_rep; // ignore ped2
    }
//------------------------------------------------------------------------------

    // calculate B_ij
    B_ij = 1.0 - Distance/(r1+r2); //TODO: Simplification to avoid accelerating predecessors
    //B_ij = 1.0 - Distance/(r1 + r2);
    //Gompertz-function parameter. 
    //TODO: Check later if other values are more appropriate
    double b = 0.25, c = 3.0; //repped
    
    B_ij = exp(-b*exp(-c*B_ij));
    //TODO: check if we need K_ij in the  f
    //f = -ped1->GetMass() * _nuPed * ped1->GetV0Norm() * K_ij * B_ij;
    f = -ped1->GetMass() * _nuPed * ped1->GetV0Norm() * B_ij;
    F_rep = ep12 * f;
    
    // if(ped1->GetID() == logped)
    // {    
    //     printf("return ----> p1=%d p2=%d Frepx=%f, Frepy=%f\n", ped1->GetID(),
    //            ped2->GetID(), F_rep.GetX(), F_rep.GetY());
    //     printf("ped1= [%f, %f], ped2=[%f %f]\n", ped1->GetPos().GetX(), ped1->GetPos().GetY(), ped2->GetPos().GetX(), ped2->GetPos().GetY());
    // }
   
 //check isNan
    if (F_rep.GetX() != F_rep.GetX() || F_rep.GetY() != F_rep.GetY()) {
        char tmp[CLENGTH];
        sprintf(tmp, "\nNAN return ----> p1=%d p2=%d Frepx=%f, Frepy=%f\n", ped1->GetID(),
                ped2->GetID(), F_rep.GetX(), F_rep.GetY());
        Log->Write(tmp);
        Log->Write("ERROR:\t fix this as soon as possible");
        return Point(0,0); // FIXME: should never happen
        exit(0); 
    }
    return F_rep;
}//END Gompertz:ForceRepPed()

Point GompertzModel::ForceRepRoom(Pedestrian* ped, SubRoom* subroom) const{
    Point f = Point(0., 0.);
    //first the walls
    const vector<Wall>& walls = subroom->GetAllWalls();
    for (int i = 0; i < subroom->GetNumberOfWalls(); i++) {
        f = f + ForceRepWall(ped, walls[i]);
    }

    //then the obstacles
    const vector<Obstacle*>& obstacles = subroom->GetAllObstacles();
    for(unsigned int obs=0;obs<obstacles.size();++obs){
        const vector<Wall>& walls = obstacles[obs]->GetAllWalls();
        for (unsigned int i = 0; i < walls.size(); i++) {
            f = f + ForceRepWall(ped, walls[i]);
        }
    }
    // and finally the closed doors
    const vector<Transition*>& transitions = subroom->GetAllTransitions();
    for (unsigned int i = 0; i < transitions.size(); i++) {
        Transition* goal=transitions[i];
        if( goal->IsOpen() == false ) {
            f = f + ForceRepWall(ped,*((Wall*)goal));
        }
    }
    
    return f;
}
Point GompertzModel::ForceRepWall(Pedestrian* ped, const Wall& w) const{
    Point F_wrep = Point(0.0, 0.0);
    // printf("in GompertzWall\n");
    // getc(stdin);
    // if direction of pedestrians does not intersect walls --> ignore
    
    Point pt = w.ShortestPoint(ped->GetPos());
    double wlen = w.LengthSquare();
    if (wlen <= 0.03) { // ignore walls smaller than 0.15m  (15cm)
        // if(ped->GetID() == 33)
        //     printf ("Wall beeing ignored [%f %f]<-->[%f %f]\n", w.GetPoint1().GetX(), w.GetPoint1().GetY(), w.GetPoint2().GetX(), w.GetPoint2().GetY());
        return F_wrep;          
    }
    Point dist = pt - ped->GetPos(); // x- and y-coordinate of the distance between ped and p
    double Distance = dist.Norm(); // distance between the centre of ped and point p
    //double vn = w.NormalComp(ped->GetV()); //normal component of the velocity on the wall
    Point e_iw; // x- and y-coordinate of the normalized vector between ped and pt
    //double K_iw;
    double Radius, B_iw;
    double tmp;
    double f;
    Point r;
    Point pinE; // vorher x1, y1
    const JEllipse& E = ped->GetEllipse();
    const Point& v = ped->GetV();

    if (Distance < J_EPS){
        Log->Write("WARNING:\t Gompertz: forceRepWall() ped %d is too near to the wall. Return default values",ped->GetID());    
        return Point(0, 0); //quick and dirty. Should react to the warning and fix the model
    }
    e_iw = dist / Distance;
    // tmp = v.ScalarP(e_iw); // < v_i , e_iw >;
    // bla = (tmp + fabs(tmp));
    // if (bla < J_EPS) // Wall is behind the direction of motion
    //     return F;
    
//------------------------- check if others are behind using v0 instead of v
    tmp = ped->GetV0().ScalarP(e_iw); // < v^0_i , e_iw >
    double tmpv = v.ScalarP(e_iw);
    double wallIsBehind = exp(-exp(-5*tmp)); //step function: continuous version
    double wallIsBehindv = exp(-exp(-5*tmpv)); //step function: continuous version
    // if(ped->GetID() == 40)
    // {
    //     printf ("Wall [%f %f]<-->[%f %f]\n", w.GetPoint1().GetX(), w.GetPoint1().GetY(), w.GetPoint2().GetX(), w.GetPoint2().GetY());
    //     printf("v=[%f, %f] v0=[%f, %f] eiw=[%f, %f], wlen=%f\n",v.GetX(), v.GetY(), ped->GetV0().GetX(),ped->GetV0().GetY(), e_iw.GetX(), e_iw.GetY(), wlen);
    //     printf("tmp=%f, tmpv=%f\n",tmp, tmpv);
    //     printf("wall2IsBehind=%f,  wallIsBehindv=%f\n", wallIsBehind, wallIsBehindv);
    // }
    


    if ( wallIsBehindv < J_EPS && wallIsBehind < J_EPS) { // Wall is behind the direction of motion
           return F_wrep;
      }
//------------------------------------------------------------------------
// if (fabs(v.GetX()) < J_EPS_V && fabs(v.GetY()) < J_EPS_V) // v==0)
    //     return F;
    
    // K_iw = 0.5 * bla / v.Norm(); // K_iw 

    // pt in coordinate system of Ellipse 
    pinE = pt.CoordTransToEllipse(E.GetCenter(), E.GetCosPhi(), E.GetSinPhi());
    // Punkt auf der Ellipse
    r = E.PointOnEllipse(pinE);
    Radius  = (r - E.GetCenter()).Norm();

    //-------------------------
    
    const Point& pos = ped->GetPos();
    double distGoal = ped->GetExitLine()->DistTo(pos);
    if(distGoal < J_EPS_GOAL)
        return F_wrep;

    Line  direction = Line(ped->GetPos(), ped->GetPos() + v*100);
    if(Distance>Radius && direction.IntersectionWith(w) == false )
    {
        return F_wrep;
    }
    
//-------------------------



    //TODO: Check later if other values are more appropriate
    double b = 0.7, c = 3.0; 
    B_iw = 1.0 - Distance/(Radius);
    B_iw = exp(-b*exp(-c*B_iw));
    //f = -ped->GetMass() * _nuWall * ped->GetV0Norm() * K_iw * B_iw;
    f = -ped->GetMass() * _nuWall * B_iw  * ped->GetV0Norm();

    F_wrep = e_iw * f;

     // if(1)// && F_wrep.Norm()>J_EPS && w.GetPoint1().GetX()==w.GetPoint2().GetX() )
     // { 
     //     printf("\n---- %d Radius=%f, dist=%f\n",ped->GetID(), Radius, Distance);
     //     printf ("Wall  [%f %f]<-->[%f %f] Force=[%f ,%f]\n", w.GetPoint1().GetX(), w.GetPoint1().GetY(), w.GetPoint2().GetX(), w.GetPoint2().GetY(), F_wrep.GetX(), F_wrep.GetY());
     //     printf("X= %f, Y=%f\n",ped->GetPos().GetX(), ped->GetPos().GetY());
     // }
    return F_wrep;
}

void GompertzModel::CalculateForceLC(double time, double tip1, Building* building) const {
    double delta = 0.5;
    double h = tip1 - time;
    // collect all pedestrians in the simulation.
    const vector< Pedestrian* >& allPeds = building->GetAllPedestrians();

    unsigned int nSize = allPeds.size();

    int nThreads = omp_get_max_threads();

    // check if worth sharing the work
    if (nSize < 20) nThreads = 1;
    int partSize = nSize / nThreads;

#pragma omp parallel  default(shared) num_threads(nThreads)
    {
        vector< Point > result_acc = vector<Point > ();
        result_acc.reserve(2200); //FIXME What is 2200?

        const int threadID = omp_get_thread_num();

        int start = threadID*partSize;
        int end = (threadID + 1) * partSize - 1;
        if ((threadID == nThreads - 1)) end = nSize - 1;

        for (int p = start; p <= end; ++p) {

            Pedestrian* ped = allPeds[p];
            Room* room = building->GetRoom(ped->GetRoomID());
            SubRoom* subroom = room->GetSubRoom(ped->GetSubRoomID());

            double normVi = ped->GetV().ScalarP(ped->GetV()); //squared
            double HighVel = (ped->GetV0Norm() + delta) * (ped->GetV0Norm() + delta); //(v0+delta)^2
            if (0 && normVi > HighVel && ped->GetV0Norm() > 0) {
                fprintf(stderr, "GompertzModel::calculateForce_LC() WARNING: actual velocity (%f) of iped %d "
                        "is bigger than desired velocity (%f) at time: %fs\n",
                        sqrt(normVi), ped->GetID(), ped->GetV0Norm(), time);

                // remove the pedestrian and abort
                for(int p=0;p<subroom->GetNumberOfPedestrians();p++){
                    if (subroom->GetPedestrian(p)->GetID() == ped->GetID()){
                        subroom->DeletePedestrian(p);
                        break;
                    }
                }             
                
                printf("\tCCRITICAL: ped [%d] was removed due to high velocity\n",ped->GetID());
                Log->Write("\tCRITICAL: ped [%d] was removed due to high velocity",ped->GetID());
                
                building->DeletePedestrian(ped);
                //continue;  //FIXME tolerate first
                exit(EXIT_FAILURE);
            }

            Point repPed = Point(0,0);
            vector<Pedestrian*> neighbours;
            building->GetGrid()->GetNeighbourhood(ped,neighbours);

            int nSize = neighbours.size();
            // double B_ij=0;
            // int count_Bij=0;

            for (int i = 0; i < nSize; i++) {
                Pedestrian* ped1 = neighbours[i];
                //-------------- TESTING ---------
                // Point distp12 = ped1->GetPos() - ped->GetPos();
                // double Distance = distp12.Norm();
                // double tmp;
                // tmp = 1.0 - Distance/(0.35 + 0.35); 
                // B_ij += exp(-1*exp(-1*tmp));
                // if (B_ij > J_EPS)
                //     count_Bij += 1;
                // if (ped->GetID()==logped)
                //     printf("Bij=%f, dist=%f\n", B_ij, Distance);
                //--------------------------------
                //if they are in the same subroom
                if (ped->GetUniqueRoomID() == ped1->GetUniqueRoomID()) {
                    repPed = repPed + ForceRepPed(ped, ped1);
                } 
                else {
                    // or in neighbour subrooms
                    SubRoom* sb2=building->GetRoom(ped1->GetRoomID())->GetSubRoom(ped1->GetSubRoomID());
                    if(subroom->IsDirectlyConnectedWith(sb2)){
                        repPed = repPed + ForceRepPed(ped, ped1);
                    }
                }
            }
            //repulsive forces to walls and closed transitions that are not my target
            Point repWall = ForceRepRoom(allPeds[p], subroom);
            Point fd = ForceDriv(ped, room);
              // if(count_Bij)
            //     B_ij /=count_Bij;
            // else
            //     B_ij = 0;
              // double correction = ped->GetV0Norm()*(-B_ij)/ped->GetTau();


              //fprintf(stderr, "%f\n", correction);
            // if (ped->GetID()==logped){
            //     printf("Bij=%f, count=%d\n", B_ij, count_Bij);
            //     printf(">>MC Gompertz fd=[%.2f, %.2f] F_rep=[%.2f, %.2f], repWall=[%.2f, %.2f] cor=%f\n", fd.GetX(), fd.GetY(),  repPed.GetX(), repPed.GetY(), repWall.GetX(), repWall.GetY(), correction );
            // }

            // make pedestrians want to walk slower in jam
            //fd = fd + Point(correction, correction);

            Point acc = (fd + repPed + repWall) / ped->GetMass();
             // if (1){
             //     printf("<<%d GOMPERTZ fd=[%.2f, %.2f] F_rep=[%.2f, %.2f], repWall=[%.2f, %.2f], acc=[%.2f, %.2f]\n",ped->GetID(), fd.GetX(), fd.GetY(),  repPed.GetX(), repPed.GetY(), repWall.GetX(), repWall.GetY(), acc.GetX(), acc.GetY() );
             // }
//--------------------------------- limiting high accelerations to v0/tau *coef
            // double  nacc = acc.Norm();
            //double maxf = ped->GetV0Norm()/ped->GetTau();
            // if (0 && nacc > maxf){
            //     Point tacc = acc/nacc;
            //     if (ped->GetID()==logped){
            //         printf("acc BEVOORE acc=[%.2f, %.2f] maxf->%f\n, nacc=%f\n\n", acc.GetX(), acc.GetY(), maxf, nacc);
            //     }
            //     acc = tacc * maxf* 0.2; 
            //     if (ped->GetID()==logped){
            //         printf("acc CHANGED acc=[%.2f, %.2f] maxf->%f\n, nacc=%f\n\n", acc.GetX(), acc.GetY(), maxf, nacc);
            //     }
            // }
//------------------------------------------------------------------------------------------

            result_acc.push_back(acc);
        }

        //#pragma omp barrier
        // update
        for (int p = start; p <= end; ++p) {
            Pedestrian* ped = allPeds[p];
          
            Point vToAdd = result_acc[p - start] * h;
            //----------------- update new pos and new vel -----------------
            Point v_neu = ped->GetV() + vToAdd;
            Point pos_neu = ped->GetPos() + v_neu * h;
            //---------------------------------------------------------------
            //Room* room = building->GetRoom(ped->GetRoomID());
            //SubRoom* subroom = room->GetSubRoom(ped->GetSubRoomID());
            //if(subroom->GetType()=="cellular") continue;


            Point e0 = ped->GetV0();
            double isBackwards; 
            //isBackwards = ped->GetV().GetX()*e0.GetX() + ped->GetV().GetX()*e0.GetY();
            isBackwards = v_neu.GetX()*e0.GetX() + v_neu.GetY()*e0.GetY();

            

            // if (ped->GetID()==logped){
            //     printf("<<MC GOMPERTZ v=[%.2f, %.2f] v0=[%.2f, %.2f], isBack=[%.8f]\n", ped->GetV().GetX(), ped->GetV().GetY(),  e0.GetX(), e0.GetY(), isBackwards );
            // }

            if (ped->GetV().Norm()>J_EPS_V)
            {       
                isBackwards = isBackwards/(v_neu.Norm() * e0.Norm()); //normalize
                if(isBackwards <= J_EPS_V) // Pedestrian is moving in the wrong direction
                {
                    // if (ped->GetID()==logped){
                    //     printf("\t\tBREAK\n");
                    // }
                    v_neu = v_neu*0.01;
                    pos_neu = ped->GetPos() + v_neu *h ;
                }           
            }
            
            if(v_neu.Norm() > ped->GetV0Norm()+0.2 ) // Stop pedestrians
            { 
                v_neu = v_neu*0.01;
                pos_neu = ped->GetPos();
            }
//--------------------------------------- apparently this depends on the chosen model Issue 9 -----
            //Jam is based on the current velocity
//            if ( v_neu.Norm() >= J_EPS_V){
//                ped->ResetTimeInJam();
//            }else{
//                ped->UpdateTimeInJam();
//            }

            if ( v_neu.Norm() >= ped->GetV0Norm()*0.5){
                ped->ResetTimeInJam();
            }else{
                ped->UpdateTimeInJam();
            }
//--------------------------------------------------------------------------------------------------


            // if (ped->GetID()==logped){
            //     printf("<<MC GOMPERTZ pos=[%.2f, %.2f] vv=[%.2f, %.2f]\n", ped->GetPos().GetX(), ped->GetPos().GetY(),  ped->GetV().GetX(), ped->GetV().GetY());
            //     printf("<<MC GOMPERTZ pos_new=[%.2f, %.2f] vv_new=[%.2f, %.2f]\n", pos_neu.GetX(), pos_neu.GetY(),  v_neu.GetX(), v_neu.GetY());
            //     //getc(stdin);
            // }                    
                       
            ped->SetPos(pos_neu);
            ped->SetV(v_neu);
            ped->SetPhiPed();
        }
    }//end parallel
}
string GompertzModel::writeParameter() const {
        string rueck;
        char tmp[CLENGTH];

        sprintf(tmp, "\t\tNu: \t\tPed: %f \tWall: %f\n", _nuPed, _nuWall);
        rueck.append(tmp);
        return rueck;
}
DirectionStrategy* GompertzModel::GetDirection() const { return _direction;}

double GompertzModel::GetNuPed() const {return _nuPed;}

double GompertzModel::GetNuWall() const {return _nuWall;}

GompertzModel::GompertzModel(DirectionStrategy* dir, double nuped, double nuwall) {
        _direction = dir;
        _nuPed = nuped;
        _nuWall = nuwall;
}

void GompertzModel::CalculateForce(double time, vector< Point >& result_acc, Building* building,
                int roomID, int subroomID) const {

        printf("CalculateForce is not working: you should not use this function");
        exit(0);
}

GompertzModel::~GompertzModel(void) { }









