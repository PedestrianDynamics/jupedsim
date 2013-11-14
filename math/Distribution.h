/**
 * File:   Distribution.h
 *
 * Created on 17. Februar 2011, 15:14
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

#ifndef _DISTRIBUTION_H
#define	_DISTRIBUTION_H


#include <vector>


class Distribution {
private:
    double _mean;
    double _sigma;
public:
    Distribution();
    Distribution(double mean, double sigma);
    Distribution(const Distribution& orig);
    virtual ~Distribution();

    /**
     * @return the mean of the distribution
     */
    double GetMean() const;

    /**
     * @return the standard deviation of the distribution
     */
    double GetSigma() const;

    /**
     * @return a random number
     */
    virtual double GetRand() =0; // Gibt EINE Zufallszahl zurück
};

class Gauss : public Distribution {
private:
    std::vector<double> _queue;
    const std::vector<double>& GetQueue() const;
    void GetPair();

public:

    Gauss();
    Gauss(double mean, double sigma);
    Gauss(const Gauss& orig);
    virtual ~Gauss();

    /**
     * @return a number (gauss distributed)
     */
    double GetRand();
};


class Equal : public Distribution {
private:

public:

	Equal();
    Equal(double m, double s);
    Equal(const Equal& orig);
    virtual ~Equal();

    /**
     * @return a random number (normal distributed)
     */
    double GetRand();
};

#endif	/* _DISTRIBUTION_H */

