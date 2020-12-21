#pragma once
#include <iostream>
#include <vector>
#include <random>

template<class T>
class TemperatureDistribution
{
public:
    TemperatureDistribution() : _vals(), gen(121541231) {}
    std::mt19937 gen;

    inline void add(T val)
    {
        _vals.push_back(val);
    }

    // Weighted random selection from PT-distribution
    inline size_t eval(double tau = 1.0)
    {
        auto PT = _getPT(tau);

        std::discrete_distribution<int> dist = std::discrete_distribution<int>(PT.begin(), PT.end());

        return dist(gen);
    }

    std::vector<T> _vals;

    inline std::vector<double> _getPT(double tau)
    {
        double sum = 0.0;
        for (T val : _vals) sum += (double)val;
        std::vector<double> PT{};
        double tauSum = 0.0;
        double V;
        for (T val : _vals)
        {
            V = pow((double)val / sum, 1.0 / tau);
            PT.push_back(V);
            tauSum += V;
        }
        for (int i = 0; i < PT.size(); i++) PT[i] /= tauSum;
        return PT;
    }
};