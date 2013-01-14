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

#ifdef _OPENMP
#include <omp.h>
#endif

using namespace std;

#include "../IO/OutputHandler.h"
#include "../geometry/Building.h"
#include "../routing/DirectionStrategy.h"
#include "../mpi/LCGrid.h"

extern OutputHandler* Log;

/************************************************************
 ForceModel (abstrakte Klasse)
 ************************************************************/

class ForceModel {

public:
    // Konstruktoren
    ForceModel();
    virtual ~ForceModel();
    // virtuelle Funktionen (werden in den abgeleiteten Klassen implementiert)
    //FIXME: remove
    virtual void CalculateForce(double time, vector< Point >& result_acc, Building* building, int roomID, int SubRoomID) const = 0;
    virtual void CalculateForceLC(double t, double tp, Building* building) const = 0;
    virtual string writeParameter() const = 0;
};

/************************************************************
 GCFM ForceModel
 ************************************************************/

class GCFMModel : public ForceModel {
private:
    DirectionStrategy* pdirection; // Strategie zur Richtungswahl, wird in Fdriv benötigt
    // Modellparameter
    double pNuPed;
    double pNuWall;
    double pintp_widthPed; // Interpolation cutoff radius (in cm)
    double pintp_widthWall; // Interpolation cutoff radius (in cm)
    double pmaxfPed;
    double pmaxfWall;
    double pDistEffMaxPed; // maximal effective distance
    double pDistEffMaxWall; // maximal effective distance

    // Private Funktionen
    inline Point ForceDriv(Pedestrian* ped, Room* room) const;
    Point ForceRepPed(Pedestrian* ped1, Pedestrian* ped2) const;
    inline Point ForceRepRoom(Pedestrian* ped, SubRoom* subroom) const;
    inline Point ForceRepWall(Pedestrian* ped, const Wall& l) const;
    Point ForceRepStatPoint(Pedestrian* ped, const Point& p, double l, double vn) const;
    Point ForceInterpolation(double v0, double K_ij, const Point& e, double v, double d, double r, double l) const;
public:
    // Konstruktoren
    GCFMModel(DirectionStrategy* dir, double nuped, double nuwall, double dist_effPed, double dist_effWall,
            double intp_widthped, double intp_widthwall, double maxfped, double maxfwall);
    GCFMModel(const GCFMModel& orig);
    virtual ~GCFMModel(void);

    // Getter-Funktionen
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
    virtual void CalculateForce(double time, vector< Point >& result_acc, Building* building,
    int roomID, int SubRoomID) const;
    virtual void CalculateForceLC(double t, double tp, Building* building) const;
    virtual string writeParameter() const;
};


#endif	/* _FORCEMODEL_H */

