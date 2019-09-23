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

#include <cstdlib>

#include "IniFromProtobufLoader.h"
#include "math/GompertzModel.h"
#include "pedestrian/AgentsParameters.h"
#include "router/GlobalRouter.h"
#include "router/QuickestPathRouter.h"
#include "router/MeshRouter.h"
#include "router/DummyRouter.h"
#include ".router/SafestPathRouter.h"
#include "router/CognitiveMapRouter.h"

IniFromProtobufLoader::IniFromProtobufLoader(Configuration* configuration)
          :_configuration(configuration)
{

}

void IniFromProtobufLoader::Initialize(const Scenario* scenario)
{


     _configuration->SetSeed(scenario->seed());
     _configuration->SetShowStatistics(false);

     if (scenario->model().type()==hybridsim::Model::Type::Model_Type_Gompertz) {
          configureGompertz(scenario->model());
     }

     IniRoutingStrategies(scenario->router());


}

void IniFromProtobufLoader::configureGompertz(const hybridsim::Model& model)
{
//    _configuration->SetSolver(model.gompertz().solver());
     setSolver(model.gompertz().solver());
     _configuration->Setdt(model.gompertz().stepsize());
     std::shared_ptr<DirectionStrategy> exit_strategy = configureExitCrossingStrategy(
               model.gompertz().exit_crossing_strategy());
     if (model.gompertz().linked_cells_enabled()) {
          _configuration->SetLinkedCellSize(model.gompertz().cell_size());
     }
     else {
          _configuration->SetLinkedCellSize(-1.0);
     }
     if (model.gompertz().has_force_ped()) {//TODO default values
          setForcePed(model.gompertz().force_ped());
     }
     if (model.gompertz().has_force_wall()) {//TODO default values
          setForceWall(model.gompertz().force_wall());
     }
     configureAgentParams(model.gompertz().agent_params());
     double beta_c = 1;
     auto params = _configuration->GetAgentsParameters();

     //TODO the following is apparently a hack (c&p from InFileParser)
     double max_Ea = params[1]->GetAmin()+params[1]->GetAtau()*params[1]->GetV0();
     double max_Eb = 0.5*(params[1]->GetBmin()+0.49);
     double max_Ea_Eb = (max_Ea>max_Eb) ? max_Ea : max_Eb;
     _configuration->SetDistEffMaxPed(2*beta_c*max_Ea_Eb);
     _configuration->SetDistEffMaxWall(_configuration->GetDistEffMaxPed());

     //TODO: models do not belong in a configuration container [gl march '16]
     _configuration->SetModel(
               std::shared_ptr<OperationalModel>(new GompertzModel(exit_strategy, _configuration->GetNuPed(),
                         _configuration->GetaPed(), _configuration->GetbPed(), _configuration->GetcPed(),
                         _configuration->GetNuWall(), _configuration->GetaWall(), _configuration->GetbWall(),
                         _configuration->GetcWall())));


}

void IniFromProtobufLoader::setSolver(const std::string& solver)
{
     if (solver=="euler") {
          _configuration->SetSolver(1);
     }
     else if (solver=="verlet")
          _configuration->SetSolver(2);
     else if (solver=="leapfrog")
          _configuration->SetSolver(3);
     else {
          Log->Write("ERROR:\twrong value [%s] for solver type\n", solver.c_str());
//        return false;
     }
     Log->Write("INFO:\tSolver <"+string(solver)+">");
//    return true;
}

std::shared_ptr<DirectionStrategy> IniFromProtobufLoader::configureExitCrossingStrategy(
          google::protobuf::int32 strategy)
{
     std::shared_ptr<DirectionStrategy> exit_strategy;

     switch (strategy) {
     case 1:
          exit_strategy = std::shared_ptr<DirectionStrategy>(new DirectionMiddlePoint());
          break;
     case 2:
          exit_strategy = std::shared_ptr<DirectionStrategy>(new DirectionMinSeperationShorterLine());
          break;
     case 3:
          exit_strategy = std::shared_ptr<DirectionStrategy>(new DirectionInRangeBottleneck());
          break;
     case 4:
          exit_strategy = std::shared_ptr<DirectionStrategy>(new DirectionGeneral());
          break;
     case 6:
          exit_strategy = std::shared_ptr<DirectionStrategy>(new DirectionFloorfield());
          break;
     case 7:
          exit_strategy = std::shared_ptr<DirectionStrategy>(new DirectionGoalFloorfield());
          break;
     case 8:
          exit_strategy = std::shared_ptr<DirectionStrategy>(new DirectionLocalFloorfield());
          break;
     case 9:
          exit_strategy = std::shared_ptr<DirectionStrategy>(new DirectionSubLocalFloorfield());
          break;
     default:
          exit_strategy = std::shared_ptr<DirectionStrategy>(new DirectionMinSeperationShorterLine());
          Log->Write("ERROR:\tunknown exit_crossing_strategy <%d>", strategy);
          Log->Write("     :\tthe default <%d> will be used", 2);
//            return true;
          break;
     }
     Log->Write("INFO: \texit_crossing_strategy < %d >", strategy);
     return exit_strategy;
}

void IniFromProtobufLoader::setForcePed(const hybridsim::Force& force)
{
     _configuration->SetNuPed(force.nu());
     _configuration->SetaPed(force.a());
     _configuration->SetbPed(force.b());
     _configuration->SetcPed(force.c());

     Log->Write("INFO:\tfrep_ped mu=%0.2f, a=%0.2f, b=%0.2f c=%0.2f", _configuration->GetNuPed(),
               _configuration->GetaPed(), _configuration->GetbPed(), _configuration->GetcPed());
}

void IniFromProtobufLoader::setForceWall(const hybridsim::Force& force)
{
     _configuration->SetNuWall(force.nu());
     _configuration->SetaWall(force.a());
     _configuration->SetbWall(force.b());
     _configuration->SetcWall(force.c());

     Log->Write("INFO:\tfrep_wall mu=%0.2f, a=%0.2f, b=%0.2f c=%0.2f", _configuration->GetNuWall(),
               _configuration->GetaWall(), _configuration->GetbWall(), _configuration->GetcWall());
}

void IniFromProtobufLoader::configureAgentParams(const hybridsim::AgentParams& params)
{
     int para_id = 1;//TODO thus far only one set of params [gl march '16]
     auto agentParameters = std::shared_ptr<AgentsParameters>(new AgentsParameters(para_id, _configuration->GetSeed()));
     _configuration->AddAgentsParameters(agentParameters, para_id);
     if (params.has_v0()) {
          double mu = params.v0().mu();
          double sigma = params.v0().sigma();
          agentParameters->InitV0(mu, sigma);
          agentParameters->InitV0DownStairs(mu, sigma);
          agentParameters->InitV0UpStairs(mu, sigma);
          Log->Write("INFO: \tdesired speed mu=%f , sigma=%f", mu, sigma);
     }
     if (params.has_v0_upstairs()) {
          double mu = params.v0_upstairs().mu();
          double sigma = params.v0_upstairs().sigma();
          agentParameters->InitV0UpStairs(mu, sigma);
          Log->Write("INFO: \tdesired speed upstairs mu=%f , sigma=%f", mu, sigma);
     }
     if (params.has_v0_downstairs()) {
          double mu = params.v0_downstairs().mu();
          double sigma = params.v0_downstairs().sigma();
          agentParameters->InitV0DownStairs(mu, sigma);
          Log->Write("INFO: \tdesired speed downstairs mu=%f , sigma=%f", mu, sigma);
     }
     if (params.has_v0_escalator_up()) {
          double mu = params.v0_escalator_up().mu();
          double sigma = params.v0_escalator_up().sigma();
          agentParameters->InitEscalatorUpStairs(mu, sigma);
          Log->Write("INFO: \tspeed of escalator upstairs mu=%f , sigma=%f", mu, sigma);
     }
     if (params.has_v0_escalator_down()) {
          double mu = params.v0_escalator_down().mu();
          double sigma = params.v0_escalator_down().sigma();
          agentParameters->InitEscalatorDownStairs(mu, sigma);
          Log->Write("INFO: \tspeed of escalator downstairs mu=%f , sigma=%f", mu, sigma);
     }
     if (params.has_v0_idle_escalator_up()) {
          double mu = params.v0_idle_escalator_up().mu();
          double sigma = params.v0_idle_escalator_up().sigma();
          agentParameters->InitV0IdleEscalatorUpStairs(mu, sigma);
          Log->Write("INFO: \tdesired speed idle escalator upstairs mu=%f , sigma=%f", mu, sigma);
     }
     if (params.has_v0_idle_escalator_down()) {
          double mu = params.v0_idle_escalator_down().mu();
          double sigma = params.v0_idle_escalator_down().sigma();
          agentParameters->InitV0IdleEscalatorDownStairs(mu, sigma);
          Log->Write("INFO: \tdesired speed idle escalator downstairs mu=%f , sigma=%f", mu, sigma);
     }

     if (params.has_b_max()) {
          double mu = params.b_max().mu();
          double sigma = params.b_max().sigma();
          agentParameters->InitBmax(mu, sigma);
          Log->Write("INFO: \tBmax mu=%f , sigma=%f", mu, sigma);
     }
     if (params.has_b_min()) {
          double mu = params.b_min().mu();
          double sigma = params.b_min().sigma();
          agentParameters->InitBmin(mu, sigma);
          Log->Write("INFO: \tBmin mu=%f , sigma=%f", mu, sigma);
     }
     if (params.has_a_min()) {
          double mu = params.a_min().mu();
          double sigma = params.a_min().sigma();
          agentParameters->InitAmin(mu, sigma);
          Log->Write("INFO: \tAmin mu=%f , sigma=%f", mu, sigma);
     }
     if (params.has_tau()) {
          double mu = params.tau().mu();
          double sigma = params.tau().sigma();
          agentParameters->InitTau(mu, sigma);
          Log->Write("INFO: \tTau mu=%f , sigma=%f", mu, sigma);
     }
     if (params.has_atau()) {
          double mu = params.atau().mu();
          double sigma = params.atau().sigma();
          agentParameters->InitAtau(mu, sigma);
          Log->Write("INFO: \tAtau mu=%f , sigma=%f", mu, sigma);
     }
//    if (params.has_T())//TODO implement T

}

void IniFromProtobufLoader::IniRoutingStrategies(const google::protobuf::RepeatedPtrField<hybridsim::Router>& field)
{
     for (hybridsim::Router r : field) {
          std::string strategy = r.description();
          int id = r.router_id();
          if (strategy=="local_shortest") {
               Router* r = new GlobalRouter(id, ROUTING_LOCAL_SHORTEST);
               _configuration->GetRoutingEngine()->AddRouter(r);
          }
          else if (strategy=="global_shortest") {
               Router* r = new GlobalRouter(id, ROUTING_GLOBAL_SHORTEST);
               _configuration->GetRoutingEngine()->AddRouter(r);
          }
          else if (strategy=="quickest") {
               Router* r = new QuickestPathRouter(id, ROUTING_QUICKEST);
               _configuration->GetRoutingEngine()->AddRouter(r);
          }
          else if (strategy=="nav_mesh") {
               Router* r = new MeshRouter(id, ROUTING_NAV_MESH);
               _configuration->GetRoutingEngine()->AddRouter(r);
          }
          else if (strategy=="dummy") {
               Router* r = new DummyRouter(id, ROUTING_DUMMY);
               _configuration->GetRoutingEngine()->AddRouter(r);
          }
          else if (strategy=="global_safest") {
               Router* r = new SafestPathRouter(id, ROUTING_SAFEST);
               _configuration->GetRoutingEngine()->AddRouter(r);
          }
          else if (strategy=="cognitive_map") {
               Router* r = new CognitiveMapRouter(id, ROUTING_COGNITIVEMAP);
               _configuration->GetRoutingEngine()->AddRouter(r);

               Log->Write("INFO:\tUsing CognitiveMapRouter");
               Log->Write("ERROR:\tCognitiveMapRouting not yet implemented in proto fiel.");
               ///Parsing additional options
//            if (!ParseCogMapOpts(e))
//                exit(-1);
          }
          else {
               Log->Write("ERROR: \twrong value for routing strategy [%s]!!!\n",
                         strategy.c_str());
               exit(-1);
          }
     }
}


