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
#include "PedDistributionParser.h"

#include "general/Filesystem.h"
#include "IO/OutputHandler.h"

#include <cstdarg> // va_start and va_end

PedDistributionParser::PedDistributionParser(const Configuration* configuration)
        :_configuration(configuration)
{

}

PedDistributionParser::~PedDistributionParser() {
      // delete _configuration;
}

bool PedDistributionParser::LoadPedDistribution(std::vector<std::shared_ptr<StartDistribution> >& startDis,
        std::vector<std::shared_ptr<StartDistribution> >& startDisSub,
        std::vector<std::shared_ptr<AgentsSource> >& startDisSources)
{
    Log->Write("INFO: \tLoading and parsing the persons attributes");

    TiXmlDocument doc(_configuration->GetProjectFile().string());

    if (!doc.LoadFile()) {
        Log->Write("ERROR: \t%s", doc.ErrorDesc());
        Log->Write("ERROR: \t could not parse the project file");
        return false;
    }

    TiXmlNode* xRootNode = doc.RootElement()->FirstChild("agents");
    if (!xRootNode) {
        Log->Write("ERROR:\tcould not load persons attributes");
        return false;
    }

    TiXmlNode* xDist = xRootNode->FirstChild("agents_distribution");
    for (TiXmlElement* e = xDist->FirstChildElement("group"); e;
         e = e->NextSiblingElement("group")) {

        int room_id = xmltoi(e->Attribute("room_id"));
        int group_id = xmltoi(e->Attribute("group_id"));
        int subroom_id = xmltoi(e->Attribute("subroom_id"), -1);
        int number = xmltoi(e->Attribute("number"), 0);
        int agent_para_id = xmltoi(e->Attribute("agent_parameter_id"), -1);

        int goal_id = xmltoi(e->Attribute("goal_id"), FINAL_DEST_OUT);
        int router_id = xmltoi(e->Attribute("router_id"), -1);
        int route_id = xmltoi(e->Attribute("route_id"), -1);
        int age = xmltoi(e->Attribute("age"), -1);
        std::string gender = xmltoa(e->Attribute("gender"), "female");
        double height = xmltof(e->Attribute("height"), -1);
        double patience = xmltof(e->Attribute("patience"), 5);
        double premovement_mean = xmltof(e->Attribute("pre_movement_mean"), 0);
        double premovement_sigma = xmltof(e->Attribute("pre_movement_sigma"), 0);
        //double risk_tolerance_mean = xmltof(e->Attribute("risk_tolerance_mean"), 0);
        //double risk_tolerance_sigma = xmltof(e->Attribute("risk_tolerance_sigma"), 0);
        std::string positions_dir = xmltoa(e->Attribute("positions_dir"), "");
        std::string unit_traj = xmltoa(e->Attribute("unit"), "m");
        double x_min = xmltof(e->Attribute("x_min"), -FLT_MAX);
        double x_max = xmltof(e->Attribute("x_max"), FLT_MAX);
        double y_min = xmltof(e->Attribute("y_min"), -FLT_MAX);
        double y_max = xmltof(e->Attribute("y_max"), FLT_MAX);
        double bounds[4] = {x_min, x_max, y_min, y_max};

        //sanity check
        if ((x_max<x_min) || (y_max<y_min)) {
            Log->Write(
                    "ERROR:\tinvalid bounds [%0.2f,%0.2f,%0.2f,%0.2f] of the group [%d]. Max and Min values mismatched?",
                    group_id, x_min, x_max, y_min, y_max);
            return false;
        }
        auto dis = std::shared_ptr<StartDistribution>(new StartDistribution(_configuration->GetSeed()));
        dis->SetRoomID(room_id);
        dis->SetSubroomID(subroom_id);
        //dis->SetSubroomUID(subroom_uid);
        dis->SetGroupId(group_id);
        dis->Setbounds(bounds);
        dis->SetAgentsNumber(number);
        dis->SetAge(age);
        dis->SetGender(gender);
        dis->SetGoalId(goal_id);
        dis->SetRouteId(route_id);
        dis->SetRouterId(router_id);
        dis->SetHeight(height);
        dis->SetPatience(patience);
        dis->InitPremovementTime(premovement_mean, premovement_sigma);
        dis->SetPositionsDir(positions_dir);
        dis->SetUnitTraj(unit_traj);

        if (dis->GetPositionsDir().length())
        {
              Log->Write("INFO:\tPositions_dir = <%s>\n", dis->GetPositionsDir().c_str());
        }
        if (e->Attribute("risk_tolerance_mean") && e->Attribute("risk_tolerance_sigma")) {
            std::string distribution_type = "normal";
            double risk_tolerance_mean = xmltof(e->Attribute("risk_tolerance_mean"), NAN);
            double risk_tolerance_sigma = xmltof(e->Attribute("risk_tolerance_sigma"), NAN);
            Log->Write("INFO:\trisk tolerance mu = %f, risk tolerance sigma = %f\n", risk_tolerance_mean,
                    risk_tolerance_sigma);
            dis->InitRiskTolerance(distribution_type, risk_tolerance_mean, risk_tolerance_sigma);
        }
        else if (e->Attribute("risk_tolerance_alpha") && e->Attribute("risk_tolerance_beta")) {
            std::string distribution_type = "beta";
            double risk_tolerance_alpha = xmltof(e->Attribute("risk_tolerance_alpha"), NAN);
            double risk_tolerance_beta = xmltof(e->Attribute("risk_tolerance_beta"), NAN);
            Log->Write("INFO:\trisk tolerance alpha = %f, risk tolerance beta = %f\n", risk_tolerance_alpha,
                    risk_tolerance_beta);
            dis->InitRiskTolerance(distribution_type, risk_tolerance_alpha, risk_tolerance_beta);
        }
        else {
            std::string distribution_type = "normal";
            double risk_tolerance_mean = 0.;
            double risk_tolerance_sigma = 1.;
            Log->Write("INFO:\trisk tolerance mu = %f, risk tolerance sigma = %f\n", risk_tolerance_mean,
                    risk_tolerance_sigma);
            dis->InitRiskTolerance(distribution_type, risk_tolerance_mean, risk_tolerance_sigma);
        }

        if (subroom_id==-1) { // no subroom was supplied
            startDis.push_back(dis);
        }
        else {
            startDisSub.push_back(dis);
        }

        if (_configuration->GetAgentsParameters().count(agent_para_id)==0) {
            Log->Write(
                    "ERROR:\t Please specify which set of agents parameters (agent_parameter_id) to use for the group [%d]!",
                    group_id);
            Log->Write("ERROR:\t Default values are not implemented yet");

            std::cout << "\n >> Exit with failure. See <" << _configuration->GetErrorLogFile() <<"> for details << \n";
            exit(EXIT_FAILURE);
            return false;
        }
        dis->SetGroupParameters(_configuration->GetAgentsParameters().at(agent_para_id).get());

        if (e->Attribute("startX") && e->Attribute("startY")) {
            double startX = xmltof(e->Attribute("startX"), NAN);
            double startY = xmltof(e->Attribute("startY"), NAN);
            Log->Write("INFO:\tstartX = %f, startY = %f\n", startX, startY);
            dis->SetStartPosition(startX, startY, 0.0);
        }
    }

    //Parse the sources
    TiXmlNode* xSources = xRootNode->FirstChild("agents_sources");
    if (xSources) {
         Log->Write("INFO:\tLoading sources");
         // ------ parse sources from inifile
        for (TiXmlElement* e = xSources->FirstChildElement("source"); e;
             e = e->NextSiblingElement("source")) {
             startDisSources.push_back(parseSourceNode(e));
        }//for
        TiXmlNode* xFileNode = xSources->FirstChild("file");
        //------- parse sources from external file
        if(xFileNode)
        {
             fs::path p(_configuration->GetProjectRootDir());
             std::string sourceFilename = xFileNode->FirstChild()->ValueStr();
             p /= sourceFilename;
             sourceFilename = p.string();
             Log->Write("Info:\t  Source file found <%s>", sourceFilename.c_str());
             TiXmlDocument docSource(sourceFilename);
             if (!docSource.LoadFile()) {
               Log->Write("ERROR: \t%s", docSource.ErrorDesc());
               Log->Write("ERROR: \t could not parse the sources file.");
               return false;
             }
             TiXmlElement* xRootNodeSource = docSource.RootElement();
             if (!xRootNodeSource) {
                  Log->Write("ERROR:\tRoot element does not exist in source file.");
                  return false;
             }

             if (xRootNodeSource->ValueStr() != "JPScore") {
                  Log->Write("ERROR:\tRoot element value in source file is not 'JPScore'.");
                  return false;
             }
             TiXmlNode* xSourceF = xRootNodeSource->FirstChild("agents_sources");
             if (!xSourceF) {
                  Log->Write("ERROR:\tNo agents_sources tag in file not found.");
                  return false;
             }
             Log->Write("INFO:\t  Loading sources from file");
             TiXmlNode* xSourceNodeF = xSourceF->FirstChild("source");
             if(xSourceNodeF)
             {
                  for (TiXmlElement* e = xSourceF->FirstChildElement("source"); e;
                       e = e->NextSiblingElement("source")) {
                       startDisSources.push_back(parseSourceNode(e));
                  }//for
             }
             else
                  Log->Write("Info:\t  no source info found in source file");
        } // source file found
    }//sources

    Log->Write("INFO: \t...Done");
    return true;
}

std::shared_ptr<AgentsSource> PedDistributionParser::parseSourceNode(TiXmlElement* e)
{
     int id = xmltoi(e->Attribute("id"), -1);
     int frequency = xmltoi(e->Attribute("frequency"), -1);
     int agents_max = xmltoi(e->Attribute("agents_max"), -1);
     int group_id = xmltoi(e->Attribute("group_id"), -1);
     std::string caption = xmltoa(e->Attribute("caption"), "no caption");
     std::string str_greedy = xmltoa(e->Attribute("greedy"), "false");
     float percent = xmltof(e->Attribute("percent"), 1);
     float rate = xmltof(e->Attribute("rate"), 1);
     double time = xmltof(e->Attribute("time"), 0);
     int agent_id = xmltoi(e->Attribute("agent_id"), -1);
     float startx = xmltof(e->Attribute("startX"), std::numeric_limits<float>::quiet_NaN());
     float starty = xmltof(e->Attribute("startY"), std::numeric_limits<float>::quiet_NaN());
     bool greedy = (str_greedy == "true")?true:false;
     float xmin =  xmltof(e->Attribute("x_min"), std::numeric_limits<float>::lowest());
     float xmax =  xmltof(e->Attribute("x_max"), FLT_MAX);
     float ymin =  xmltof(e->Attribute("y_min"), std::numeric_limits<float>::lowest());
     float ymax =  xmltof(e->Attribute("y_max"), FLT_MAX);
     float chunkAgents = xmltof(e->Attribute("N_create"), 1);
     int timeMin = xmltof(e->Attribute("time_min"), std::numeric_limits<int>::min());
     int timeMax = xmltof(e->Attribute("time_max"), std::numeric_limits<int>::max());
     std::vector<float> boundaries = {xmin, xmax, ymin, ymax};
     std::vector<int> lifeSpan = {timeMin, timeMax};
     float SizeBB = 1;
     bool isBigEnough = (abs(xmin-xmax) > SizeBB) && (abs(ymin-ymax) > SizeBB);
     if(!isBigEnough )
     {
          Log->Write("Warning: Source %d got too small bounding box.\n\t BB [Dx, Dy] should be such Dx>%.2f and Dy>%.2f. Ignoring BB!!", id, SizeBB, SizeBB);
          xmin = std::numeric_limits<float>::min();
          xmax = std::numeric_limits<float>::max();
          ymin = std::numeric_limits<float>::min();
          ymax = std::numeric_limits<float>::max();
     }
     if(timeMin > timeMax)
     {
          Log->Write("Warning: Source %d given wrong life span. Assuming timeMin = timeMax.", id);
          timeMin = timeMax;
     }
     if(time > 0)
     {
          timeMin = std::numeric_limits<int>::min();
          timeMax = std::numeric_limits<int>::max();
          Log->Write("Warning: Source %d. Planned time %d. Ignoring timeMin and timeMax (in case they are specified)", id, time);
     }
     if (agent_id >= 0){
          agents_max = 1;
          frequency = 1;
     }
     if(percent > 1)
     {
          Log->Write("Warning: Source %d. Passed erronuous percent <%.2f>. Set percent=1", id, percent);
          percent = 1.0;
     }
     else if(percent < 0)
     {
          Log->Write("Warning: Source %d. Passed erronuous percent <%.2f>. Set percent=0 (this source is kinda inactive)", id, percent);
          percent = 0.0;
     }
     auto source = std::shared_ptr<AgentsSource>(
          new AgentsSource(id,
                           caption,
                           agents_max,
                           group_id,
                           frequency,
                           greedy,
                           time,
                           agent_id,
                           startx,
                           starty,
                           percent,
                           rate,
                           chunkAgents,
                           boundaries,
                           lifeSpan));

     Log->Write("INFO:\t  Source with id %d will be parsed (greedy = %d)!", id, greedy);
     return source;
}
