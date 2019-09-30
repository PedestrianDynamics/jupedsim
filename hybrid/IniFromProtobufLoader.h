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

#ifndef JPSCORE_INIFROMPROTOBUFLOADER_H
#define JPSCORE_INIFROMPROTOBUFLOADER_H

#include "../general/Configuration.h"

class IniFromProtobufLoader
{
public:
    IniFromProtobufLoader(Configuration * configuration);

    void Initialize(const hybridsim::Scenario * scenario);

private:
    Configuration * _configuration;

    void configureGompertz(const hybridsim::Model & model);

    void setSolver(const std::string & solver);

    void setForcePed(const hybridsim::Force & force);

    void setForceWall(const hybridsim::Force & force);

    void configureAgentParams(const hybridsim::AgentParams & params);

    std::shared_ptr<DirectionStrategy>
    configureExitCrossingStrategy(google::protobuf::int32 strategy);

    void IniRoutingStrategies(const google::protobuf::RepeatedPtrField<hybridsim::Router> & field);
};

#endif //JPSCORE_INIFROMPROTOBUFLOADER_H
