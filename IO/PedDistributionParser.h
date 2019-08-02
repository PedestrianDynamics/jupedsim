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
// Created by laemmel on 31.03.16.
//
#pragma once

#include "pedestrian/PedDistributionLoader.h"

#include <tinyxml.h>

class Configuration;

class PedDistributionParser : public PedDistributionLoader {

public:
     PedDistributionParser(const Configuration* configuration);
     ~PedDistributionParser();
     virtual bool LoadPedDistribution(std::vector<std::shared_ptr<StartDistribution> >& startDis,
               std::vector<std::shared_ptr<StartDistribution> >& startDisSub,
               std::vector<std::shared_ptr<AgentsSource> >& startDisSources) override;
     std::shared_ptr<AgentsSource> parseSourceNode(TiXmlElement* e);


private:
     const Configuration* _configuration;

};
