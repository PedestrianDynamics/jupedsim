/**
 * File:   ForceModel.cpp
 *
 * Created on 13. December 2010, 15:05
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
using namespace std;

ForceModel::ForceModel() {
}


ForceModel::~ForceModel() {
}

/************************************************************
 GCFM ForceModel
 ************************************************************/

// Private Funktionen

/* treibende Kraft
 * Parameter:
 *   - ped: Fußgänger für den die Kraft berechnet wird
 *   - room: Raum (mit SubRooms) in dem das Ziel gesucht wird
 * Rückgabewerte:
 *   - Vektor(x,y) zum Ziel
 * */
inline Point GCFMModel::ForceDriv(Pedestrian* ped, Room* room) const {
	const Point& target = _direction->GetTarget(room, ped);
	Point F_driv;
	const Point& pos = ped->GetPos();
	double dist = ped->GetExitLine()->DistTo(pos);

	if (dist > J_EPS_GOAL) {
		const Point& v0 = ped->GetV0(target);
		F_driv = ((v0 * ped->GetV0Norm() - ped->GetV()) * ped->GetMass()) / ped->GetTau();
	} else {
		const Point& v0 = ped->GetV0();
		F_driv = ((v0 * ped->GetV0Norm() - ped->GetV()) * ped->GetMass()) / ped->GetTau();
	}
	return F_driv;
}

/* abstoßende Kraft zwischen ped1 und ped2
 * Parameter:
 *   - ped1: Fußgänger für den die Kraft berechnet wird
 *   - ped2: Fußgänger mit dem die Kraft berechnet wird
 * Rückgabewerte:
 *   - Vektor(x,y) mit abstoßender Kraft
 * */
inline Point GCFMModel::ForceRepPed(Pedestrian* ped1, Pedestrian* ped2) const {

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
	//	     ----|-------------|--------------------------|--------------|----
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

//	//eventually crossings
//	const vector<Crossing*>& crossings = subroom->GetAllCrossings();
//	for (unsigned int i = 0; i < crossings.size(); i++) {
//		Crossing* goal=crossings[i];
//		int uid1= goal->GetUniqueID();
//		int uid2=ped->GetExitLine()->GetUniqueID();
//		// ignore my transition
//		if (uid1 != uid2) {
//			f = f + ForceRepWall(ped,*((Wall*)goal));
//		}
//	}
//
	// and finally the closed doors
	const vector<Transition*>& transitions = subroom->GetAllTransitions();
	for (unsigned int i = 0; i < transitions.size(); i++) {
		Transition* goal=transitions[i];
		//int uid1= goal->GetUniqueID();
		//int uid2=ped->GetExitLine()->GetUniqueID();
		// ignore my transition consider closed doors
		//closed doors are considered as wall

		if( goal->IsOpen()==false ) {
			f = f + ForceRepWall(ped,*((Wall*)goal));
		}
	}

	return f;
}


inline Point GCFMModel::ForceRepWall(Pedestrian* ped, const Wall& w) const {
	Point F = Point(0.0, 0.0);
	Point pt = w.ShortestPoint(ped->GetPos());
	double wlen = w.LengthSquare();
	if (wlen < 0.01) { // ignore wa;; smaller than 10 cm
		return F;
	}
	// Kraft soll nur orthgonal wirken

	if (fabs((w.GetPoint1() - w.GetPoint2()).ScalarP(ped->GetPos() - pt)) > J_EPS)
		return F;

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
	//	     ----|-------------|--------------------------|--------------|----
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

void GCFMModel::CalculateForceLC(double time, double tip1, Building* building, int hpc) const {
	double delta = 0.5;
    double h = tip1 - time;

	// collect all pedestrians in the simulation.
	const vector< Pedestrian* >& allPeds = building->GetAllPedestrians();

    unsigned int nSize = allPeds.size();
	int nThreads = omp_get_max_threads();

	// check if worth sharing the work
    if (nSize < 20) nThreads = 1;
    int partSize = nSize / nThreads;

    vector< Point > result_acc(nSize);

    //for gpu and xeonphi
    if(hpc!=0){
        //Buffer anlegen
        //Buffer fuer die Krafteinwirkung der Fussgaenger untereinander
        pedGetV_x=malloc(nSize*sizeof(double));
        pedGetV_y=malloc(nSize*sizeof(double));
        pedGetV0Norm=malloc(nSize*sizeof(double));
        pedGetID=malloc(nSize*sizeof(int));
        pedGetPos_x=malloc(nSize*sizeof(double));
        pedGetPos_y=malloc(nSize*sizeof(double));
        double cellSize=building->GetGrid()->GetCellSize();
        double gridXmin=building->GetGrid()->GetGridXmin();
        double gridYmin=building->GetGrid()->GetGridYmin();
        double maxEffDist=_distEffMaxPed;
        double widthPed=_intp_widthPed;
        double numPed=_nuPed;
        double maxf=_maxfPed;
        pedGetUniqueRoomID=malloc(nSize*sizeof(int));
        force_x=malloc(nSize*sizeof(double));
        force_y=malloc(nSize*sizeof(double));
        nearDoor=malloc(mSize*sizeof(int));
        elCenter_x=malloc(nSize*sizeof(double));
        elCenter_y=malloc(nSize*sizeof(double));
        sinPhi=malloc(nSize*sizeof(double));
        cosPhi=malloc(nSize*sizeof(double));
        elEA=malloc(nSize*sizeof(double));
        elEB=malloc(nSize*sizeof(double));
        elXp=malloc(nSize*sizeof(double));
        pedMass=malloc(nSize*sizeof(double));
        //Buffer anlegen fertig

        //Buffer fuellen
        for(unsigned int p=0;p<nSize;p++){
            Pedestrian* ped=allPeds[p];
            pedGetV_x[p]=ped->GetV().GetX();
            pedGetV_y[p]=ped->GetV().GetY();
            pedGetV0Norm[p]=ped->GetV0Norm();
            pedGetID[p]=ped->GetID();
            pedGetPos_x[p]=ped->GetPos().GetX();
            pedGetPos_y[p]=ped->GetPos().GetY();
            pedGetUniqueRoomID[p]=ped->GetUniqueRoomID();
            force_x[p]=0.0;
            force_y[p]=0.0;
            //ist der Fussgaenger naeher als 1 m an einer Tuer? wenn ja ID der Tuer in nearDoor speichern sonst nearDoor=-1
            Room* room = building->GetRoom(ped->GetRoomID());
            SubRoom* subroom = room->GetSubRoom(ped->GetSubRoomID());
            vector<Transition*> t=subroom->GetAllTransitions();
            nearDoor[p]=-1;
            for(int i=0;i<t.size();i++){
                Line* l = new Line(t[i]->GetPoint1(),t[i]->GetPoint2());
                if(l->DistTo(ped->GetPos())<1.0){
                    nearDoor[p]=t[i]->GetID();
                    break;
                }
            }
            elCenter_x[p]=ped->GetEllipse().GetCenter().GetX();
            elCenter_y[p]=ped->GetEllipse().GetCenter().GetY();
            cosPhi[p]=ped->GetEllipse().GetCosPhi();
            sinPhi[p]=ped->GetEllipse().GetSinPhi();
            elEA[p]=ped->GetEllipse().GetEA();
            elEB[p]=ped->GetEllipse().GetEB();
            elXp[p]=ped->GetEllipse().GetXp();
            pedMass[p]=ped->GetMass();
        }
        //Buffer fuellen fertig

        //Rechnung
        for(int p=0;p<nSize;p++){
            double normVi = pedGetV_x[p]*pedGetV_x[p]+pedGetV_y[p]*pedGetV_y[p];
            double tmp = (pedGetV0Norm[p]+delta)*(pedGetV0Norm[p]+delta);
            if(normVi>tmp && pedGetV0Norm[p]>0){
                fprintf(stderr, "GCFMModel::calculateForce() WARNING: actual velocity (%f) of iped %d "
                        "is bigger than desired velocity (%f) at time: %fs\n",
                        sqrt(normVi), pedGetID[p], pedGetV0Norm[p], time);
                exit(EXIT_FAILURE);
            }
            //Krafteinwirkung der Nachbarn berechnen
            //Nachbarn finden
            double xPed=pedGetPos_x[p];
            double yPed=pedGetPos_y[p];
            int xpos=(int)(xPed/cellSize);
            int ypos=(int)(yPed/cellSize);
            int myID=pedGetID[p]-1;
            for(int n=0;n<nSize;n++){
                if(n!=p){
                    if(pedGetPos_x[n]<=(xpos+1) && pedGetPos_x[n]>=(xpos-1)){
                       if(pedGetPos_y[n]<=(ypos+1) && pedGetPos_y[n]>=(ypos-1)){
                           //n liegt in der Nachbarschaft
                           double dist=((pedGetPos_x[n]-xPed)*(pedGetPos_x[n]-xPed) + (pedGetPos_y[n]-yPed)*(pedGetPos_y[n]-yPed));
                           if(dist<cellSize*cellSize){
                               //n ist nah genug an p, sodass die Kraefte berechnet werden koennen, falls sie im selben Raum sind
                               //oder beide in der Naehe der gleichen Tuer sind
                               if(pedGetUniqueRoomID[p]==pedGetUniqueRoomID[n] || nearDoor[p]==nearDoor[n]){
                                   //Kraft berechnen
                                   // x- and y-coordinate of the distance between p1 and p2
                                   double distp12_x = pedGetPos_x[n] - pedGetPos_x[p];
                                   double distp12_y = pedGetPos_y[n] - pedGetPos_y[p];
                                   //Distanz zwischen den Ellipsen
                                   double eff_dist;
                                   //E2inE1
                                   double tmp_x=elCenter_x[n]-elCenter_x[p];
                                   double tmp_y=elCenter_y[n]-elCenter_y[p];
                                   double E2inE1_x=tmp_x*cosPhi[p]-tmp_y*(-sinPhi[p]);
                                   double E2inE1_y=tmp_x*(-sinPhi[p])+tmp_y*cosPhi[p];
                                   //E1inE2
                                   tmp_x=elCenter_x[p]-elCenter_x[n];
                                   tmp_y=elCenter_y[p]-elCenter_y[n];
                                   double E1inE2_x=tmp_x*cosPhi[n]-tmp_y*(-sinPhi[n]);
                                   double E1inE2_y=tmp_x*(-sinPhi[n])+tmp_y*cosPhi[n];
                                   // distance between centers of E1 and E2
                                   double elDist=sqrt(tmp_x*tmp_x+tmp_y*tmp_y);
                                   //PointOnEllipse() R1 und R2
                                   double R1_x, R1_y, R2_x, R2_y;
                                   //R1
                                   double r=E2inE1_x*E2inE1_x+E2inE1_y*E2inE1_y;
                                   if(r<0.001*0.001){
                                       double cp_x=elEA[p];
                                       double cp_y=0.0;
                                       //cp.CoordTransToCart
                                       R1_x=cp_x*cosPhi[p]-cp_y*sinPhi[p]+elCenter_x[p];
                                       R1_y=cp_x*sinPhi[p]+cp_y*cosPhi[p]+elCenter_y[p];
                                   }
                                   else{
                                       r=sqrt(r);
                                       double cosTheta=x/r;
                                       double sinTheta=y/r;
                                       double a=elEA[p];
                                       double b=elEB[p];
                                       double s_x=a*cosTheta;
                                       double s_y=b*sinTheta;
                                       //s.CoordTransToCart
                                       R1_x=s_x*cosPhi[p]-s_y*sinPhi[p]+elCenter_x[p];
                                       R1_y=s_x*sinPhi[p]+s_y*cosPhi[p]+elCenter_y[p];
                                   }
                                   //R2
                                   double r=E1inE2_x*E1inE2_x+E1inE2_y*E1inE2_y;
                                   if(r<0.001*0.001){
                                       double cp_x=elEA[n];
                                       double cp_y=0.0;
                                       //cp.CoordTransToCart
                                       R2_x=cp_x*cosPhi[n]-cp_y*sinPhi[n]+elCenter_x[n];
                                       R2_y=cp_x*sinPhi[n]+cp_y*cosPhi[n]+elCenter_y[n];
                                   }
                                   else{
                                       r=sqrt(r);
                                       double cosTheta=x/r;
                                       double sinTheta=y/r;
                                       double a=elEA[n];
                                       double b=elEB[n];
                                       double s_x=a*cosTheta;
                                       double s_y=b*sinTheta;
                                       //s.CoordTransToCart
                                       R2_x=s_x*cosPhi[n]-s_y*sinPhi[n]+elCenter_x[n];
                                       R2_y=s_x*sinPhi[n]+s_y*cosPhi[n]+elCenter_y[n];
                                   }
                                   //effective distance
                                   double norm1=sqrt((elCenter_x[p]-R1_x) * (elCenter_x[p]-R1_x) + (elCenter_y[p]-R1_y) * (elCenter_y[p]-R1_y));
                                   double norm2=sqrt((elCenter_x[n]-R2_x) * (elCenter_x[n]-R2_x) + (elCenter_y[n]-R2_y) * (elCenter_y[n]-R2_y));
                                   eff_dist=elDist-norm1-norm2;
                                   // If the pedestrian is outside the cutoff distance, the force is zero.
                                   if(eff_dist >= maxEffDist) {
                                       force_x[p]=force_x[p]+0.0;
                                       force_y[p]=force_y[p]+0.0;
                                   }
                                   //sonst Kraft berechnen
                                   else{
                                       double p1_x=elXp[p]*cosPhi[p]+elCenter_x[p];
                                       double p1_y=elXp[p]*sinPhi[p]+elCenter_y[p];
                                       double p2_x=elXp[n]*cosPhi[n]+elCenter_x[n];
                                       double p2_y=elXp[n]*sinPhi[n]+elCenter_y[n];
                                       double ep12_x, ep12_y;
                                       distp12_x = p2_x - p1_x;
                                       distp12_y = p2_y - p1_y;

                                       //mindist = E1.MinimumDistanceToEllipse(E2); //ONE
                                       double mindist = 0.5; //for performance reasons, it is assumed that this distance is about 50 cm
                                       double dist_intpol_left = mindist + widthPed; // lower cut-off for Frep (modCFM)
                                       double dist_intpol_right = maxEffDist - widthPed; //upper cut-off for Frep (modCFM)
                                       double smax = mindist - widthPed; //max overlapping
                                       double norm=sqrt(distp12_x*distp12_x + distp12_y*distp12_y);
                                       double f = 0.0f, f1 = 0.0f; //function value and its derivative at the interpolation point'
                                       if ( norm >= 0.001) {
                                           ep12_x = distp12_x/norm;
                                           ep12_y = distp12_y/norm;
                                           // calculate the parameter (whatever dist is)
                                           double tmp = (pedGetV_x[p]-pedGetV_x[n])*ep12_x + (pedGetV_y[p]-pedGetV_y[n])*ep12_y; // < v_ij , e_ij >
                                           double v_ij = 0.5 * (tmp + fabs(tmp));
                                           double tmp2 = pedGetV_x[p]*ep12_x + pedGetV_y[p]*ep12_y; // < v_i , e_ij >
                                           double K_ij;
                                           //todo: runtime normsquare?
                                           if (sqrt(pedGetV_x[p]*pedGetV_x[p] + pedGetV_y[p]*pedGetV_y[p]) < J_EPS) { // if(norm(v_i)==0)
                                               K_ij = 0;
                                           } else {
                                               double bla = tmp2 + fabs(tmp2);
                                               K_ij = 0.25 * bla * bla / (pedGetV_x[p]*pedGetV_x[p] + pedGetV_y[p]*pedGetV_y[p]); //squared

                                               if (K_ij < 0.001 * 0.001) {
                                                   force_x[p]=force_x[p]+0.0;
                                                   force_y[p]=force_y[p]+0.0;
                                               }
                                           }
                                           double nom = numPed * pedGetV0Norm[p] + v_ij; // Nu: 0=CFM, 0.28=modifCFM;
                                           nom = nom * nom;


                                           K_ij = sqrt(K_ij);
                                           if (eff_dist <= smax) { //5
                                               f = -pedMass[p] * K_ij * nom / dist_intpol_left;
                                               force_x[p] = ep12_x * maxf * f;
                                               force_y[p] = ep12_y * maxf * f;
                                           }
                                           else{
                                               if (eff_dist >= dist_intpol_right) { //2
                                                   f = -pedMass[p] * K_ij * nom / dist_intpol_right; // abs(NR-Dv(i)+Sa)
                                                   f1 = -f / dist_intpol_right;
                                                   //hermite_interp
                                                   double t = eff_dist;
                                                   double x1 = dist_intpol_right;
                                                   double x2 = maxEffDist;
                                                   double y1 = f;
                                                   double y2 =0.0;
                                                   double dy1 = f1;
                                                   double dy2 = 0.0;
                                                   double scale = x2 - x1;
                                                   t = ( t - x1 ) / scale;
                                                   double t2 = t * t;
                                                   double t3 = t2 * t;
                                                   double h1 = 2 * t3 - 3 * t2 + 1;
                                                   double h2 = -2 * t3 + 3 * t2;
                                                   double h3 = t3 - 2 * t2 + t;
                                                   double h4 = t3 - t2;
                                                   double left = y1 * h1 + dy1 * h3 * scale;
                                                   double right = y2 * h2 + dy2 * h4 * scale;
                                                   double px = left + right; //hermite interpolation end
                                                   force_x[p] = ep12_x * px;
                                                   force_y[p] = ep12_y * px;
                                               }
                                               else if (eff_dist >= dist_intpol_left) { //3
                                                   f = -pedMass[p] * K_ij * nom / fabs(eff_dist); // abs(NR-Dv(i)+Sa)
                                                   force_x[p] = ep12_x * f;
                                                   force_y[p] = ep12_y * f;
                                               }
                                               else {//4
                                                   f = -pedMass[p]* K_ij * nom / dist_intpol_left;
                                                   f1 = -f / dist_intpol_left;
                                                   //hermite_interp
                                                   double t = eff_dist;
                                                   double x1 = smax;
                                                   double x2 = dist_intpol_left;
                                                   double y1 = maxf*f;
                                                   double y2 =f;
                                                   double dy1 = 0;
                                                   double dy2 = f1;
                                                   double scale = x2 - x1;
                                                   t = ( t - x1 ) / scale;
                                                   double t2 = t * t;
                                                   double t3 = t2 * t;
                                                   double h1 = 2 * t3 - 3 * t2 + 1;
                                                   double h2 = -2 * t3 + 3 * t2;
                                                   double h3 = t3 - 2 * t2 + t;
                                                   double h4 = t3 - t2;
                                                   double left = y1 * h1 + dy1 * h3 * scale;
                                                   double right = y2 * h2 + dy2 * h4 * scale;
                                                   double px = left + right; //hermite interpolation end
                                                   force_x[p] = ep12_x * px;
                                                   force_y[p] = ep12_y * px;
                                               }
                                           }
                                       }
                                   }
                               }
                           }
                       }
                    }
                }
            }
            //Krafteinwirkung der Nachbarn berechnet.


        }
    }
#pragma omp parallel  default(shared) num_threads(nThreads)
{

        //vector< Point > result_acc = vector<Point > ();
        //result_acc.reserve(2200);

        const int threadID = omp_get_thread_num();
        //cout << threadID << endl;
        //int start = threadID*partSize;
        //int end = (threadID + 1) * partSize - 1;
        //if ((threadID == nThreads - 1)) end = nSize - 1;
        //for (int p = start; p <= end; ++p) {

#pragma omp for
        for(int p=0;p<nSize;p++){
			Pedestrian* ped = allPeds[p];
			Room* room = building->GetRoom(ped->GetRoomID());
			SubRoom* subroom = room->GetSubRoom(ped->GetSubRoomID());
			double normVi = ped->GetV().ScalarP(ped->GetV());
			double tmp = (ped->GetV0Norm() + delta) * (ped->GetV0Norm() + delta);
			if (normVi > tmp && ped->GetV0Norm() > 0) {
				fprintf(stderr, "GCFMModel::calculateForce() WARNING: actual velocity (%f) of iped %d "
						"is bigger than desired velocity (%f) at time: %fs\n",
                        sqrt(normVi), ped->GetID(), ped->GetV0Norm(), time);////time statt t

				// remove the pedestrian and abort
				for(int p=0;p<subroom->GetNumberOfPedestrians();p++){
					if (subroom->GetPedestrian(p)->GetID()==ped->GetID()){
						subroom->DeletePedestrian(p);
						break;
					}
				}

				building->DeletePedestrian(ped);
				Log->Write("\tCRITICAL: one ped was removed due to high velocity");

				//	continue;
				exit(EXIT_FAILURE);
			}
            //cout << threadID << ": nach der if." << endl;
			Point F_rep;
			vector<Pedestrian*> neighbours;
			building->GetGrid()->GetNeighbourhood(ped,neighbours);
            int NSize=neighbours.size();
            for (int i = 0; i < NSize; i++) {
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
			}

			//repulsive forces to the walls and transitions that are not my target
			Point repwall = ForceRepRoom(allPeds[p], subroom);

			Point acc = (ForceDriv(ped, room) + F_rep + repwall) / ped->GetMass();
            //result_acc.push_back(acc);
           // cout << threadID << " " << result_acc.begin()+2 << ": " << nSize << endl;

            result_acc[p]=acc;
		}
        //cout << threadID << ": erste for geschafft." << endl;
#pragma omp barrier
		// update
        //for (int p = start; p <= end; ++p) {
#pragma omp for
        for (int p = 0; p < nSize; p++) {
			Pedestrian* ped = allPeds[p];
            //Point v_neu = ped->GetV() + result_acc[p - start] * h;
            Point v_neu = ped->GetV() + result_acc[p] * h;
			Point pos_neu = ped->GetPos() + v_neu * h;

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
    //cout << "openmp endet" << endl;
}
