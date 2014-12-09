/**
 * \file        AgentsParameters.h
 * \date        Jul 4, 2014
 * \version     v0.6
 * \copyright   <2009-2014> Forschungszentrum Jülich GmbH. All rights reserved.
 *
 * \section License
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * \section Description
 * This class contains the different force models dependent parameters for the agents.
 * They are used to defined different population group, for instance children, men, elderly...
 * It is done by specifying different sizes, desired velocities and reaction times
 *
 **/


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
     * Initialize the desired velocity distribution walking up stairs
     * @param mean, mean value
     * @param stv, standard deviation
     */
    void InitV0UpStairs(double mean, double stv);

    /**
     * Initialize the desired velocity distribution walking downstairs
     * @param mean, mean value
     * @param stv, standard deviation
     */
    void InitV0DownStairs(double mean, double stv);

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
    double GetV0UpStairs();

    /**
     * @return a random number following the distribution
     */
    double GetV0DownStairs();

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

    /**
     * return a summry of the parameters
     */
    std::string writeParameter();

private:
    int _id;
    //std::random_device _rd;
    std::default_random_engine _generator;
    std::normal_distribution<double> _V0;
    std::normal_distribution<double> _V0UpStairs;
    std::normal_distribution<double> _V0DownStairs;
    std::normal_distribution<double> _Bmax;
    std::normal_distribution<double> _Bmin;
    std::normal_distribution<double> _Atau;
    std::normal_distribution<double> _Amin;
    std::normal_distribution<double> _Tau;

};

#endif /* AGENTSPARAMETERS_H_ */
