/**
 * \file        AgentsParameters.h
 * \date        Jul 4, 2014
 * \version     v0.7
 * \copyright   <2009-2015> Forschungszentrum Jülich GmbH. All rights reserved.
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
#pragma once

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
     * Initialize the speed distribution of escalators upstairs
     * @param mean, mean value
     * @param stv, standard deviation
     */
    void InitEscalatorUpStairs(double mean, double stv);

     /**
     * Initialize the speed distribution of escalators downstairs
     * @param mean, mean value
     * @param stv, standard deviation
     */
    void InitEscalatorDownStairs(double mean, double stv);

     /**
     * Initialize the desired speed distribution walking idle escalators upstairs
     * @param mean, mean value
     * @param stv, standard deviation
     */
    void InitV0IdleEscalatorUpStairs(double mean, double stv);

     /**
     * Initialize the desired speed distribution walking idle escalators downstairs
     * @param mean, mean value
     * @param stv, standard deviation
     */
    void InitV0IdleEscalatorDownStairs(double mean, double stv);

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
     * Initialize the reaction time
     * @param mean, mean value
     * @param stv, standard deviation
     */
    void InitT(double mean, double stv);

    void SetSwayParams(double freqA, double freqB, double ampA, double ampB);

    void EnableStretch(bool stretch);


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
    double GetEscalatorUpStairs();

    /**
     * @return a random number following the distribution
     */
    double GetEscalatorDownStairs();

    /**
     * @return a random number following the distribution
     */
    double GetV0IdleEscalatorUpStairs();

    /**
     * @return a random number following the distribution
     */
    double GetV0IdleEscalatorDownStairs();

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
     * @return a random number following the distribution
     */
    double GetT();

    double GetSwayFreqA() const;

    double GetSwayFreqB() const;

    double GetSwayAmpA() const;

    double GetSwayAmpB() const;

    /**
     * @return whether Ellipse stretching is enabled
     */
    bool StretchEnabled();

    /**
     * return a summary of the parameters
     */
    std::string writeParameter();

    double GetAminMean();

    double GetBmaxMean();

private:
    int _id;
    //std::random_device _rd;
    std::default_random_engine _generator;
    std::normal_distribution<double> _V0;
    std::normal_distribution<double> _V0UpStairs;
    std::normal_distribution<double> _V0DownStairs;
    std::normal_distribution<double> _EscalatorUpStairs;
    std::normal_distribution<double> _EscalatorDownStairs;
    std::normal_distribution<double> _V0IdleEscalatorUpStairs;
    std::normal_distribution<double> _V0IdleEscalatorDownStairs;
    std::normal_distribution<double> _Bmax;
    std::normal_distribution<double> _Bmin;
    bool _enableStretch = true;
    std::normal_distribution<double> _Atau;
    std::normal_distribution<double> _Amin;
    std::normal_distribution<double> _Tau;
    std::normal_distribution<double> _T;
    double _swayFreqA = 0.44;
    double _swayFreqB = 0.35;
    double _swayAmpA = -0.14;
    double _swayAmpB = 0.21;
	const double judge = 10000;
};
