/**
 * File:   ForceModel.h
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

#ifndef _FORCEMODEL_H
#define	_FORCEMODEL_H

#include <vector>

#include "../geometry/Building.h"



class Pedestrian;
class DirectionStrategy;



class ForceModel {

public:
    // Konstruktoren
    ForceModel();
    virtual ~ForceModel();

    //FIXME: remove
    virtual void CalculateForce(double time, std::vector< Point >& result_acc, Building* building, int roomID, int SubRoomID) const = 0;

    /**
     * Solve the differential equations and update the positions and velocities
     * @param t the actual time
     * @param tp the next timestep
     * @param building the geometry object
     * @param hpc the hpc architecture
     */
    virtual void CalculateForceLC(double t, double tp, Building* building, int hpc) const = 0;

    /**
     * @return all model parameters in a nicely formatted string
     */
    virtual std::string writeParameter() const = 0;
};

/************************************************************
 GCFM ForceModel
 ************************************************************/

class GCFMModel : public ForceModel {
private:
    /// define the strategy for crossing a door (used for calculating the driving foce)
	DirectionStrategy* _direction;
    // Modellparameter
    double _nuPed;
    double _nuWall;
    double _intp_widthPed; // Interpolation cutoff radius (in cm)
    double _intp_widthWall; // Interpolation cutoff radius (in cm)
    double _maxfPed;
    double _maxfWall;
    double _distEffMaxPed; // maximal effective distance
    double _distEffMaxWall; // maximal effective distance

    //buffers for gpu and xeonphi
    double* pedGetV_x;
    double* pedGetV_y;
    double* pedMass;
    double* pedGetV0Norm;
    int* pedGetID;
    double* pedGetPos_x;
    double* pedGetPos_y;
    int* pedGetUniqueRoomID;
    double* force_x;
    double* force_y;
    double* elCenter_x;
    double* elCenter_y;
    double* cosPhi;
    double* sinPhi;
    int* nearDoor;
    // Private Funktionen
    Point ForceDriv(Pedestrian* ped, Room* room) const;
    Point ForceRepPed(Pedestrian* ped1, Pedestrian* ped2) const;
    Point ForceRepRoom(Pedestrian* ped, SubRoom* subroom) const;
    Point ForceRepWall(Pedestrian* ped, const Wall& l) const;
    Point ForceRepStatPoint(Pedestrian* ped, const Point& p, double l, double vn) const;
    Point ForceInterpolation(double v0, double K_ij, const Point& e, double v, double d, double r, double l) const;
public:

    GCFMModel(DirectionStrategy* dir, double nuped, double nuwall, double dist_effPed, double dist_effWall,
            double intp_widthped, double intp_widthwall, double maxfped, double maxfwall);
    virtual ~GCFMModel(void);

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



    // virtuelle Funktionen
    virtual void CalculateForce(double time, std::vector< Point >& result_acc, Building* building,
    int roomID, int SubRoomID) const;
    virtual void CalculateForceLC(double t, double tp, Building* building, int hpc) const;
    virtual std::string writeParameter() const;
};


#endif	/* _FORCEMODEL_H */

