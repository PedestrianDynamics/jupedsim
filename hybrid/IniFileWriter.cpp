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
// Created by laemmel on 11.04.16.
//

#include "IniFileWriter.h"
#include "../general/Macros.h"
#include "../geometry/Point.h"
#include "../geometry/Goal.h"
#include "../geometry/Building.h"
#include "../Simulation.h"
#include "../math/GompertzModel.h"
#include "../pedestrian/AgentsParameters.h"

IniFileWriter::IniFileWriter(const Configuration* configuration, Simulation* simulation, SimObserver* simObserver)
          :_configuration(configuration),
           _simulation(simulation),
           _simObserver(simObserver)
{

}


void IniFileWriter::WriteToFile(std::string file)
{
     _fileHandler = std::unique_ptr<FileHandler>(new FileHandler(file));
     WriteHeader();

     WriteBody();

     WriteFooter();
}

void IniFileWriter::WriteHeader()
{
     std::stringstream str;
     str << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" << std::endl;
     str << "<JuPedSim project=\"JPS-Project\" version=\"" << JPS_VERSION << "\"" << std::endl;
     str << "          xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"" << std::endl;
     str << "          xsi:noNamespaceSchemaLocation=\"http://xsd.jupedsim.org/" << JPS_VERSION
               << "/jps_ini_core.xsd\">\n" << std::endl;
     _fileHandler->Write(str.str());
}

void IniFileWriter::WriteFooter()
{
     _fileHandler->Write("</JuPedSim>\n");
}

void IniFileWriter::WriteBody()
{
     WriteRndSeed();
     WriteMxSimTm();
     WriteGeoFileName();
     WriteTrajectoryOutputDefinition();
     WriteLogFileLocation();
     WriteTrafficInformation();
     WriteRouting();
     WriteAgents();
     WriteOpModels();
     WriteRouteChoiceModels();
}

void IniFileWriter::WriteRndSeed()
{
     std::stringstream str;
     str << "\t<!-- seed used for initialising random generator -->" << std::endl;
     str << "\t<seed>" << _configuration->GetSeed() << "</seed>" << std::endl;
     _fileHandler->Write(str.str());
}

void IniFileWriter::WriteMxSimTm()
{
     std::stringstream str;
     str << "\t<!-- latest simulation termination time -->" << std::endl;
     str << "\t<max_sim_time>" << _configuration->GetTmax() << "</max_sim_time>" << std::endl;
     _fileHandler->Write(str.str());
}

void IniFileWriter::WriteGeoFileName()
{
     std::stringstream str;
     str << "\t<!-- geometry file name -->" << std::endl;
     str << "\t<geometry>" << _configuration->GetGeometryFile() << "</geometry>" << std::endl;
     _fileHandler->Write(str.str());
}

void IniFileWriter::WriteTrajectoryOutputDefinition()
{
     std::stringstream str;
     str << "\t<!-- trajectories file and format -->" << std::endl;
     std::string fm;
     switch (_configuration->GetFileFormat()) {
     case FORMAT_XML_PLAIN :
          fm = "xml-plain";
          break;
     case FORMAT_PLAIN:
          fm = "plain";
          break;
     default:
          std::cerr << "file format not yet supported by IniFileWriter" << std::endl;
          exit(-1);
     }
     str << "\t<trajectories format=\"" << fm << "\" fps=\"" << _configuration->GetFps() << "\">" << std::endl;
     if (!_configuration->GetTrajectoriesFile().empty()) {
          str << "\t\t<file location=\"" << _configuration->GetTrajectoriesFile() << "\"/>" << std::endl;
     }
     else {
          str << "\t\t<!-- <file location=\"somelocation.xml\"/> -->" << std::endl;
     }
     if (_configuration->GetHostname()!="" && _configuration->GetPort()!=0) {
          str << "\t\t<socket hostname=\"" << _configuration->GetHostname() << "\" port=\"" << _configuration->GetPort()
                    << "\"/>" << std::endl;
     }
     else {
          str << "\t\t<!--<socket hostname=\"127.0.0.1\" port=\"8989\"/>-->" << std::endl;
     }
     str << "\t</trajectories>" << std::endl;

     _fileHandler->Write(str.str());
}

void IniFileWriter::WriteLogFileLocation()
{
     std::stringstream str;
     str << "\t<!-- output log file location -->" << std::endl;
     if (!_configuration->GetErrorLogFile().empty()) {
          str << "\t<logfile>" << _configuration->GetErrorLogFile() << "</logfile>" << std::endl;
     }
     else {
          str << "\t<!--<logfile>log.txt</logfile> -->" << std::endl;
     }
     _fileHandler->Write(str.str());
}

void IniFileWriter::WriteTrafficInformation()
{
     std::stringstream str;
     str << "\t<!-- traffic information: e.g closed doors -->" << std::endl;
     str << "\t<traffic_constraints>" << std::endl;
     str << "\t\t<!-- NOT YET IMPLEMENTED [gl Apr '16] -->" << std::endl;
     str << "\t</traffic_constraints>" << std::endl;

     _fileHandler->Write(str.str());
}

void IniFileWriter::WriteRouting()
{
     std::stringstream str;
     str << "\t<!-- routing information such as goals and the like -->" << std::endl;
     str << "\t<routing>" << std::endl;
     str << "\t\t<goals>" << std::endl;
     WriteGoals(str);
     str << "\t\t</goals>" << std::endl;
     str << "\t</routing>" << std::endl;
     _fileHandler->Write(str.str());
}

void IniFileWriter::WriteGoals(std::stringstream& str)
{

     for (auto&& entry : _simulation->GetBuilding()->GetAllGoals()) {
          Goal* goal = entry.second;
          str << "\t\t\t<goal id=\"" << goal->GetId() << "\" final=\"" << (goal->GetIsFinalGoal()==0 ? "false" : "true")
                    << "\" caption=\"" << goal->GetCaption() << "\">" << std::endl;
          const std::vector<Point>& p = goal->GetPolygon();
          str << "\t\t\t\t<polygon>" << std::endl;
          for (Point pnt : p) {
               str << "\t\t\t\t\t<vertex px=\"" << pnt._x << "\" py=\"" << pnt._y << "\" />" << std::endl;
          }
          str << "\t\t\t\t\t<vertex px=\"" << p.begin()->_x << "\" py=\"" << p.begin()->_y << "\" />" << std::endl;
          str << "\t\t\t\t</polygon>" << std::endl;
          str << "\t\t\t</goal>" << std::endl;
     }
}

void IniFileWriter::WriteAgents()
{

//     const vector<std::shared_ptr<AgentsSource>>& srcs = _simulation->GetAgentSrcManager().GetSources();


     std::stringstream str;
     str << "\t<!--persons information and distribution -->" << std::endl;
     str << "\t<agents operational_model_id=\"" << "2" << "\">"
               << std::endl; //TODO: currently Gompertz one model is supported [gl Apr. '16]
     str << "\t\t<agents_distribution>" << std::endl;

     std::vector<std::string> sources;
     int srcIds = -1;
//     for (std::shared_ptr<AgentsSource> ptr : srcs) {
     int grpIds = -1;
     for (auto entry : _simObserver->GetAllRelations()) {
          SimObserver::ODRelation& od = entry.second;
          std::shared_ptr<AgentsSource>& ptr = od.src;
          StartDistribution* sd = ptr->GetStartDistribution().get();
          double bounds[4] = {0, 0, 0, 0};
          sd->Getbounds(bounds);

          str << "\t\t\t<group group_id=\"" << ++grpIds << "\" room_id=\"" << sd->GetRoomId() << "\" "
                    "subroom_id=\"" << sd->GetSubroomID() << "\" number=\"" << od.nrAgents
                    << "\" goal_id=\"" << od.goalId << "\" router_id=\""
                    << sd->GetRouterId() << "\" agent_parameter_id=\"1\" x_min=\"" << bounds[0] << "\" x_max=\""
                    << bounds[1] << "\" y_min=\"" << bounds[2] << "\" y_max=\"" << bounds[3] << "\" />" << std::endl;

          std::stringstream src;
          src << "\t\t\t<source id=\"" << ++srcIds << "\" frequency=\"1\" agents_max=\"0\" group_id=\""
                    << grpIds
                    << "\" caption=\"source " << srcIds << "\"/>" << std::endl;
          sources.push_back(src.str());

     }

     str << "\t\t</agents_distribution>" << std::endl;
     str << "\t\t<agents_sources>" << std::endl;
     for (std::string src : sources) {
          str << src;
     }
     str << "\t\t</agents_sources>" << std::endl;
     str << "\t</agents>" << std::endl;
     _fileHandler->Write(str.str());
}

void IniFileWriter::WriteOpModels()
{

     std::stringstream str;
     str << "\t<operational_models>" << std::endl;
     OperationalModel* op = _configuration->GetModel().get();

     if (GompertzModel* g = dynamic_cast<GompertzModel*>(op)) { //TODO bad practice and poor oop design [gl Apr. '16]
          str << "\t\t<model operational_model_id=\"2\" description=\"gompertz\">" << std::endl;
          WriteGompertzModel(str);
     }
     else {
          std::cerr << "operational model:" << op->GetDescription() << " not yet supported by IniFileWriter"
                    << std::endl;
          exit(-1);
     }

     WriteAgentParameters(str);

     str << "\t\t</model>" << std::endl;
     str << "\t</operational_models>" << std::endl;

     _fileHandler->Write(str.str());

}

void IniFileWriter::WriteRouteChoiceModels()
{
     std::stringstream str;
     str << "\t<route_choice_models>" << std::endl;
     for (Router* r : _configuration->GetRoutingEngine().get()->GetAvailableRouters()) {
          switch (r->GetStrategy()) {
          case RoutingStrategy::ROUTING_GLOBAL_SHORTEST :
               str << "\t\t<router router_id=\"" << r->GetID() << "\" description=\"global_shortest\">" << std::endl;
               str << "\t\t\t<parameters>" << std::endl;
               str << "\t\t\t</parameters>" << std::endl;
               str << "\t\t</router>" << std::endl;
               break;
          default:
               std::cerr << "Routing strategy: " << r->GetID() << " not yet supported by IniFileWriter" << std::endl;
               exit(-1);
          }
     }
     str << "\t</route_choice_models>" << std::endl;

     _fileHandler->Write(str.str());
}

void IniFileWriter::WriteGompertzModel(std::stringstream& str)
{
     GompertzModel* m = (GompertzModel*) _configuration->GetModel().get();

     str << "\t\t\t<model_parameters>" << std::endl;
     switch (_configuration->GetSolver()) { //TODO bad practice, add at least enum in Macros.h [gl Apr. '16]
     case 1:
          str << "\t\t\t\t<solver>euler</solver>" << std::endl;
          break;
     case 2:
          str << "\t\t\t\t<solver>verlet</solver>" << std::endl;
          break;
     case 3:
          str << "\t\t\t\t<solver>leapfrog</solver>" << std::endl;
          break;
     default:
          std::cerr << "unknown solver type: " << _configuration->GetSolver() << std::endl;
          exit(-1);
     }

     str << "\t\t\t\t<stepsize>" << _configuration->Getdt() << "</stepsize>" << std::endl;
     str << "\t\t\t\t<exit_crossing_strategy>" << "3"
               << "</exit_crossing_strategy> <!-- currently, ``3'' is hard coded "
                         "as the IniFileWriter does not support anything else [gl Apr. '16] -->" << std::endl;
     str << "\t\t\t\t<linkedcells enabled=\"" << (_configuration->GetLinkedCellSize()>0. ? "true" : "false")
               << "\" cell_size=\""
               << _configuration->GetLinkedCellSize() << "\" />" << std::endl;
     str << "\t\t\t\t<force_ped nu=\"" << m->GetNuPed() << "\" b=\"" << m->GetbPed() << "\" c=\"" << m->GetcPed()
               << "\" />" << std::endl;
     str << "\t\t\t\t<force_wall nu=\"" << m->GetNuWall() << "\" b=\"" << m->GetbWall() << "\" c=\"" << m->GetcWall()
               << "\" />" << std::endl;
     str << "\t\t\t</model_parameters>" << std::endl;

}

void IniFileWriter::WriteAgentParameters(std::stringstream& str)
{
//     str <<
     for (auto entry : _configuration->GetAgentsParameters()) {
          const int id = entry.first;
          AgentsParameters* p = entry.second.get();
          str << "\t\t\t<agent_parameters agent_parameter_id=\"" << id << "\">" << std::endl;
          str << "\t\t\t<!-- IniFileWriter cannot reconstruct parameters' distributions, setting sigma to 0.0 for all "
                    "entries. Change them to something else at your convenience -->" << std::endl;
          str << "\t\t\t\t<v0 mu=\"" << p->GetV0() << "\" sigma=\"0.0\"/>" << std::endl;
          str << "\t\t\t\t<bmax mu=\"" << p->GetBmax() << "\" sigma=\"0.0\"/>" << std::endl;
          str << "\t\t\t\t<bmin mu=\"" << p->GetBmin() << "\" sigma=\"0.0\"/>" << std::endl;
          str << "\t\t\t\t<amin mu=\"" << p->GetAmin() << "\" sigma=\"0.0\"/>" << std::endl;
          str << "\t\t\t\t<tau mu=\"" << p->GetTau() << "\" sigma=\"0.0\"/>" << std::endl;
          str << "\t\t\t\t<atau mu=\"" << p->GetAtau() << "\" sigma=\"0.0\"/>" << std::endl;
          str << "\t\t\t</agent_parameters>" << std::endl;
     }

}









