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

#include "PedDistributionFromProtobufLoader.h"

PedDistributionFromProtobufLoader::PedDistributionFromProtobufLoader(
    const Configuration * configuration) :
    _configuration(configuration)
{
}

bool PedDistributionFromProtobufLoader::LoadPedDistribution(
    std::vector<std::shared_ptr<StartDistribution>> & startDis,
    std::vector<std::shared_ptr<StartDistribution>> & startDisSub,
    std::vector<std::shared_ptr<AgentsSource>> & startDisSources)
{
    for(hybridsim::Group pbGroup : _configuration->GetScenario()->group()) {
        double x_min = pbGroup.min_x_y().x();
        double y_min = pbGroup.min_x_y().y();
        double x_max = pbGroup.max_x_y().x();
        double y_max = pbGroup.max_x_y().y();

        double bounds[4] = {x_min, x_max, y_min, y_max};

        //sanity check
        if((x_max < x_min) || (y_max < y_min)) {
            Log->Write(
                "ERROR:\tinvalid bounds [%0.2f,%0.2f,%0.2f,%0.2f] of the group [%d]. Max and Min "
                "values mismatched?",
                pbGroup.group_id(),
                x_min,
                x_max,
                y_min,
                y_max);
            return false;
        }

        auto dis =
            std::shared_ptr<StartDistribution>(new StartDistribution(_configuration->GetSeed()));
        dis->SetRoomID(pbGroup.room_id());
        dis->SetSubroomID(pbGroup.subroom_id());
        //dis->SetSubroomUID(subroom_uid);
        dis->SetGroupId(pbGroup.group_id());
        dis->Setbounds(bounds);
        dis->SetAgentsNumber(pbGroup.number());
        dis->SetRouterId(pbGroup.router_id());

        Log->Write("WARNING:\tserveral group parameters are not yet implemented in proto file, "
                   "using generic values [gl march '16]");
        dis->SetAge(42);
        dis->SetGender("other");
        dis->SetGoalId(FINAL_DEST_OUT);
        dis->SetRouteId(-1);
        dis->SetHeight(2);
        dis->SetPatience(5);
        dis->InitPremovementTime(0., 0.);
        dis->InitRiskTolerance(0., 0.);

        if(pbGroup.subroom_id() == -1) { // no subroom was supplied
            startDis.push_back(dis);
        } else {
            startDisSub.push_back(dis);
        }

        Log->Write("WARNING:\tmultiple agent_parameters not yet implemented in proto file, using "
                   "default values [gl march '16]");
        int agent_para_id = 1; //pbGroup.agent_parameter_id();
        if(_configuration->GetAgentsParameters().count(agent_para_id) == 0) {
            Log->Write(
                "ERROR:\t Please specify which set of agents parameters (agent_parameter_id) to "
                "use for the group [%d]!",
                pbGroup.group_id());
            Log->Write("ERROR:\t Default values are not implemented yet");
            return false;
        }
        dis->SetGroupParameters(_configuration->GetAgentsParameters().at(agent_para_id).get());

        Log->Write("WARNING:\tparams start_x and start_y not yet implemented in proto file, using "
                   "default values [gl march '16]");
    }

    for(hybridsim::Source pbSource : _configuration->GetScenario()->source()) {
        auto source = std::shared_ptr<AgentsSource>(new AgentsSource(
            pbSource.id(),
            pbSource.caption(),
            pbSource.max_agents(),
            pbSource.group_id(),
            pbSource.frequency()));
        startDisSources.push_back(source);
        Log->Write("INFO:\tSource with id %d will not be parsed !", pbSource.id());
    }

    Log->Write("INFO: \t...Done");
    return true;
}
