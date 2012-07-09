/* 
 * File:   ODESolver.h
 * Author: andrea
 *
 * Created on 17. August 2010, 15:31
 */

#ifndef _ODESOLVER_H
#define	_ODESOLVER_H

#include "ForceModel.h"
#include "../geometry/Building.h"

#include <vector>
using namespace std;

class ODESolver {
protected:
    ForceModel *model;
public:
    ODESolver(ForceModel* model);
    virtual void solveODE(double t, double tp, Building* building) const = 0;
};

// Euler Löser für die Differentialgleichung

class EulerSolver : public ODESolver {
public:
    EulerSolver(ForceModel *model);
    virtual void solveODE(double t, double tp, Building* building) const;
};

// Velocity Verlet Löser für die Differentialgleichung

class VelocityVerletSolver : public ODESolver {
public:
    VelocityVerletSolver(ForceModel *model);
    virtual void solveODE(double t, double tp, Building* building) const;
};

// Leapfrog Löser für die Differentialgleichung

class LeapfrogSolver : public ODESolver {
public:
    LeapfrogSolver(ForceModel *model);
    virtual void solveODE(double t, double tp, Building* building) const;
};

// für Linked Cell (Ulrich)

class EulerSolverLC : public ODESolver {
public:
    EulerSolverLC(ForceModel *model);
    virtual void solveODE(double t, double tp, Building* building) const;
};



#endif	/* _ODESOLVER_H */

