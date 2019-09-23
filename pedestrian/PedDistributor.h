/**
 * \file        PedDistributor.h
 * \date        Oct 12, 2010
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
 *
 *
 **/
#pragma once

#include <vector>
#include <string>
#include <memory>

#include "AgentsParameters.h"
#include "AgentsSource.h"
#include "StartDistribution.h"

#include "router/Router.h"
#include "geometry/Building.h"
#include "general/ArgumentParser.h"


class PedDistributor {
private:
    std::vector<std::shared_ptr<StartDistribution> > _start_dis; // ID startraum, subroom und Anz
    std::vector<std::shared_ptr<StartDistribution> > _start_dis_sub; // ID startraum, subroom und Anz
    std::vector<std::shared_ptr<AgentsSource> > _start_dis_sources; // contain the sources
    //std::string _projectFilename; // store the file for later user
    //std::map<int, AgentsParameters*> _agentsParameters;

    std::vector<Point> PositionsOnFixX(double max_x, double min_x, double max_y,
                                              double min_y, const SubRoom &r, double bufx, double bufy, double dy) const;

    std::vector<Point> PositionsOnFixY(double max_x, double min_x, double max_y,
                                              double min_y, const SubRoom &r, double bufx, double bufy, double dx) const;

    const Configuration * _configuration;
public:
    /**
     * constructor
     */
    PedDistributor(const Configuration *configuration);

    /**
     * destructor
     */
    virtual ~PedDistributor();

    /**
     * Return the possible positions for distributing the agents in the subroom
     */
    std::vector<Point> PossiblePositions(const SubRoom &r) const;

    /**
     * Distribute the pedestrians in the Subroom with the given parameters
     */
    void DistributeInSubRoom(int N, std::vector<Point> &positions, int *pid,
                             StartDistribution *parameters, Building *building) const;

    /**
     *
     *Distribute all agents based on the configuration (ini) file
     * @return true if everything went fine
     */
    bool Distribute(Building *building) const;

    /**
     * provided for convenience
     */

    const std::vector<std::shared_ptr<AgentsSource> > &GetAgentsSources() const;

    /**
     * Get positions from filename
     * Only for frame's position for every id is given
     * WARNING: It is assumed that positions should be in the corresponding room/subroom.
     * checks with n if the file delivers the right number of peds
     */
    const std::vector<Point> GetPositionsFromFile(std::string filename, int n, std::string unit) const;

    double GetA_dist() const;

    double GetB_dist() const;
};
