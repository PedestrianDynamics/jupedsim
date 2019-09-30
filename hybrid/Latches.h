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
//
// Created by laemmel on 22.03.16.
//

#ifndef JPSCORE_LATCHES_H
#define JPSCORE_LATCHES_H

#include <mutex>

class Latches
{
public:
    explicit Latches();

    //    Latches(const Latches&) = delete;
    ////    Latches& operator = (const Latches&) = delete;
    ~Latches() = default;

    void WaitForScenarioLoaded();

    void WaitForSimulationPrepared();

    void WaitForSimulationFinished();

    void ScenarioLoaded();

    void SimulationPrepared();

    void SimulationFinished();

private:
    std::mutex _sc;
    std::mutex _simPrep;
    std::mutex _simFinshed;
};

#endif //JPSCORE_LATCHES_H
