/**
 * @file GCFMModel.h
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


#ifndef GPU_GCFMMODEL_H_
#define GPU_GCFMMODEL_H_

#include <vector>

#include "../geometry/Building.h"
#include "ForceModel.h"



//forward declaration
class Pedestrian;
class DirectionStrategy;


class GPU_GCFMModel : public ForceModel {

public:

	GPU_GCFMModel(DirectionStrategy* dir, double nuped, double nuwall, double dist_effPed, double dist_effWall,
			double intp_widthped, double intp_widthwall, double maxfped, double maxfwall);
	virtual ~GPU_GCFMModel(void);

	// Getter
	DirectionStrategy* GetDirection() const;
	double GetNuPed() const;
	double GetNuWall() const;
	double GetDistEffMax() const;
	double GetIntpWidthPed() const;
	double GetIntpWidthWall() const;
	double GetMaxFPed() const;
	double GetMaxFWall() const;
	double GetDistEffMaxPed() const;
	double GetDistEffMaxWall() const;

	// virtual function
	virtual void CalculateForce(double t, double tp, Building* building) const;
	virtual std::string writeParameter() const;

	/**
	 *Create and fill Buffers for accelerators
	 **/
	void CreateBuffer(int numPeds);
	void DeleteBuffers();
	void deletePed(int id);
	void SetHPC(int f);

private:
	/// define the strategy for crossing a door (used for calculating the driving force)
	DirectionStrategy* _direction;
	// Modellparameter
	double _nuPed;                /**< strength of the pedestrian repulsive force */
	double _nuWall;               /**< strength of the wall repulsive force */
	double _intp_widthPed; /**< Interpolation cutoff radius (in cm) */
	double _intp_widthWall; /**< Interpolation cutoff radius (in cm) */
	double _maxfPed;
	double _maxfWall;
	double _distEffMaxPed; // maximal effective distance
	double _distEffMaxWall; // maximal effective distance

	//buffers for gpu and xeonphi
	//Buffer fuer die Krafteinwirkung der Fussgaenger untereinander
	double* pedGetV_x;
	double* pedGetV_y;
	double* pedGetV0Norm;
	int* pedGetID;
	double* pedGetPos_x;
	double* pedGetPos_y;
	//double gridXmin=building->GetGrid()->GetGridXmin();
	//double gridYmin=building->GetGrid()->GetGridYmin();
	int* pedGetUniqueRoomID;
	double* force_x;
	double* force_y;
	int* nearDoor;
	double* elCenter_x;
	double* elCenter_y;
	double* sinPhi;
	double* cosPhi;
	double* elEA;
	double* elEB;
	double* elXp;
	double* pedMass;
	int* flag; //1 = Ped noh dabei; 0 = Ped nicht mehr dabei
	//zusaetzliche Buffer fuer die anziehende Kraft des Ziels
	double* forceDriv_x;
	double* forceDriv_y;
	double* distToExitLine;
	double* targetV0_x;
	double* targetV0_y;
	double* pedTau;
	double* pedV0_x;
	double* pedV0_y;
	//Buffer fuer repwall
	double* forceWall_x;
	double* forceWall_y;
	int nrPeds;
	int hpc;

private:

	/**
	 * Driving force \f$ F_i =\frac{\mathbf{v_0}-\mathbf{v_i}}{\tau}\f$
	 *
	 * @param ped Pointer to Pedestrians
	 * @param room Pointer to Room
	 *
	 * @return Point
	 */
	Point ForceDriv(Pedestrian* ped, Room* room) const;
	/**
	 * Repulsive force between two pedestrians ped1 and ped2 according to
	 * the Generalized Centrifugal Force Model (chraibi2010a)
	 *
	 * @param ped1 Pointer to Pedestrian: First pedestrian
	 * @param ped2 Pointer to Pedestrian: Second pedestrian
	 *
	 * @return Point
	 */
	Point ForceRepPed(Pedestrian* ped1, Pedestrian* ped2) const;
	/**
	 * Repulsive force acting on pedestrian <ped> from the walls in
	 * <subroom>. The sum of all repulsive forces of the walls in <subroom> is calculated
	 * @see ForceRepWall
	 * @param ped Pointer to Pedestrian
	 * @param subroom Pointer to SubRoom
	 *
	 * @return
	 */
	Point ForceRepRoom(Pedestrian* ped, SubRoom* subroom) const;
	Point ForceRepWall(Pedestrian* ped, const Wall& l) const;
	Point ForceRepStatPoint(Pedestrian* ped, const Point& p, double l, double vn) const;
	Point ForceInterpolation(double v0, double K_ij, const Point& e, double v, double d, double r, double l) const;

};


#endif /* GPU_GCFMMODEL_H_ */
