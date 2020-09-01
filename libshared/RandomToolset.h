/**
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
 **/
#pragma once

#include <algorithm>
#include <atomic>
#include <boost/random.hpp>
#include <cassert>
#include <stdexcept>

/**
 * Toolset for creating random numbers
 */
namespace Random
{
/**
 * Random engine used for creating the random numbers
 */
extern boost::random::mt19937 _randomEngine;

/**
 * Flag if a seed was properly set
 */
extern std::atomic<bool> _init;

/**
 * Seed the random number generator
 * @param seed seed used for the random number generator
 */
void Setup(int seed);

/**
 * Returns a random number of a normal distribution with mean (\p mean) and standard deviation (\p stddev).
 * @tparam RealType
 * @param mean mean of the normal distribution
 * @param stddev standard deviation of the normal distribution
 * @return random number of a normal distribution with given parameters
 */
template <typename RealType = double>
RealType GetNormal(RealType mean, RealType stddev)
{
    assert(_init.load());
    boost::random::normal_distribution dist(mean, stddev);
    return dist(_randomEngine);
}

/**
 * Returns an integer random number of a uniform distribution with min (\p min) and max (\p max).
 * @pre \p min <= \p max
 * @tparam IntType
 * @param min min of the uniform distribution
 * @param max max of the uniform distribution
 * @return random number of a uniform distribution with given parameters
 */
template <typename IntType = int>
IntType GetUniformInt(IntType min, IntType max)
{
    assert(_init.load());
    boost::random::uniform_int_distribution dist(min, max);
    return dist(_randomEngine);
}

/**
 * Returns an real random number of a uniform distribution with min (\p min) and max (\p max).
 * @pre \p min <= \p max
 * @tparam RealType
 * @param min min of the uniform distribution
 * @param max max of the uniform distribution
 * @return random number of a uniform distribution with given parameters
 */
template <typename RealType = double>
RealType GetUniformReal(RealType min, RealType max)
{
    assert(_init.load());
    boost::random::uniform_real_distribution dist(min, max);
    return dist(_randomEngine);
}

/**
 * Returns an real random number of a triangular distribution with min (\p min), max (\p max) and peak (\p peak).
 * Between \param min and \param peak the probability is increasing linearly and from \param peak to \p max it is decreasing linearly.
 * @pre \p min <= \p peak <= \p max
 * @tparam RealType
 * @param min min of the triangular distribution
 * @param max max of the triangular distribution
 * @param peak peak of the triangular distribution
 * @return random number of a triangular distribution with given parameters
 */
template <typename RealType = double>
RealType GetTriangular(RealType min, RealType max, RealType peak)
{
    assert(_init.load());
    std::vector<RealType> i{min, peak, max};
    std::vector<RealType> w{0, 1, 0};
    boost::random::piecewise_linear_distribution<RealType> dist{i.begin(), i.end(), w.begin()};
    return dist(_randomEngine);
}

/**
 * Returns a random number of a lognormal distribution with mean (\p mean) and standard deviation (\p stddev).
 * @tparam RealType
 * @param mean mean of the normal distribution
 * @param stddev standard deviation of the normal distribution
 * @return random number of a normal distribution with given parameters
 */
template <typename RealType = double>
RealType GetLogNormal(RealType mean, RealType stddev)
{
    assert(_init.load());
    boost::random::lognormal_distribution dist(mean, stddev);
    return dist(_randomEngine);
}

/**
 * Returns a random number based on the weights given by \p weights.
 * @tparam IntType
 * @param weights weights for the different values
 * @return random number of a normal distribution with given parameters
 */
template <typename IntType = int>
IntType GetDiscreteDistribution(std::vector<double> weights)
{
    assert(_init.load());
    boost::random::discrete_distribution<IntType> dist(std::begin(weights), std::end(weights));
    return dist(_randomEngine);
}

/**
 * Returns a random number based on a beta distribution with alpha (\p alpha) and beta (\p beta).
 * @tparam RealType
 * @param alpha alpha of the beta distribution
 * @param beta beta of the beta distribution
 * @return random number of a beta distribution
 */
template <typename RealType = double>
RealType GetBetaDistribution(RealType alpha, RealType beta)
{
    assert(_init.load());
    boost::random::beta_distribution dist(alpha, beta);
    return dist(_randomEngine);
}

/**
 * Shuffles the given \p container with the Fisher-Yates algorithm (see
 * https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle for details).
 * Note: somehow std::shuffle and boost::random_shuffle do not work with MacOs
 * @tparam RandomAccessRange
 * @param container Container to shuffle
 */
template <class RandomAccessRange>
void ShuffleContainer(RandomAccessRange & container)
{
    assert(_init.load());

    auto currentIndexCounter = container.size();
    for(auto iter = std::rbegin(container); iter != std::rend(container);
        ++iter, --currentIndexCounter) {
        const int randomIndex = GetUniformInt(0ul, currentIndexCounter);

        if(*iter != container.at(randomIndex)) {
            std::swap(container.at(randomIndex), *iter);
        }
    }
}
} // namespace Random
