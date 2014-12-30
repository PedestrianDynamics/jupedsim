/**
 * @file GCFMModel.cpp
 *
 * @brief Implementation of classes for some force-based models
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
 * Implementation of classes for force-based models.
 * Actually we've got two different models:
 * 1. Generalized Centrifugal Force Model
 *
 * @date Tue Apr 15 19:19:04 2014
 */

#include "GPU_GCFMModel.h"
#include "../pedestrian/Pedestrian.h"
#include "../routing/DirectionStrategy.h"
#include "../mpi/LCGrid.h"
#include "../geometry/SubRoom.h"
#include "../geometry/Wall.h"

#ifdef _OPENMP
#include <omp.h>
#else
#define omp_get_thread_num() 0
#define omp_get_max_threads()  1
#endif


using std::vector;
using std::string;

GPU_GCFMModel::GPU_GCFMModel(DirectionStrategy* dir, double nuped, double nuwall, double dist_effPed,
		double dist_effWall, double intp_widthped, double intp_widthwall, double maxfped,
		double maxfwall)
{
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

GPU_GCFMModel::~GPU_GCFMModel(void)
{

}

inline  Point GPU_GCFMModel::ForceDriv(Pedestrian* ped, Room* room) const
{
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

Point GPU_GCFMModel::ForceRepPed(Pedestrian* ped1, Pedestrian* ped2) const
{
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
		exit(EXIT_FAILURE);

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
		char msg[CLENGTH];
		sprintf(msg, "\nNAN return ----> p1=%d p2=%d Frepx=%f, Frepy=%f\n", ped1->GetID(),
				ped2->GetID(), F_rep.GetX(), F_rep.GetY());
		Log->Write(msg);
		Log->Write("ERROR:\t fix this as soon as possible");
		return Point(0,0); // FIXME: should never happen
		exit(EXIT_FAILURE);
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

inline Point GPU_GCFMModel::ForceRepRoom(Pedestrian* ped, SubRoom* subroom) const
{
	 unsigned int i;
	Point f = Point(0., 0.);
	//first the walls

	const vector<Wall>& walls = subroom->GetAllWalls();
	for (i = 0; i < subroom->GetNumberOfWalls(); i++) {
		f += ForceRepWall(ped, walls[i]);
	}

	//then the obstacles
	const vector<Obstacle*>& obstacles = subroom->GetAllObstacles();
	for(unsigned int obs=0; obs<obstacles.size(); ++obs) {
		const vector<Wall>&owalls = obstacles[obs]->GetAllWalls();
		for (i = 0; i < owalls.size(); i++) {
			f += ForceRepWall(ped, owalls[i]);
		}
	}

	//eventually crossings
	const vector<Crossing*>& crossings = subroom->GetAllCrossings();
	for (i = 0; i < crossings.size(); i++) {
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
	for (i = 0; i < transitions.size(); i++) {
		Transition* goal=transitions[i];
		int uid1= goal->GetUniqueID();
		int uid2=ped->GetExitIndex();
		// ignore my transition consider closed doors
		//closed doors are considered as wall

		if((uid1 != uid2) || !goal->IsOpen()) {
			f += ForceRepWall(ped,*((Wall*)goal));
		}
	}

	return f;
}


inline Point GPU_GCFMModel::ForceRepWall(Pedestrian* ped, const Wall& w) const
{
	Point F = Point(0.0, 0.0);
	Point pt = w.ShortestPoint(ped->GetPos());
	double wlen = w.LengthSquare();

	if (wlen < 0.01) { // ignore walls smaller than 10 cm
		return F;
	}
	// Kraft soll nur orthgonal wirken
	// ???
	if (fabs((w.GetPoint1() - w.GetPoint2()).ScalarP(ped->GetPos() - pt)) > J_EPS) {
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
Point GPU_GCFMModel::ForceRepStatPoint(Pedestrian* ped, const Point& p, double l, double vn) const
{
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

Point GPU_GCFMModel::ForceInterpolation(double v0, double K_ij, const Point& e, double vn, double d, double r, double l) const
{
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





// Getter-Funktionen

DirectionStrategy* GPU_GCFMModel::GetDirection() const
{
	return _direction;
}

double GPU_GCFMModel::GetNuPed() const
{
	return _nuPed;
}

double GPU_GCFMModel::GetNuWall() const
{
	return _nuWall;
}

double GPU_GCFMModel::GetIntpWidthPed() const
{
	return _intp_widthPed;
}

double GPU_GCFMModel::GetIntpWidthWall() const
{
	return _intp_widthWall;
}

double GPU_GCFMModel::GetMaxFPed() const
{
	return _maxfPed;
}

double GPU_GCFMModel::GetMaxFWall() const
{
	return _maxfWall;
}

double GPU_GCFMModel::GetDistEffMaxPed() const
{
	return _distEffMaxPed;
}

double GPU_GCFMModel::GetDistEffMaxWall() const
{
	return _distEffMaxWall;
}

string GPU_GCFMModel::writeParameter() const
{
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

void GPU_GCFMModel::SetHPC(int f){
	hpc=f;
}

void GPU_GCFMModel::CreateBuffer(int numPeds){
	//Buffer anlegen
	std::cout << numPeds << std::endl;
	//Buffer fuer die Krafteinwirkung der Fussgaenger untereinander
	pedGetV_x=(double*)malloc(numPeds *sizeof(double));
	pedGetV_y=(double*)malloc(numPeds *sizeof(double));
	pedGetV0Norm=(double*)malloc(numPeds *sizeof(double));
	pedGetID=(int*)malloc(numPeds *sizeof(int));
	pedGetPos_x=(double*)malloc(numPeds *sizeof(double));
	pedGetPos_y=(double*)malloc(numPeds *sizeof(double));
	pedGetUniqueRoomID=(int*)malloc(numPeds *sizeof(int));
	force_x=(double*)malloc(numPeds *sizeof(double));
	force_y=(double*)malloc(numPeds *sizeof(double));
	nearDoor=(int*)malloc(numPeds *sizeof(int));
	elCenter_x=(double*)malloc(numPeds *sizeof(double));
	elCenter_y=(double*)malloc(numPeds *sizeof(double));
	sinPhi=(double*)malloc(numPeds *sizeof(double));
	cosPhi=(double*)malloc(numPeds *sizeof(double));
	elEA=(double*)malloc(numPeds *sizeof(double));
	elEB=(double*)malloc(numPeds *sizeof(double));
	elXp=(double*)malloc(numPeds *sizeof(double));
	pedMass=(double*)malloc(numPeds *sizeof(double));
	//zusaetzliche Buffer fuer die anziehende Kraft des Ziels
	forceDriv_x=(double*)malloc(numPeds *sizeof(double));
	forceDriv_y=(double*)malloc(numPeds *sizeof(double));
	distToExitLine=(double*)malloc(numPeds *sizeof(double));
	targetV0_x=(double*)malloc(numPeds *sizeof(double));
	targetV0_y=(double*)malloc(numPeds *sizeof(double));
	pedTau=(double*)malloc(numPeds *sizeof(double));
	pedV0_x=(double*)malloc(numPeds *sizeof(double));
	pedV0_y=(double*)malloc(numPeds *sizeof(double));
	//Buffer fuer repwall
	forceWall_x=(double*)malloc(numPeds *sizeof(double));
	forceWall_y=(double*)malloc(numPeds *sizeof(double));
	//Buffer anlegen fertig
}

void GPU_GCFMModel::DeleteBuffers(){
	free(pedGetV_x);
	free(pedGetV_y);
	free(pedGetV0Norm);
	free(pedGetID);
	free(pedGetPos_x);
	free(pedGetPos_y);
	free(pedGetUniqueRoomID);
	free(force_x);
	free(force_y);
	free(nearDoor);
	free(elCenter_x);
	free(elCenter_y);
	free(sinPhi);
	free(cosPhi);
	free(elEA);
	free(elEB);
	free(elXp);
	free(pedMass);
	free(forceDriv_x);
	free(forceDriv_y);
	free(distToExitLine);
	free(targetV0_x);
	free(targetV0_y);
	free(pedTau);
	free(pedV0_x);
	free(pedV0_y);
	free(forceWall_x);
	free(forceWall_y);
}

/**
 * implementation of Linked-cell combined with openMP
 */

void GPU_GCFMModel::CalculateForce(double time, double tip1, Building* building) const
{
	double delta = 0.5;
	double h = tip1 - time;

	// collect all pedestrians in the simulation.
	const vector< Pedestrian* >& allPeds = building->GetAllPedestrians();
	unsigned int nSize = allPeds.size();

	int nThreads = omp_get_max_threads();
	double cellSize=building->GetGrid()->GetCellSize();
	// check if worth sharing the work
	if (nSize < 20) nThreads = 1;
	//int partSize = nSize / nThreads;

	vector< Point > result_acc(nSize);

	//for gpu and xeonphi
	if(hpc!=0){
#pragma omp parallel num_threads(nThreads)
		{
			//Buffer fuellen
#pragma omp for
			for(int p=0; p<(int)nSize; p++){
				Pedestrian* ped=allPeds[p];
				pedGetV_x[p]=ped->GetV().GetX();
				pedGetV_y[p]=ped->GetV().GetY();
				pedGetV0Norm[p]=ped->GetV0Norm();//
				pedGetID[p]=ped->GetID();
				pedGetPos_x[p]=ped->GetPos().GetX();
				pedGetPos_y[p]=ped->GetPos().GetY();
				pedGetUniqueRoomID[p]=ped->GetUniqueRoomID();//
				force_x[p]=0.0;
				force_y[p]=0.0;
				Room* room = building->GetRoom(ped->GetRoomID());
				elCenter_x[p]=ped->GetPos().GetX();
				elCenter_y[p]=ped->GetPos().GetY();
				cosPhi[p]=ped->GetEllipse().GetCosPhi();
				sinPhi[p]=ped->GetEllipse().GetSinPhi();
				elEA[p]=ped->GetEllipse().GetEA();//
				elEB[p]=ped->GetEllipse().GetEB();//
				elXp[p]=ped->GetEllipse().GetXp();//
				pedMass[p]=ped->GetMass();
				//vector<Transition*> t=room->GetSubRoom(ped->GetSubRoomID())->GetAllTransitions();
				//nearDoor[p]=-1;//
				//for(unsigned int i=0;i<t.size();i++){
				//  Line* l = new Line(t[i]->GetPoint1(),t[i]->GetPoint2());
				//if(l->DistTo(ped->GetPos())<1.0){
				// nearDoor[p]=t[i]->GetID();
				//break;
				//}
				//}
				//forceDriv_x[p]=0.0;
				//forceDriv_y[p]=0.0;
				//forceWall_x[p]=0.0;
				//forceWall_y[p]=0.0;
				distToExitLine[p]=ped->GetExitLine()->DistTo(ped->GetPos());//
				const Point& target = _direction->GetTarget(room, ped);//
				targetV0_x[p]=ped->GetV0(target).GetX();//
				targetV0_y[p]=ped->GetV0(target).GetY();//
				pedV0_x[p]=ped->GetV0().GetX();
				pedV0_y[p]=ped->GetV0().GetY();
				pedTau[p]=ped->GetTau();
			}
#pragma omp for
			for(int p=0; p<(int)nSize; p++){
				double pedV_x=pedGetV_x[p];
				double pedV_y=pedGetV_y[p];
				double pedV0Norm=pedGetV0Norm[p];
				double xPed=elCenter_x[p];
				double yPed=elCenter_y[p];
				double normVi = pedV_x*pedV_x + pedV_y*pedV_y;
				double v0Threshold = (pedV0Norm+delta) * (pedV0Norm+delta);
				if(normVi> v0Threshold && pedV0Norm>0){
					fprintf(stderr, "GCFMModel::calculateForce() WARNING: actual velocity (%f) of iped %d "
							"is bigger than desired velocity (%f) at time: %fs\n",
							sqrt(normVi), pedGetID[p], pedV0Norm, time);
					//exit(EXIT_FAILURE);
				}

				//Kraefte der Fussgaenger untereinander
				//Nachbarn finden
				for(unsigned int n=0; n<nSize; n++){
					if((int)n != p){
						if(pedGetUniqueRoomID[p]==pedGetUniqueRoomID[n]){
							//Pedestrian* ped1=allPeds[n];
							//F_rep = F_rep + ForceRepPed(ped, ped1);
							double nPed_x=elCenter_x[n];
							double nPed_y=elCenter_y[n];
							double pCosPhi=cosPhi[p];
							double pSinPhi=sinPhi[p];
							double dist=(nPed_x-xPed)*(nPed_x-xPed) + (nPed_y-yPed)*(nPed_y-yPed);
							if(dist<cellSize*cellSize){
								//n liegt in der Nachbarschaft
								//Kraft berechnen
								//Pedestrian* ped1=allPeds[n];
								//F_rep = F_rep + ForceRepPed(ped, ped1);
								// x- and y-coordinate of the distance between p1 and p2
								double distp12_x = nPed_x - xPed;
								double distp12_y = nPed_y - yPed;
								//Distanz zwischen den Ellipsen
								double eff_dist;
								//E2inE1
								double tmp_x=nPed_x-xPed;
								double tmp_y=nPed_y-yPed;
								double E2inE1_x=tmp_x*pCosPhi-tmp_y*(-pSinPhi);
								double E2inE1_y=tmp_x*(-pSinPhi)+tmp_y*pCosPhi;
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
									double cosTheta=E2inE1_x/r;
									double sinTheta=E2inE1_y/r;
									double a=elEA[p];
									double b=elEB[p];
									double s_x=a*cosTheta;
									double s_y=b*sinTheta;
									//s.CoordTransToCart
									R1_x=s_x*cosPhi[p]-s_y*sinPhi[p]+elCenter_x[p];
									R1_y=s_x*sinPhi[p]+s_y*cosPhi[p]+elCenter_y[p];
								}
								//R2
								r=E1inE2_x*E1inE2_x+E1inE2_y*E1inE2_y;
								if(r<0.001*0.001){
									double cp_x=elEA[n];
									double cp_y=0.0;
									//cp.CoordTransToCart
									R2_x=cp_x*cosPhi[n]-cp_y*sinPhi[n]+elCenter_x[n];
									R2_y=cp_x*sinPhi[n]+cp_y*cosPhi[n]+elCenter_y[n];
								}
								else{
									r=sqrt(r);
									double cosTheta=E1inE2_x/r;
									double sinTheta=E1inE2_y/r;
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
								if(eff_dist >= _distEffMaxPed) {
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
									double dist_intpol_left = mindist + _intp_widthPed; // lower cut-off for Frep (modCFM)
									double dist_intpol_right = _distEffMaxPed - _intp_widthPed; //upper cut-off for Frep (modCFM)
									double smax = mindist - _intp_widthPed; //max overlapping
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
										double nom = _nuPed * pedGetV0Norm[p] + v_ij; // Nu: 0=CFM, 0.28=modifCFM;
										nom = nom * nom;

										K_ij = sqrt(K_ij);
										if (eff_dist <= smax) { //5
											f = -pedMass[p] * K_ij * nom / dist_intpol_left;
											force_x[p] = force_x[p]+(ep12_x * _maxfPed * f);
											force_y[p] = force_y[p]+(ep12_y * _maxfPed * f);
										}
										else{
											if (eff_dist >= dist_intpol_right) { //2
												f = -pedMass[p] * K_ij * nom / dist_intpol_right; // abs(NR-Dv(i)+Sa)
												f1 = -f / dist_intpol_right;
												//hermite_interp
												double t = eff_dist;
												double x1 = dist_intpol_right;
												double x2 = _distEffMaxPed;
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
												force_x[p] = force_x[p]+(ep12_x * px);
												force_y[p] = force_y[p]+(ep12_y * px);
											}
											else if (eff_dist >= dist_intpol_left) { //3
												f = -pedMass[p] * K_ij * nom / fabs(eff_dist); // abs(NR-Dv(i)+Sa)
												force_x[p] = force_x[p]+(ep12_x * f);
												force_y[p] = force_y[p]+(ep12_y * f);
											}
											else {//4
												f = -pedMass[p]* K_ij * nom / dist_intpol_left;
												f1 = -f / dist_intpol_left;
												//hermite_interp
												double t = eff_dist;
												double x1 = smax;
												double x2 = dist_intpol_left;
												double y1 = _maxfPed*f;
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
												force_x[p] = force_x[p]+(ep12_x * px);
												force_y[p] = force_y[p]+(ep12_y * px);
											}
										}
									}
								}
							}
						}
					}
				}
				//force_x[p]=F_rep.GetX();
				//force_y[p]=F_rep.GetY();

				//repulsive forces to the walls and transitions that are not my target
				//Pedestrian* ped=allPeds[p];
				//F_rep=Point(force_x[p],force_y[p]);
				//Room* room=building->GetRoom(ped->GetRoomID());
				//SubRoom* subroom=room->GetSubRoom(ped->GetSubRoomID());
				//Point repwall = ForceRepRoom(allPeds[p], subroom);

				//Point acc = (ForceDriv(ped, room) + F_rep + repwall) / ped->GetMass();
				//Anziehende Kraft des Ziels berechnen
				if (distToExitLine[p] > 0.005) {
					forceDriv_x[p]=(targetV0_x[p]*pedGetV0Norm[p] - pedGetV_x[p]*pedMass[p])/pedTau[p];
					forceDriv_y[p]=(targetV0_y[p]*pedGetV0Norm[p] - pedGetV_y[p]*pedMass[p])/pedTau[p];
				} else {
					forceDriv_x[p]=(pedV0_x[p]*pedGetV0Norm[p] - pedGetV_x[p]*pedMass[p])/pedTau[p];
					forceDriv_y[p]=(pedV0_y[p]*pedGetV0Norm[p] - pedGetV_y[p]*pedMass[p])/pedTau[p];
				}


				//result_acc[p]=acc;
			}
#pragma omp for
			for(int p=0; p<(int)nSize; p++){
				Pedestrian* ped = allPeds[p];
				Room* room = building->GetRoom(ped->GetRoomID());
				SubRoom* subroom = room->GetSubRoom(ped->GetSubRoomID());
				Point repwall = ForceRepRoom(allPeds[p], subroom);
				forceWall_x[p]=repwall.GetX();
				forceWall_y[p]=repwall.GetY();
				//Point F_driv=ForceDriv(ped, room);
				//forceDriv_x[p]=F_driv.GetX();
				//forceDriv_y[p]=F_driv.GetY();
			}
#pragma omp barrier
			// update
			//for (int p = start; p <= end; ++p) {
#pragma omp for
			for (int p = 0; p < (int)nSize; p++) {
				Pedestrian* ped = allPeds[p];
				Point F_rep=Point(force_x[p],force_y[p]);
				Point F_driv=Point(forceDriv_x[p],forceDriv_y[p]);
				//SubRoom* subroom = building->GetRoom(ped->GetRoomID())->GetSubRoom(ped->GetSubRoomID());
				Point repwall = Point(forceWall_x[p],forceWall_y[p]);
				Point acc = (F_driv + F_rep + repwall) / ped->GetMass();
				//Point acc=result_acc[p];
				//Point v_neu = ped->GetV() + result_acc[p - start] * h;
				Point v_neu = ped->GetV() + acc * h;
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
		}
	}//end parallel
	else{
#pragma omp parallel  default(shared) num_threads(nThreads)
		{

			//vector< Point > result_acc = vector<Point > ();
			//result_acc.reserve(2200);

			//const int threadID = omp_get_thread_num();
			//cout << threadID << endl;
			//int start = threadID*partSize;
			//int end = (threadID + 1) * partSize - 1;
			//if ((threadID == nThreads - 1)) end = nSize - 1;
			//for (int p = start; p <= end; ++p) {

#pragma omp for
			for(int p=0; p<(int)nSize; p++){
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
					building->DeletePedestrian(ped);
					Log->Write("\tERROR: one ped was removed due to high velocity");

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
			for (int p = 0; p < (int)nSize; p++) {
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


}
