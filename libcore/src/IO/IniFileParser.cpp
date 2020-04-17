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
// Created by laemmel on 24.03.16.
//
#include "IniFileParser.h"

#include "OutputHandler.h"
#include "direction/DirectionManager.h"
#include "direction/waiting/WaitingMiddle.h"
#include "direction/waiting/WaitingRandom.h"
#include "direction/waiting/WaitingStrategy.h"
#include "direction/walking/DirectionStrategy.h"
#include "general/Filesystem.h"
#include "general/Logger.h"
#include "general/OpenMP.h"
#include "math/GCFMModel.h"
#include "math/VelocityModel.h"
#include "pedestrian/Pedestrian.h"
#include "routing/ff_router/ffRouter.h"
#include "routing/global_shortest/GlobalRouter.h"
#include "routing/quickest/QuickestPathRouter.h"
#include "routing/smoke_router/SmokeRouter.h"

#include <stdexcept>
#include <string>
#include <tinyxml.h>

IniFileParser::IniFileParser(Configuration * config)
{
    _config = config;
}

void IniFileParser::Parse(const fs::path & iniFile)
{
    LOG_INFO("Loading and parsing the project file <{}>", iniFile.string());
    _config->SetProjectFile(
        iniFile); //TODO in some locations it is called iniFile and in others project file,
    // and as I just realized, I called it configuration. We should be consistent here anything else
    // is confusing [gl march '16]
    fs::path parentPath = iniFile.parent_path();
    if(parentPath.empty()) {
        parentPath = fs::current_path();
    }
    _config->SetProjectRootDir(fs::absolute(parentPath));

    TiXmlDocument doc(iniFile.string());
    if(!doc.LoadFile()) {
        LOG_ERROR("{}", doc.ErrorDesc());
        throw std::runtime_error("Could not parse the project file");
    }

    // everything is fine. proceed with parsing

    TiXmlElement * xMainNode = doc.RootElement();
    if(!xMainNode) {
        throw std::logic_error("Root element does not exist");
    }

    if(xMainNode->ValueStr() != "JuPedSim") {
        throw std::logic_error("Root element value is not 'JuPedSim'.");
    }

    //check the header version
    if(!xMainNode->Attribute("version")) {
        LOG_WARNING("There is no header version. I am assuming {}", JPSCORE_VERSION);
    } else if(std::stod(xMainNode->Attribute("version")) < std::stod(JPS_OLD_VERSION)) {
        throw std::logic_error(fmt::format(
            FMT_STRING("Wrong header version. Only version greater than {} is supported."),
            JPS_OLD_VERSION));
    }

    //check the structure of inifile
    if(xMainNode->FirstChild("header")) {
        TiXmlNode * xHeader = xMainNode->FirstChild("header");
        ParseHeader(xHeader);
    } //if header
    else {
        ParseHeader(xMainNode);
    } //else header


    // read walkingspeed
    std::shared_ptr<WalkingSpeed> W(new WalkingSpeed(iniFile.string()));
    _config->SetWalkingSpeed(W);
    // read  ToxicityAnalysis
    std::shared_ptr<ToxicityAnalysis> T(new ToxicityAnalysis(iniFile.string(), _config->GetFps()));
    _config->SetToxicityAnalysis(T);

    //pick up which model to use
    //get the wanted ped model id
    _model = xmltoi(xMainNode->FirstChildElement("agents")->Attribute("operational_model_id"), -1);
    if(_model == -1) {
        throw std::logic_error("Missing operational_model_id attribute in the agent section.");
    }

    bool parsingModelSuccessful = false;
    for(TiXmlElement * xModel =
            xMainNode->FirstChild("operational_models")->FirstChildElement("model");
        xModel;
        xModel = xModel->NextSiblingElement("model")) {
        if(!xModel->Attribute("description")) {
            throw std::logic_error("Missing attribute description in models?");
        }

        std::string modelName = std::string(xModel->Attribute("description"));
        int model_id          = xmltoi(xModel->Attribute("operational_model_id"), -1);

        if((_model == MODEL_GCFM) && (model_id == MODEL_GCFM)) {
            if(modelName != "gcfm") {
                throw std::logic_error("Mismatch model ID and description. Did you mean gcfm?");
            }
            if(!ParseGCFMModel(xModel, xMainNode))
                throw std::logic_error("Error parsing GCFM model parameters.");

            parsingModelSuccessful = true;
            //only parsing one model
            break;
        } else if((_model == MODEL_VELOCITY) && (model_id == MODEL_VELOCITY)) {
            if(modelName != "Tordeux2015") {
                throw std::logic_error(
                    "Mismatch model ID and description. Did you mean Tordeux2015?");
            }
            //only parsing one model
            if(!ParseVelocityModel(xModel, xMainNode))
                throw std::logic_error("Error parsing Velocity model parameters.");
            parsingModelSuccessful = true;
            break;
        }
    }

    if(!parsingModelSuccessful) {
        LOG_ERROR("Wrong model id [{}]. Choose 1 (GCFM) or 3 (Tordeux2015)", _model);
        LOG_ERROR(
            "Please make sure that all models are specified in the operational_models section");
        LOG_ERROR("And make sure to use the same ID in the agent section");
        throw std::logic_error("Parsing Model Failed.");
    }

    //route choice strategy
    TiXmlNode * xRouters     = xMainNode->FirstChild("route_choice_models");
    TiXmlNode * xAgentDistri = xMainNode->FirstChild("agents")->FirstChild("agents_distribution");

    if(!ParseRoutingStrategies(xRouters, xAgentDistri))
        throw std::logic_error("Error while parsing routing strategies.");

    ParseExternalFiles(*xMainNode);
    LOG_INFO("Parsing the project file completed");
}

bool IniFileParser::ParseHeader(TiXmlNode * xHeader)
{
    //seed
    if(xHeader->FirstChild("seed")) {
        TiXmlNode * seedNode = xHeader->FirstChild("seed")->FirstChild();
        if(seedNode) {
            const char * seedValue = seedNode->Value();
            _config->SetSeed((unsigned int) atoi(seedValue)); //strtol
        } else {
            _config->SetSeed((unsigned int) time(NULL));
        }
    }
    LOG_INFO("Random seed <{}>", _config->GetSeed());

    // max simulation time
    if(xHeader->FirstChild("max_sim_time")) {
        const char * tmax = xHeader->FirstChildElement("max_sim_time")->FirstChild()->Value();
        _config->SetTmax(atof(tmax));
        LOG_INFO("Maximal simulation time <{:.2f}> seconds", _config->GetTmax());
    }

    // geometry file name
    if(xHeader->FirstChild("geometry")) {
        std::string filename = xHeader->FirstChild("geometry")->FirstChild()->Value();
        _config->SetGeometryFile(filename);
        LOG_INFO("Geometry file <{}>", filename);
    }


    //max CPU
    int max_threads = 1;
#ifdef _OPENMP
    max_threads = omp_get_max_threads();
#endif
    if(xHeader->FirstChild("num_threads")) {
        TiXmlNode * numthreads = xHeader->FirstChild("num_threads")->FirstChild();
        if(numthreads) {
#ifdef _OPENMP
            omp_set_num_threads(xmltoi(numthreads->Value()));
#endif
        }
    }
    _config->SetMaxOpenMPThreads(omp_get_max_threads());
    LOG_INFO("Using {} OpenMP threads, {} available.", _config->GetMaxOpenMPThreads(), max_threads);

    //display statistics
    if(xHeader->FirstChild("show_statistics")) {
        std::string value = xHeader->FirstChild("show_statistics")->FirstChild()->Value();
        _config->SetShowStatistics(value == "true");
        LOG_INFO("Show statistics: {}", value);
    }

    // Results Output Path
    auto * xmlOutput = xHeader->FirstChildElement("output");
    if(xmlOutput != nullptr) {
        auto * xmlOutputPath = xmlOutput->Attribute("path");
        if(xmlOutputPath != nullptr) {
            _config->SetOutputPath(xmlOutputPath);
        }
    }
    _config->ConfigureOutputPath();
    LOG_INFO("Output Path configured <{}>", _config->GetOutputPath().string());

    //trajectories
    TiXmlNode * xTrajectories = xHeader->FirstChild("trajectories");
    if(xTrajectories) {
        double fps;
        xHeader->FirstChildElement("trajectories")->Attribute("fps", &fps);
        _config->SetFps(fps);

        std::string format = xHeader->FirstChildElement("trajectories")->Attribute("format") ?
                                 xHeader->FirstChildElement("trajectories")->Attribute("format") :
                                 "plain";
        std::transform(format.begin(), format.end(), format.begin(), ::tolower);

        if(format == "plain") {
            _config->SetFileFormat(FileFormat::TXT);
        } else {
            LOG_WARNING("no output format specified. Using default: TXT");
            _config->SetFileFormat(FileFormat::TXT);
        }

        //color mode
        std::string color_mode =
            xHeader->FirstChildElement("trajectories")->Attribute("color_mode") ?
                xHeader->FirstChildElement("trajectories")->Attribute("color_mode") :
                "velocity";

        if(color_mode == "velocity")
            Pedestrian::SetColorMode(
                AgentColorMode::
                    BY_VELOCITY); //TODO: config parameter! does not belong to the pedestrian model, we should create a pedestrian config instead. [gl march '16]
        if(color_mode == "spotlight")
            Pedestrian::SetColorMode(AgentColorMode::BY_SPOTLIGHT);
        if(color_mode == "group")
            Pedestrian::SetColorMode(AgentColorMode::BY_GROUP);
        if(color_mode == "knowledge")
            Pedestrian::SetColorMode(AgentColorMode::BY_KNOWLEDGE);
        if(color_mode == "router")
            Pedestrian::SetColorMode(AgentColorMode::BY_ROUTER);
        if(color_mode == "final_goal")
            Pedestrian::SetColorMode(AgentColorMode::BY_FINAL_GOAL);
        if(color_mode == "intermediate_goal")
            Pedestrian::SetColorMode(AgentColorMode::BY_INTERMEDIATE_GOAL);

        fs::path trajectoryFile = _config->GetTrajectoriesFile();
        //a file descriptor was given
        if(xTrajectories->FirstChild("file")) {
            const fs::path trajLoc(xTrajectories->FirstChildElement("file")->Attribute("location"));
            if(!trajLoc.empty()) {
                trajectoryFile = trajLoc;
            }
        }
        std::string extension =
            (trajectoryFile.has_extension()) ? (trajectoryFile.extension().string()) : ("");
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

        // At the moment we only suport plain txt file format.
        // Check file extension and if it is not txt, change it to correct one
        if(extension != ".txt") {
            trajectoryFile.replace_extension(".txt");
            LOG_WARNING("replaced output file extension with: .txt");
        }

        fs::path canonicalTrajPath =
            fs::weakly_canonical(_config->GetOutputPath() / trajectoryFile);

        _config->SetTrajectoriesFile(canonicalTrajPath);
        _config->SetOriginalTrajectoriesFile(canonicalTrajPath);

        LOG_INFO("Output file  <{}>", _config->GetTrajectoriesFile().string());
        LOG_INFO("In format <{}> at <{:.0f}> frames per seconds", format, _config->GetFps());


        if(xTrajectories->FirstChild("optional_output")) {
            LOG_WARNING("These optional options do only work with plain output format!");

            auto node = xTrajectories->FirstChildElement("optional_output");
            //check if speed is wanted
            if(const char * attribute = node->Attribute("speed"); attribute) {
                std::string in = xmltoa(attribute, "false");
                std::transform(in.begin(), in.end(), in.begin(), ::tolower);

                if(in == "true") {
                    _config->AddOptionalOutputOption(OptionalOutput::speed);
                    LOG_INFO("speed added to output");
                } else {
                    LOG_INFO("speed not added to output");
                }
            }

            //check if velocity is wanted
            if(const char * attribute = node->Attribute("velocity"); attribute) {
                std::string in = xmltoa(attribute, "false");
                std::transform(in.begin(), in.end(), in.begin(), ::tolower);

                if(in == "true") {
                    _config->AddOptionalOutputOption(OptionalOutput::velocity);
                    LOG_INFO("velocity added to output");
                } else {
                    LOG_INFO("velocity not added to output");
                }
            }

            //check if final_goal is wanted
            if(const char * attribute = node->Attribute("final_goal"); attribute) {
                std::string in = xmltoa(attribute, "false");
                std::transform(in.begin(), in.end(), in.begin(), ::tolower);

                if(in == "true") {
                    _config->AddOptionalOutputOption(OptionalOutput::final_goal);
                    LOG_INFO("final_goal added to output");
                } else {
                    LOG_INFO("final_goal not added to output");
                }
            }

            //check if intermediate_goal is wanted
            if(const char * attribute = node->Attribute("intermediate_goal"); attribute) {
                std::string in = xmltoa(attribute, "false");
                std::transform(in.begin(), in.end(), in.begin(), ::tolower);

                if(in == "true") {
                    _config->AddOptionalOutputOption(OptionalOutput::intermediate_goal);
                    LOG_INFO("intermediate_goal added to output");
                } else {
                    LOG_INFO("intermediate_goal not added to output");
                }
            }

            //check if desired_direction is wanted
            if(const char * attribute = node->Attribute("desired_direction"); attribute) {
                std::string in = xmltoa(attribute, "false");
                std::transform(in.begin(), in.end(), in.begin(), ::tolower);

                if(in == "true") {
                    _config->AddOptionalOutputOption(OptionalOutput::desired_direction);
                    LOG_INFO("desired_direction added to output");
                } else {
                    LOG_INFO("desired_direction not added to output");
                }
            }

            //check if spotlight is wanted
            if(const char * attribute = node->Attribute("spotlight"); attribute) {
                std::string in = xmltoa(attribute, "false");
                std::transform(in.begin(), in.end(), in.begin(), ::tolower);

                if(in == "true") {
                    _config->AddOptionalOutputOption(OptionalOutput::spotlight);
                    LOG_INFO("spotlight added to output");
                } else {
                    LOG_INFO("spotlight not added to output");
                }
            }

            //check if router is wanted
            if(const char * attribute = node->Attribute("router"); attribute) {
                std::string in = xmltoa(attribute, "false");
                std::transform(in.begin(), in.end(), in.begin(), ::tolower);

                if(in == "true") {
                    _config->AddOptionalOutputOption(OptionalOutput::router);
                    LOG_INFO("router added to output");
                } else {
                    LOG_INFO("router not added to output");
                }
            }

            //check if router is wanted
            if(const char * attribute = node->Attribute("group"); attribute) {
                std::string in = xmltoa(attribute, "false");
                std::transform(in.begin(), in.end(), in.begin(), ::tolower);

                if(in == "true") {
                    _config->AddOptionalOutputOption(OptionalOutput::group);
                    LOG_INFO("group added to output");
                } else {
                    LOG_INFO("group not added to output");
                }
            }
        }
    }

    return true;
}

bool IniFileParser::ParseGCFMModel(TiXmlElement * xGCFM, TiXmlElement * xMainNode)
{
    LOG_INFO("Using the GCFM model");
    LOG_INFO("Parsing the model parameters");

    TiXmlNode * xModelPara = xGCFM->FirstChild("model_parameters");
    if(!xModelPara) {
        LOG_ERROR("!!!! Changes in the operational model section !!!");
        LOG_ERROR("!!!! The new version is in inputfiles/ship_msw/ini_ship2.xml !!!");
        return false;
    }

    // For convenience. This moved to the header as it is not model specific
    if(xModelPara->FirstChild("tmax")) {
        LOG_ERROR("The maximal simulation time section moved to the header!!!");
        LOG_ERROR("\t <max_sim_time> </max_sim_time>\n");
        return false;
    }

    //stepsize
    if(!ParseStepSize(*xModelPara))
        return false;

    //exit crossing strategy
    if(!ParseStrategyNodeToObject(*xModelPara))
        return false;

    //linked-cells
    if(!ParseLinkedCells(*xModelPara))
        return false;

    //force_ped
    if(xModelPara->FirstChild("force_ped")) {
        std::string nu       = xModelPara->FirstChildElement("force_ped")->Attribute("nu");
        std::string dist_max = xModelPara->FirstChildElement("force_ped")->Attribute("dist_max");
        std::string disteff_max =
            xModelPara->FirstChildElement("force_ped")
                ->Attribute("disteff_max"); // @todo: rename disteff_max to force_max
        std::string interpolation_width =
            xModelPara->FirstChildElement("force_ped")->Attribute("interpolation_width");

        _config->SetMaxFPed(std::stod(dist_max));
        _config->SetNuPed(std::stod(nu));
        _config->SetDistEffMaxPed(std::stod(disteff_max));
        _config->SetIntPWidthPed(std::stod(interpolation_width));
        LOG_INFO(
            "Frep_ped nu={:.3f}, dist_max={:.3f}, disteff_max={:.3f}, "
            "interpolation_width={:.3f}",
            std::stod(nu),
            std::stod(dist_max),
            std::stod(disteff_max),
            std::stod(interpolation_width));
    }

    //force_wall
    if(xModelPara->FirstChild("force_wall")) {
        std::string nu       = xModelPara->FirstChildElement("force_wall")->Attribute("nu");
        std::string dist_max = xModelPara->FirstChildElement("force_wall")->Attribute("dist_max");
        std::string disteff_max =
            xModelPara->FirstChildElement("force_wall")->Attribute("disteff_max");
        std::string interpolation_width =
            xModelPara->FirstChildElement("force_wall")->Attribute("interpolation_width");
        _config->SetMaxFWall(std::stod(dist_max));
        _config->SetNuWall(std::stod(nu));
        _config->SetDistEffMaxWall(std::stod(disteff_max));
        _config->SetIntPWidthWall(std::stod(interpolation_width));
        LOG_INFO(
            "Frep_wall mu={:.3f}, dist_max={:.3f}, disteff_max={:.3f}, "
            "interpolation_width={:.3f}",
            std::stod(nu),
            std::stod(dist_max),
            std::stod(disteff_max),
            std::stod(interpolation_width));
    }

    //Parsing the agent parameters
    TiXmlNode * xAgentDistri = xMainNode->FirstChild("agents")->FirstChild("agents_distribution");
    ParseAgentParameters(xGCFM, xAgentDistri);

    //TODO: models do not belong in a configuration container [gl march '16]
    _config->SetModel(std::shared_ptr<OperationalModel>(new GCFMModel(
        _directionManager,
        _config->GetNuPed(),
        _config->GetNuWall(),
        _config->GetDistEffMaxPed(),
        _config->GetDistEffMaxWall(),
        _config->GetIntPWidthPed(),
        _config->GetIntPWidthWall(),
        _config->GetMaxFPed(),
        _config->GetMaxFWall())));

    return true;
}

bool IniFileParser::ParseVelocityModel(TiXmlElement * xVelocity, TiXmlElement * xMainNode)
{
    //parsing the model parameters
    LOG_INFO("Using Tordeux2015 model");
    LOG_INFO("Parsing the model parameters");

    TiXmlNode * xModelPara = xVelocity->FirstChild("model_parameters");

    if(!xModelPara) {
        LOG_ERROR("!!!! Changes in the operational model section !!!");
        LOG_ERROR("!!!! The new version is in inputfiles/ship_msw/ini_ship3.xml !!!");
        return false;
    }

    // For convenience. This moved to the header as it is not model specific
    if(xModelPara->FirstChild("tmax")) {
        LOG_ERROR("The maximal simulation time section moved to the header!!!");
        LOG_ERROR("\t <max_sim_time> </max_sim_time>\n");
        return false;
    }

    //stepsize
    if(!ParseStepSize(*xModelPara))
        return false;

    //exit crossing strategy
    if(!ParseStrategyNodeToObject(*xModelPara))
        return false;

    //linked-cells
    if(!ParseLinkedCells(*xModelPara))
        return false;

    //periodic
    if(!ParsePeriodic(*xModelPara))
        return false;

    //force_ped
    if(xModelPara->FirstChild("force_ped")) {
        if(!xModelPara->FirstChildElement("force_ped")->Attribute("a"))
            _config->SetaPed(1.0); // default value
        else {
            std::string a = xModelPara->FirstChildElement("force_ped")->Attribute("a");
            _config->SetaPed(std::stod(a));
        }

        if(!xModelPara->FirstChildElement("force_ped")->Attribute("D"))
            _config->SetDPed(0.1); // default value in [m]
        else {
            std::string D = xModelPara->FirstChildElement("force_ped")->Attribute("D");
            _config->SetDPed(std::stod(D));
        }
        LOG_INFO("Frep_ped a={:.2f}, D={:.2f}", _config->GetaPed(), _config->GetDPed());
    }
    //force_wall
    if(xModelPara->FirstChild("force_wall")) {
        if(!xModelPara->FirstChildElement("force_wall")->Attribute("a"))
            _config->SetaWall(1.0); // default value
        else {
            std::string a = xModelPara->FirstChildElement("force_wall")->Attribute("a");
            _config->SetaWall(std::stod(a));
        }

        if(!xModelPara->FirstChildElement("force_wall")->Attribute("D"))
            _config->SetDWall(0.1); // default value in [m]
        else {
            std::string D = xModelPara->FirstChildElement("force_wall")->Attribute("D");
            _config->SetDWall(std::stod(D));
        }
        LOG_INFO("Frep_wall a={:.2f}, D={:.2f}", _config->GetaWall(), _config->GetDWall());
    }

    //Parsing the agent parameters
    TiXmlNode * xAgentDistri = xMainNode->FirstChild("agents")->FirstChild("agents_distribution");
    ParseAgentParameters(xVelocity, xAgentDistri);
    _config->SetModel(std::shared_ptr<OperationalModel>(new VelocityModel(
        _directionManager,
        _config->GetaPed(),
        _config->GetDPed(),
        _config->GetaWall(),
        _config->GetDWall())));

    return true;
}

void IniFileParser::ParseAgentParameters(TiXmlElement * operativModel, TiXmlNode * agentsDistri)
{
    //Parsing the agent parameters
    LOG_INFO("Parsing agents  parameters");
    //first get list of actually used router
    std::vector<int> usedAgentParams;
    usedAgentParams.clear();
    for(TiXmlElement * e = agentsDistri->FirstChildElement("group"); e;
        e                = e->NextSiblingElement("group")) {
        int agentsParams = -1;
        if(e->Attribute("agent_parameter_id")) {
            agentsParams = atoi(e->Attribute("agent_parameter_id"));
            if(std::find(usedAgentParams.begin(), usedAgentParams.end(), agentsParams) ==
               usedAgentParams.end()) {
                usedAgentParams.emplace_back(agentsParams);
            }
        }
    }
    for(TiXmlElement * xAgentPara = operativModel->FirstChildElement("agent_parameters");
        xAgentPara;
        xAgentPara = xAgentPara->NextSiblingElement("agent_parameters")) {
        //get the group ID
        int para_id = xmltoi(xAgentPara->Attribute("agent_parameter_id"), -1);
        if(std::find(usedAgentParams.begin(), usedAgentParams.end(), para_id) !=
           usedAgentParams.end()) {
            LOG_INFO("Parsing the group parameter id [{}]", para_id);

            auto agentParameters = std::shared_ptr<AgentsParameters>(
                new AgentsParameters(para_id, _config->GetSeed()));
            _config->AddAgentsParameters(agentParameters, para_id);

            //desired speed
            if(xAgentPara->FirstChild("v0")) {
                double mu    = xmltof(xAgentPara->FirstChildElement("v0")->Attribute("mu"));
                double sigma = xmltof(xAgentPara->FirstChildElement("v0")->Attribute("sigma"));
                agentParameters->InitV0(mu, sigma);
                agentParameters->InitV0DownStairs(mu, sigma);
                agentParameters->InitV0UpStairs(mu, sigma);
                LOG_INFO("Desired speed mu={} , sigma={}", mu, sigma);
            }

            if(xAgentPara->FirstChild("v0_upstairs")) {
                double mu = xmltof(xAgentPara->FirstChildElement("v0_upstairs")->Attribute("mu"));
                double sigma =
                    xmltof(xAgentPara->FirstChildElement("v0_upstairs")->Attribute("sigma"));
                agentParameters->InitV0UpStairs(mu, sigma);
                LOG_INFO("Desired speed upstairs mu={} , sigma={}", mu, sigma);
            }

            if(xAgentPara->FirstChild("v0_downstairs")) {
                double mu = xmltof(xAgentPara->FirstChildElement("v0_downstairs")->Attribute("mu"));
                double sigma =
                    xmltof(xAgentPara->FirstChildElement("v0_downstairs")->Attribute("sigma"));
                agentParameters->InitV0DownStairs(mu, sigma);
                LOG_INFO("Desired speed downstairs mu={} , sigma={}", mu, sigma);
            } //------------------------------------------------------------------------
            if(xAgentPara->FirstChild("escalator_upstairs")) {
                double mu =
                    xmltof(xAgentPara->FirstChildElement("escalator_upstairs")->Attribute("mu"));
                double sigma =
                    xmltof(xAgentPara->FirstChildElement("escalator_upstairs")->Attribute("sigma"));
                agentParameters->InitEscalatorUpStairs(mu, sigma);
                LOG_INFO("speed of escalator upstairs mu={} , sigma={}", mu, sigma);
            }
            if(xAgentPara->FirstChild("escalator_downstairs")) {
                double mu =
                    xmltof(xAgentPara->FirstChildElement("escalator_downstairs")->Attribute("mu"));
                double sigma = xmltof(
                    xAgentPara->FirstChildElement("escalator_downstairs")->Attribute("sigma"));
                agentParameters->InitEscalatorDownStairs(mu, sigma);
                LOG_INFO("speed of escalator downstairs mu={} , sigma={}", mu, sigma);
            }
            if(xAgentPara->FirstChild("v0_idle_escalator_upstairs")) {
                double mu = xmltof(
                    xAgentPara->FirstChildElement("v0_idle_escalator_upstairs")->Attribute("mu"));
                double sigma = xmltof(xAgentPara->FirstChildElement("v0_idle_escalator_upstairs")
                                          ->Attribute("sigma"));
                agentParameters->InitV0IdleEscalatorUpStairs(mu, sigma);
                LOG_INFO("Desired speed idle escalator upstairs mu={} , sigma={}", mu, sigma);
            }
            if(xAgentPara->FirstChild("v0_idle_escalator_downstairs")) {
                double mu = xmltof(
                    xAgentPara->FirstChildElement("v0_idle_escalator_downstairs")->Attribute("mu"));
                double sigma = xmltof(xAgentPara->FirstChildElement("v0_idle_escalator_downstairs")
                                          ->Attribute("sigma"));
                agentParameters->InitV0IdleEscalatorDownStairs(mu, sigma);
                LOG_INFO("Desired speed idle escalator downstairs mu={} , sigma={}", mu, sigma);
            }
            //bmax
            if(xAgentPara->FirstChild("bmax")) {
                double mu    = xmltof(xAgentPara->FirstChildElement("bmax")->Attribute("mu"));
                double sigma = xmltof(xAgentPara->FirstChildElement("bmax")->Attribute("sigma"));
                agentParameters->InitBmax(mu, sigma);
                LOG_INFO("Bmax mu={} , sigma={}", mu, sigma);
            }

            //bmin
            if(xAgentPara->FirstChild("bmin")) {
                double mu    = xmltof(xAgentPara->FirstChildElement("bmin")->Attribute("mu"));
                double sigma = xmltof(xAgentPara->FirstChildElement("bmin")->Attribute("sigma"));
                agentParameters->InitBmin(mu, sigma);
                LOG_INFO("Bmin mu={} , sigma={}", mu, sigma);
            }

            //amin
            if(xAgentPara->FirstChild("amin")) {
                double mu    = xmltof(xAgentPara->FirstChildElement("amin")->Attribute("mu"));
                double sigma = xmltof(xAgentPara->FirstChildElement("amin")->Attribute("sigma"));
                agentParameters->InitAmin(mu, sigma);
                LOG_INFO("Amin mu={} , sigma={}", mu, sigma);
            }
            //tau
            if(xAgentPara->FirstChild("tau")) {
                double mu    = xmltof(xAgentPara->FirstChildElement("tau")->Attribute("mu"));
                double sigma = xmltof(xAgentPara->FirstChildElement("tau")->Attribute("sigma"));
                agentParameters->InitTau(mu, sigma);
                LOG_INFO("Tau mu={} , sigma={}", mu, sigma);
            }
            //atau
            if(xAgentPara->FirstChild("atau")) {
                double mu    = xmltof(xAgentPara->FirstChildElement("atau")->Attribute("mu"));
                double sigma = xmltof(xAgentPara->FirstChildElement("atau")->Attribute("sigma"));
                agentParameters->InitAtau(mu, sigma);
                LOG_INFO("Atau mu={} , sigma={}", mu, sigma);
            }
            // T
            if(xAgentPara->FirstChild("T")) {
                double mu    = xmltof(xAgentPara->FirstChildElement("T")->Attribute("mu"));
                double sigma = xmltof(xAgentPara->FirstChildElement("T")->Attribute("sigma"));
                agentParameters->InitT(mu, sigma);
                LOG_INFO("T mu={} , sigma={}", mu, sigma);
            }

            if(_model == 3) { // Tordeux2015
                double max_Eb = 2 * agentParameters->GetBmax();
                _config->SetDistEffMaxPed(
                    max_Eb + agentParameters->GetT() * agentParameters->GetV0());
                _config->SetDistEffMaxWall(_config->GetDistEffMaxPed());
            }
        }
    }
}

bool IniFileParser::ParseRoutingStrategies(TiXmlNode * routingNode, TiXmlNode * agentsDistri)
{
    if(!routingNode) {
        LOG_ERROR("Route_choice_models section is missing");
        return false;
    }

    if(!agentsDistri) {
        LOG_ERROR("Agent Distribution section is missing");
        return false;
    }
    //first get list of actually used router
    std::vector<int> usedRouter;
    usedRouter.clear();
    bool hasSpecificGoals = false;
    for(TiXmlElement * e = agentsDistri->FirstChildElement("group"); e;
        e                = e->NextSiblingElement("group")) {
        int router = -1;
        if(e->Attribute("router_id")) {
            router = atoi(e->Attribute("router_id"));
            if(std::find(usedRouter.begin(), usedRouter.end(), router) == usedRouter.end()) {
                usedRouter.emplace_back(router);
            }
        }
        int goal = -1;
        if(e->Attribute("goal_id")) {
            goal = atoi(e->Attribute("goal_id"));
            if(goal != -1) {
                hasSpecificGoals = true;
            }
        }
    }
    _config->set_has_specific_goals(hasSpecificGoals);
    for(TiXmlElement * e = routingNode->FirstChildElement("router"); e;
        e                = e->NextSiblingElement("router")) {
        std::string strategy = e->Attribute("description");
        int id               = atoi(e->Attribute("router_id"));

        if((strategy == "local_shortest") &&
           (std::find(usedRouter.begin(), usedRouter.end(), id) != usedRouter.end())) {
            Router * r = new GlobalRouter(id, ROUTING_LOCAL_SHORTEST);
            _config->GetRoutingEngine()->AddRouter(r);
        } else if(
            (strategy == "global_shortest") &&
            (std::find(usedRouter.begin(), usedRouter.end(), id) != usedRouter.end())) {
            Router * r = new GlobalRouter(id, ROUTING_GLOBAL_SHORTEST);
            _config->GetRoutingEngine()->AddRouter(r);
        } else if(
            (strategy == "quickest") &&
            (std::find(usedRouter.begin(), usedRouter.end(), id) != usedRouter.end())) {
            Router * r = new QuickestPathRouter(id, ROUTING_QUICKEST);
            _config->GetRoutingEngine()->AddRouter(r);
        } else if(
            (strategy == "smoke") &&
            (std::find(usedRouter.begin(), usedRouter.end(), id) != usedRouter.end())) {
            Router * r = new SmokeRouter(id, ROUTING_SMOKE);
            _config->GetRoutingEngine()->AddRouter(r);

            LOG_INFO("Using SmokeRouter");
            ///Parsing additional options
            if(!ParseCogMapOpts(e))
                return false;
        } else if(
            (strategy == "ff_global_shortest") &&
            (std::find(usedRouter.begin(), usedRouter.end(), id) != usedRouter.end())) {
            Router * r = new FFRouter(id, ROUTING_FF_GLOBAL_SHORTEST, hasSpecificGoals, _config);
            _config->GetRoutingEngine()->AddRouter(r);

            if((_exit_strat_number == 8) || (_exit_strat_number == 9)) {
                LOG_INFO("Using FF Global Shortest Router");
            } else {
                LOG_WARNING("Exit Strategy Number is not 8 or 9!!!");
                // config object holds default values, so we omit any set operations
            }

            ///Parsing additional options
            if(!ParseFfRouterOps(e, ROUTING_FF_GLOBAL_SHORTEST)) {
                return false;
            }
        } else if(
            (strategy == "ff_quickest") &&
            (std::find(usedRouter.begin(), usedRouter.end(), id) != usedRouter.end())) {
            Router * r = new FFRouter(id, ROUTING_FF_QUICKEST, hasSpecificGoals, _config);
            _config->GetRoutingEngine()->AddRouter(r);
            LOG_INFO("Using FF Quickest Router");

            if(!ParseFfRouterOps(e, ROUTING_FF_QUICKEST)) {
                return false;
            }
        } else if(std::find(usedRouter.begin(), usedRouter.end(), id) != usedRouter.end()) {
            LOG_ERROR("Wrong value for routing strategy [{}].", strategy);
            return false;
        }
    }
    return true;
}

bool IniFileParser::ParseFfRouterOps(TiXmlNode * routingNode, RoutingStrategy s)
{
    //set defaults
    if(s == ROUTING_FF_QUICKEST) {
        //parse ini-file-information
        if(routingNode->FirstChild("parameters")) {
            TiXmlNode * pParameters = routingNode->FirstChild("parameters");
            if(pParameters->FirstChild(
                   "recalc_interval")) { //@todo: ar.graf: test ini file with recalc value
                _config->set_recalc_interval(
                    atof(pParameters->FirstChild("recalc_interval")->FirstChild()->Value()));
            }
        }
    }
    _config->set_write_VTK_files(false);
    if(routingNode->FirstChild("parameters")) {
        TiXmlNode * pParametersForAllFF = routingNode->FirstChild("parameters");
        if(pParametersForAllFF->FirstChild("write_VTK_files")) {
            bool tmp_write_VTK = !std::strcmp(
                pParametersForAllFF->FirstChild("write_VTK_files")->FirstChild()->Value(), "true");
            _config->set_write_VTK_files(tmp_write_VTK);
        }
    }
    return true;
}

bool IniFileParser::ParseCogMapOpts(TiXmlNode * routingNode)
{
    TiXmlNode * sensorNode = routingNode->FirstChild();
    if(!sensorNode) {
        LOG_ERROR("No sensors found.\n");
        return false;
    }

    /// static_cast to get access to the method 'addOption' of the SmokeRouter
    SmokeRouter * r =
        static_cast<SmokeRouter *>(_config->GetRoutingEngine()->GetAvailableRouters().back());

    std::vector<std::string> sensorVec;
    for(TiXmlElement * e = sensorNode->FirstChildElement("sensor"); e;
        e                = e->NextSiblingElement("sensor")) {
        std::string sensor = e->Attribute("description");
        //adding Smoke Sensor specific parameters is executed in the class FDSFIreMeshStorage
        sensorVec.push_back(sensor);

        LOG_INFO("Sensor <{}> added.", sensor);
    }

    r->addOption("Sensors", sensorVec);

    TiXmlElement * cogMap = routingNode->FirstChildElement("cognitive_map");

    if(!cogMap) {
        LOG_ERROR("Cognitive Map not specified.\n");
        return false;
    }

    std::vector<std::string> cogMapStatus;
    cogMapStatus.push_back(cogMap->Attribute("status"));
    LOG_INFO("All pedestrian starting with a(n) {} cognitive maps", cogMapStatus[0]);
    r->addOption("CognitiveMap", cogMapStatus);

    std::vector<std::string> cogMapFiles;
    if(!cogMap->Attribute("files")) {
        LOG_WARNING("No input files for the cognitive map specified!");
        return true;
    }
    cogMapFiles.push_back(cogMap->Attribute("files"));
    r->addOption("CognitiveMapFiles", cogMapFiles);

    return true;
}

bool IniFileParser::ParseLinkedCells(const TiXmlNode & linkedCellNode)
{
    if(linkedCellNode.FirstChild("linkedcells")) {
        std::string linkedcells =
            linkedCellNode.FirstChildElement("linkedcells")->Attribute("enabled");
        std::string cell_size =
            linkedCellNode.FirstChildElement("linkedcells")->Attribute("cell_size");

        if(linkedcells == "true") {
            _config->SetLinkedCellSize(std::stod(cell_size));
            LOG_INFO("Linked cells enabled with size  <{:.2f}>", _config->GetLinkedCellSize());
            return true;
        } else {
            _config->SetLinkedCellSize(-1.0);
            LOG_WARNING("Invalid parameters for linkedcells");
            return false;
        }
    }
    return false;
}

bool IniFileParser::ParseStepSize(TiXmlNode & stepNode)
{
    if(stepNode.FirstChild("stepsize")) {
        const char * stepsize = stepNode.FirstChild("stepsize")->FirstChild()->Value();
        if(stepsize) {
            double tmp         = 1. / _config->GetFps();
            double stepsizeDBL = atof(stepsize);
            if((stepNode.FirstChildElement("stepsize")->Attribute("fix")) &&
               (std::string(stepNode.FirstChildElement("stepsize")->Attribute("fix")) == "yes")) {
                _config->Setdt(atof(stepsize));
                LOG_INFO("Stepsize <{}>", _config->Getdt());
                if(tmp < _config->Getdt()) {
                    LOG_WARNING("Stepsize dt = {} > {} = frameinterval.", _config->Getdt(), tmp);
                    LOG_WARNING("You should decrease stepsize or fps!");
                }
                return true;
            }
            //find a stepsize, that can be multiplied by (int) to get framerate
            for(int i = 1; i < 2000; ++i) {
                if((tmp / i) <= stepsizeDBL) {
                    _config->Setdt(tmp / i);
                    if((tmp / i) < stepsizeDBL) {
                        LOG_WARNING(
                            "Decreased stepsize from <{}> to <{}> to match fps",
                            stepsizeDBL,
                            (tmp / i));
                    }
                    LOG_INFO("Stepsize <{}>", _config->Getdt());
                    return true;
                }
            }
            //below should never execute
            _config->Setdt(stepsizeDBL);
            LOG_INFO("Stepsize <{}>", _config->Getdt());
            if(tmp < _config->Getdt()) {
                LOG_WARNING("Stepsize dt = {} > {} = frameinterval.", _config->Getdt(), tmp);
                LOG_WARNING("You should decrease stepsize or fps!");
            }
            return true;
        }
    }
    return false;
}

bool IniFileParser::ParsePeriodic(TiXmlNode & Node)
{
    if(Node.FirstChild("periodic")) {
        const char * periodic = Node.FirstChild("periodic")->FirstChild()->Value();
        if(periodic)
            _config->SetIsPeriodic(atoi(periodic));
        LOG_INFO("Periodic <{}>", _config->IsPeriodic());
        return true;
    } else {
        _config->SetIsPeriodic(0);
    }
    return true; //default is periodic=0. If not specified than is OK
}

bool IniFileParser::ParseStrategyNodeToObject(const TiXmlNode & strategyNode)
{
    // Init DirectionManager
    _directionManager = std::shared_ptr<DirectionManager>(new DirectionManager);

    std::string query = "exit_crossing_strategy";
    if(!strategyNode.FirstChild(query.c_str())) {
        query = "exitCrossingStrategy";
        LOG_ERROR("The keyword exitCrossingStrategy is deprecated. Please consider using "
                  "\"exit_crossing_strategy\" in the ini file");
        return false;
    }

    if(strategyNode.FirstChild(query.c_str())) {
        const char * tmp = strategyNode.FirstChild(query.c_str())->FirstChild()->Value();
        int pExitStrategy;
        if(tmp) {
            pExitStrategy = atoi(tmp);

            //check for ff router to avoid exit strat <> router mismatch
            const TiXmlNode * agentsDistri =
                strategyNode.GetDocument()->RootElement()->FirstChild("agents")->FirstChild(
                    "agents_distribution");
            std::vector<int> usedRouter;
            for(const TiXmlElement * e = agentsDistri->FirstChildElement("group"); e;
                e                      = e->NextSiblingElement("group")) {
                int router = -1;
                if(e->Attribute("router_id")) {
                    router = atoi(e->Attribute("router_id"));
                    if(std::find(usedRouter.begin(), usedRouter.end(), router) ==
                       usedRouter.end()) {
                        usedRouter.emplace_back(router);
                    }
                }
            }
            //continue: check for ff router to avoid exit strat <> router mismatch
            const TiXmlNode * routeChoice =
                strategyNode.GetDocument()->RootElement()->FirstChild("route_choice_models");
            for(const TiXmlElement * e = routeChoice->FirstChildElement("router"); e;
                e                      = e->NextSiblingElement("router")) {
                int router_id = atoi(e->Attribute("router_id"));
                if(!(std::find(usedRouter.begin(), usedRouter.end(), router_id) ==
                     usedRouter.end())) {
                    std::string router_descr = e->Attribute("description");
                    if((pExitStrategy != 9) && (pExitStrategy != 8) &&
                       ((router_descr == "ff_global_shortest") ||
                        (router_descr == "ff_local_shortest") || (router_descr == "ff_quickest"))) {
                        pExitStrategy = 8;
                        LOG_WARNING("Changing Exit Strategie to work with floorfield!");
                    }
                }
            }
            _exit_strat_number = pExitStrategy;

            if(pExitStrategy == 8 || pExitStrategy == 9) {
                _config->set_write_VTK_files_direction(false);
                if(strategyNode.FirstChild("write_VTK_files")) {
                    const char * tmp1 =
                        strategyNode.FirstChild("write_VTK_files")->FirstChild()->Value();
                    bool tmp_write_VTK = !std::strcmp(tmp1, "true");
                    _config->set_write_VTK_files_direction(tmp_write_VTK);
                }
            }
            switch(pExitStrategy) {
                case 1:
                    _directionStrategy =
                        std::shared_ptr<DirectionStrategy>(new DirectionMiddlePoint());
                    break;
                case 2:
                    _directionStrategy =
                        std::shared_ptr<DirectionStrategy>(new DirectionMinSeperationShorterLine());
                    break;
                case 3:
                    _directionStrategy =
                        std::shared_ptr<DirectionStrategy>(new DirectionInRangeBottleneck());
                    break;
                case 6:
                    // dead end -> not supported anymore (global ff needed, but not available in 3d)
                    LOG_ERROR("Exit Strategy 6 is not supported any longer. Please refer to "
                              "www.jupedsim.org");
                    LOG_WARNING(
                        "Changing Exit-Strategy to #9 (Floorfields with targets within subroom)");
                    pExitStrategy      = 9;
                    _exit_strat_number = 9;
                    _directionStrategy =
                        std::shared_ptr<DirectionStrategy>(new DirectionSubLocalFloorfield());
                    if(!ParseFfOpts(strategyNode)) {
                        return false;
                    };
                    break;
                case 7:
                    // dead end -> not supported anymore (global ff needed, but not available in 3d)
                    LOG_ERROR("Exit Strategy 7 is not supported any longer. Please refer to "
                              "www.jupedsim.org");
                    LOG_WARNING(
                        "Changing Exit-Strategy to #9 (Floorfields with targets within subroom)");
                    pExitStrategy      = 9;
                    _exit_strat_number = 9;
                    _directionStrategy =
                        std::shared_ptr<DirectionStrategy>(new DirectionSubLocalFloorfield());
                    if(!ParseFfOpts(strategyNode)) {
                        return false;
                    };
                    break;
                case 8:
                    _directionStrategy =
                        std::shared_ptr<DirectionStrategy>(new DirectionLocalFloorfield());
                    if(!ParseFfOpts(strategyNode)) {
                        return false;
                    };
                    break;
                case 9:
                    _directionStrategy =
                        std::shared_ptr<DirectionStrategy>(new DirectionSubLocalFloorfield());
                    if(!ParseFfOpts(strategyNode)) {
                        return false;
                    };
                    break;
                case 12:
                    _directionStrategy = std::shared_ptr<DirectionStrategy>(new DirectionTrain());
                    break;

                default:
                    _directionStrategy =
                        std::shared_ptr<DirectionStrategy>(new DirectionMinSeperationShorterLine());
                    LOG_ERROR("Unknown exit_crossing_strategy <{}>", pExitStrategy);
                    LOG_WARNING("The default exit_crossing_strategy <2> will be used");
                    break;
            }
        } else {
            return false;
        }
        LOG_INFO("exit_crossing_strategy <{}>", pExitStrategy);
        _config->set_exit_strat(_exit_strat_number);
        _directionManager->SetDirectionStrategy(_directionStrategy);
    }

    // Read waiting
    std::string queryWaiting = "waiting_strategy";
    int waitingStrategyIndex = -1;
    if(strategyNode.FirstChild(queryWaiting) &&
       strategyNode.FirstChild(queryWaiting)->FirstChild()) {
        if(const char * attribute = strategyNode.FirstChild(queryWaiting)->FirstChild()->Value();
           attribute) {
            if(waitingStrategyIndex = xmltoi(attribute, -1);
               waitingStrategyIndex > -1 && attribute == std::to_string(waitingStrategyIndex)) {
                switch(waitingStrategyIndex) {
                    case 1:
                        _waitingStrategy = std::shared_ptr<WaitingStrategy>(new WaitingMiddle());
                        break;
                    case 2:
                        _waitingStrategy = std::shared_ptr<WaitingStrategy>(new WaitingRandom());
                        break;
                    default:
                        _waitingStrategy = std::shared_ptr<WaitingStrategy>(new WaitingRandom());
                        LOG_ERROR("Unknown waiting_strategy <{}>", waitingStrategyIndex);
                        LOG_WARNING("The default waiting_strategy <2> will be used");
                }
            }
        }
    }

    if(waitingStrategyIndex < 0) {
        _waitingStrategy = nullptr;
        LOG_INFO("Could not parse waiting_strategy, no waiting_strategy is used.");
    } else {
        LOG_INFO("Waiting_strategy <{:d}>", waitingStrategyIndex);
    }

    _directionManager->SetWaitingStrategy(_waitingStrategy);
    _config->SetDirectionManager(_directionManager);

    return true;
}

bool IniFileParser::ParseFfOpts(const TiXmlNode & strategyNode)
{
    std::string query = "delta_h";
    if(strategyNode.FirstChild(query.c_str())) {
        const char * tmp = strategyNode.FirstChild(query.c_str())->FirstChild()->Value();
        double pDeltaH   = atof(tmp);
        _config->set_deltaH(pDeltaH);
        LOG_INFO("deltaH: {}", pDeltaH);
    }

    query = "wall_avoid_distance";
    if(strategyNode.FirstChild(query.c_str())) {
        const char * tmp      = strategyNode.FirstChild(query.c_str())->FirstChild()->Value();
        double pWallAvoidance = atof(tmp);
        _config->set_wall_avoid_distance(pWallAvoidance);
        LOG_INFO("Wall avoidance: {}", pWallAvoidance);
    }

    query = "use_wall_avoidance";
    if(strategyNode.FirstChild(query.c_str())) {
        std::string tmp        = strategyNode.FirstChild(query.c_str())->FirstChild()->Value();
        bool pUseWallAvoidance = !(tmp == "false");
        _config->set_use_wall_avoidance(pUseWallAvoidance);
        if(pUseWallAvoidance)
            LOG_INFO("UseWAD: yes");
        else
            LOG_INFO("UseWAD: no");
    }
    return true;
}

bool IniFileParser::ParseExternalFiles(const TiXmlNode & mainNode)
{
    // read external traffic constraints file name
    if(mainNode.FirstChild("traffic_constraints") &&
       mainNode.FirstChild("traffic_constraints")->FirstChild("file")) {
        fs::path trafficFile =
            _config->GetProjectRootDir() /
            mainNode.FirstChild("traffic_constraints")->FirstChild("file")->FirstChild()->Value();
        _config->SetTrafficContraintFile(fs::weakly_canonical(trafficFile));
    }

    // read external goals file name
    if(mainNode.FirstChild("routing") && mainNode.FirstChild("routing")->FirstChild("goals") &&
       mainNode.FirstChild("routing")->FirstChild("goals")->FirstChild("file")) {
        fs::path goalFile = _config->GetProjectRootDir() / mainNode.FirstChild("routing")
                                                               ->FirstChild("goals")
                                                               ->FirstChild("file")
                                                               ->FirstChild()
                                                               ->Value();
        _config->SetGoalFile(fs::weakly_canonical(goalFile));
    }

    // read external sources file name
    if(mainNode.FirstChild("agents") &&
       mainNode.FirstChild("agents")->FirstChild("agents_sources") &&
       mainNode.FirstChild("agents")->FirstChild("agents_sources")->FirstChild("file")) {
        fs::path sourceFile = _config->GetProjectRootDir() / mainNode.FirstChild("agents")
                                                                 ->FirstChild("agents_sources")
                                                                 ->FirstChild("file")
                                                                 ->FirstChild()
                                                                 ->Value();
        _config->SetSourceFile(fs::weakly_canonical(sourceFile));
    }

    // read external event file name
    fs::path eventFile;
    if(mainNode.FirstChild("events_file") && mainNode.FirstChild("events_file")->FirstChild()) {
        eventFile = mainNode.FirstChild("events_file")->FirstChild()->Value();
    } else if(
        mainNode.FirstChild("header") && mainNode.FirstChild("header")->FirstChild("events_file") &&
        mainNode.FirstChild("header")->FirstChild("events_file")->FirstChild()) {
        eventFile = mainNode.FirstChild("header")->FirstChild("events_file")->FirstChild()->Value();
    } else {
        LOG_INFO("No event file given.");
    }

    if(!eventFile.empty()) {
        _config->SetEventFile(fs::weakly_canonical(_config->GetProjectRootDir() / eventFile));
        LOG_INFO("Events are read from: <{}>", _config->GetEventFile().string());
    }

    // read external schedule file name
    fs::path scheduleFile;
    if(mainNode.FirstChild("schedule_file") && mainNode.FirstChild("schedule_file")->FirstChild()) {
        scheduleFile = mainNode.FirstChild("schedule_file")->FirstChild()->Value();
    } else if(
        mainNode.FirstChild("header") &&
        mainNode.FirstChild("header")->FirstChild("schedule_file") &&
        mainNode.FirstChild("header")->FirstChild("schedule_file")->FirstChild()) {
        scheduleFile =
            mainNode.FirstChild("header")->FirstChild("schedule_file")->FirstChild()->Value();
    } else {
        LOG_INFO("No schedule file given.");
    }

    if(!scheduleFile.empty()) {
        _config->SetScheduleFile(fs::weakly_canonical(_config->GetProjectRootDir() / scheduleFile));
        LOG_INFO("Schedule is read from: <{}>", _config->GetScheduleFile().string());
    }

    return true;
}
