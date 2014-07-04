/*
 * AgentsParameters.h
 *
 *  Created on: 03.07.2014
 *      Author: piccolo
 */

#ifndef AGENTSPARAMETERS_H_
#define AGENTSPARAMETERS_H_

#include <random>

class AgentsParameters {
public:
    /**
     * Constructor
     */
    AgentsParameters(int id, int seed=1234);

    /**
     * Default copy constructor
     * @param orig, the object to copy
     */
    AgentsParameters(AgentsParameters const& orig );

    /**
     * Destructor
     */
    virtual ~AgentsParameters();

    /**
     * @return the ID of the agents parameters sets.
     */
    int GetID();

    /**
     * Set the ID of the parameter set
     * @param id
     */
    void SetID(int id);//not implemented

    /**
     * Initialize the desired velocity distribution
     * @param mean, mean value
     * @param stv, standard deviation
     */
    void InitV0(double mean, double stv);

    /**
     * Initialize the maximal value if the major axis
     * @param mean, mean value
     * @param stv, standard deviation
     */
    void InitBmax(double mean, double stv);

    /**
     * Initialize the minimal value if the major axis
     * @param mean, mean value
     * @param stv, standard deviation
     */
    void InitBmin(double mean, double stv);

    /**
     * Initialize the minimal value of the minor axis
     * @param mean, mean value
     * @param stv, standard deviation
     */
    void InitAmin(double mean, double stv);

    /**
     * Initialize the reaction time
     * @param mean, mean value
     * @param stv, standard deviation
     */
    void InitAtau(double mean, double stv);

    /**
     * Initialize the reaction time
     * @param mean, mean value
     * @param stv, standard deviation
     */
    void InitTau(double mean, double stv);

    /**
     * @return a random number following the distribution
     */
    double GetV0();

    /**
     * @return a random number following the distribution
     */
    double GetBmax();

    /**
     * @return a random number following the distribution
     */
    double GetBmin();

    /**
     * @return a random number following the distribution
     */
    double GetAtau();

    /**
     * @return a random number following the distribution
     */
    double GetAmin();

    /**
     * @return a random number following the distribution
     */
    double GetTau();

private:
    int _id;
    //std::random_device _rd;
    std::default_random_engine _generator;
    std::normal_distribution<double> _V0;
    std::normal_distribution<double> _Bmax;
    std::normal_distribution<double> _Bmin;
    std::normal_distribution<double> _Atau;
    std::normal_distribution<double> _Amin;
    std::normal_distribution<double> _Tau;

};

#endif /* AGENTSPARAMETERS_H_ */
