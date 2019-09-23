#pragma once

#include <random>

class RandomNumberGenerator
{
public:
    //Engine and distribution with random seed
    explicit RandomNumberGenerator(){std::random_device rd;
                                     _randomEngine = std::mt19937(rd());
                                     _rdDistribution = std::uniform_real_distribution<double> (0,1);}
    //Engine and distribution with specific seed
    explicit RandomNumberGenerator(int seed):_randomEngine(std::mt19937(seed)),
        _rdDistribution(std::uniform_real_distribution<double> (0,1)){}

    double GetRandomRealBetween0and1(){return _rdDistribution(_randomEngine);}
    std::mt19937& GetRandomEngine(){return _randomEngine;}

    void SetSeed(int seed){_randomEngine = std::mt19937(seed);}

private:
    std::mt19937 _randomEngine;
    std::uniform_real_distribution<double> _rdDistribution;
};
