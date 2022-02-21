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
#include "IniFileParser.hpp"

#include "NavLineFileParser.hpp"
#include "OperationalModelType.hpp"
#include "OutputHandler.hpp"
#include "direction/waiting/WaitingStrategyType.hpp"
#include "direction/walking/DirectionStrategyType.hpp"
#include "general/Filesystem.hpp"
#include "general/Macros.hpp"
#include "math/GCFMModel.hpp"
#include "math/VelocityModel.hpp"
#include "routing/RoutingStrategy.hpp"
#include "routing/ff_router/ffRouter.hpp"
#include "routing/global_shortest/GlobalRouter.hpp"

#include <Logger.hpp>
#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>
#include <tinyxml.h>

class IniFileParser
{
public:
    IniFileParser(Configuration * config);
    ~IniFileParser(){};

    void Parse(const fs::path & iniFile);

private:
    bool ParseHeader(TiXmlNode * xHeader);

    bool ParseGCFMModel(TiXmlElement * xGCFM, TiXmlElement * xMain);

    bool ParseVelocityModel(TiXmlElement * xVelocity, TiXmlElement * xMain);

    void ParseAgentParameters(TiXmlElement * operativModel, TiXmlNode * agentDistri);

    bool ParseRoutingStrategies(TiXmlNode * routingNode, TiXmlNode * agentDistri);

    bool ParseLinkedCells(const TiXmlNode & linkedCellNode);

    bool ParseStepSize(const TiXmlNode & stepNode);

    bool ParseStrategyNodeToObject(const TiXmlNode & strategyNode);

    bool ParseFfOpts(const TiXmlNode & strategyNode);

    bool ParseExternalFiles(const TiXmlNode & xMain);

    std::optional<GlobalRouterParameters> ParseGlobalRouterParmeters(const TiXmlElement * e);

    Configuration * _config;
    int _model;
};

IniFileParser::IniFileParser(Configuration * config)
{
    _config = config;
}

void IniFileParser::Parse(const fs::path & iniFile)
{
    LOG_INFO("Loading and parsing the project file <{}>", iniFile.string());
    _config->iniFile    = iniFile;
    fs::path parentPath = iniFile.parent_path();
    if(parentPath.empty()) {
        parentPath = fs::current_path();
    }
    _config->projectRootDir = fs::absolute(parentPath);

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

        if((_model == to_underlying(OperationalModelType::GCFM)) &&
           (model_id == to_underlying(OperationalModelType::GCFM))) {
            if(modelName != "gcfm") {
                throw std::logic_error("Mismatch model ID and description. Did you mean gcfm?");
            }
            if(!ParseGCFMModel(xModel, xMainNode)) {
                throw std::logic_error("Error parsing GCFM model parameters.");
            }

            parsingModelSuccessful = true;
            break;
        }
        if((_model == to_underlying(OperationalModelType::VELOCITY)) &&
           (model_id == to_underlying(OperationalModelType::VELOCITY))) {
            if(modelName != "Tordeux2015") {
                throw std::logic_error(
                    "Mismatch model ID and description. Did you mean Tordeux2015?");
            }
            //only parsing one model
            if(!ParseVelocityModel(xModel, xMainNode)) {
                throw std::logic_error("Error parsing Velocity model parameters.");
            }
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
            _config->seed          = (unsigned int) atoi(seedValue); //strtol
        } else {
            _config->seed = static_cast<unsigned int>(time(NULL));
        }
    } else {
        _config->seed = static_cast<unsigned int>(time(NULL));
    }
    LOG_INFO("Random seed <{}>", _config->seed);

    // max simulation time
    if(xHeader->FirstChild("max_sim_time")) {
        const char * tmax = xHeader->FirstChildElement("max_sim_time")->FirstChild()->Value();
        _config->tMax     = atof(tmax);
        LOG_INFO("Maximal simulation time <{:.2f}> seconds", _config->tMax);
    }

    // geometry file name
    if(xHeader->FirstChild("geometry")) {
        std::string filename  = xHeader->FirstChild("geometry")->FirstChild()->Value();
        _config->geometryFile = filename;
        LOG_INFO("Geometry file <{}>", filename);
    }

    //display statistics
    if(xHeader->FirstChild("show_statistics")) {
        std::string value       = xHeader->FirstChild("show_statistics")->FirstChild()->Value();
        _config->showStatistics = value == "true";
        LOG_INFO("Show statistics: {}", value);
    }

    // Results Output Path
    auto * xmlOutput = xHeader->FirstChildElement("output");
    if(xmlOutput != nullptr) {
        auto * xmlOutputPath = xmlOutput->Attribute("path");
        if(xmlOutputPath != nullptr) {
            fs::path path(xmlOutputPath);
            if(path.is_relative()) {
                path = fs::absolute(path);
            }
            _config->outputPath = path;
        }
    }
    LOG_INFO("Output Path configured <{}>", _config->outputPath.string());

    //trajectories
    TiXmlNode * xTrajectories = xHeader->FirstChild("trajectories");
    if(xTrajectories) {
        double fps;
        xHeader->FirstChildElement("trajectories")->Attribute("fps", &fps);
        _config->fps = fps;


        unsigned int precision;
        auto ret = xHeader->FirstChildElement("trajectories")
                       ->QueryUnsignedAttribute("precision", &precision);

        if((ret == TIXML_SUCCESS && (precision < 1 || precision > 6)) ||
           (ret == TIXML_WRONG_TYPE)) {
            LOG_WARNING("Invalid value. Precision should be in [1, 6] (default: 2)", precision);
        } else if(ret != TIXML_NO_ATTRIBUTE) {
            _config->precision = precision;
        }

        //color mode
        std::string color_mode =
            xHeader->FirstChildElement("trajectories")->Attribute("color_mode") ?
                xHeader->FirstChildElement("trajectories")->Attribute("color_mode") :
                "velocity";

        if(color_mode == "velocity") {
            _config->agentColorMode = AgentColorMode::BY_VELOCITY;
        }
        if(color_mode == "group") {
            _config->agentColorMode = AgentColorMode::BY_GROUP;
        }
        if(color_mode == "router") {
            _config->agentColorMode = AgentColorMode::BY_ROUTER;
        }
        if(color_mode == "final_goal") {
            _config->agentColorMode = AgentColorMode::BY_FINAL_GOAL;
        }
        if(color_mode == "intermediate_goal") {
            _config->agentColorMode = AgentColorMode::BY_INTERMEDIATE_GOAL;
        }

        fs::path trajectoryFile = _config->trajectoriesFile;
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

        fs::path canonicalTrajPath = fs::weakly_canonical(_config->outputPath / trajectoryFile);

        _config->trajectoriesFile = canonicalTrajPath;

        LOG_INFO("Output file  <{}>", _config->trajectoriesFile.string());
        LOG_INFO(
            "With <{:.0f}> frames per seconds. Precision: <{:d}>",
            _config->fps,
            _config->precision);


        if(xTrajectories->FirstChild("optional_output")) {
            LOG_WARNING("These optional options do only work with plain output format!");

            auto node = xTrajectories->FirstChildElement("optional_output");
            //check if speed is wanted
            if(const char * attribute = node->Attribute("speed"); attribute) {
                std::string in = xmltoa(attribute, "false");
                std::transform(in.begin(), in.end(), in.begin(), ::tolower);

                if(in == "true") {
                    _config->optionalOutput.insert(OptionalOutput::speed);
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
                    _config->optionalOutput.insert(OptionalOutput::velocity);
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
                    _config->optionalOutput.insert(OptionalOutput::final_goal);
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
                    _config->optionalOutput.insert(OptionalOutput::intermediate_goal);
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
                    _config->optionalOutput.insert(OptionalOutput::desired_direction);
                    LOG_INFO("desired_direction added to output");
                } else {
                    LOG_INFO("desired_direction not added to output");
                }
            }

            //check if router is wanted
            if(const char * attribute = node->Attribute("router"); attribute) {
                std::string in = xmltoa(attribute, "false");
                std::transform(in.begin(), in.end(), in.begin(), ::tolower);

                if(in == "true") {
                    _config->optionalOutput.insert(OptionalOutput::router);
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
                    _config->optionalOutput.insert(OptionalOutput::group);
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

        _config->maxFPed       = std::stod(dist_max);
        _config->nuPed         = std::stod(nu);
        _config->distEffMaxPed = std::stod(disteff_max);
        _config->intPWidthPed  = std::stod(interpolation_width);
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
        _config->maxFWall       = std::stod(dist_max);
        _config->nuWall         = std::stod(nu);
        _config->distEffMaxWall = std::stod(disteff_max);
        _config->intPWidthWall  = std::stod(interpolation_width);
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

    _config->operationalModel = OperationalModelType::GCFM;

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

    //force_ped
    if(xModelPara->FirstChild("force_ped")) {
        if(xModelPara->FirstChildElement("force_ped")->Attribute("a")) {
            std::string a = xModelPara->FirstChildElement("force_ped")->Attribute("a");
            _config->aPed = std::stod(a);
        }

        if(xModelPara->FirstChildElement("force_ped")->Attribute("D")) {
            std::string D = xModelPara->FirstChildElement("force_ped")->Attribute("D");
            _config->dPed = std::stod(D);
        }
        LOG_INFO("Frep_ped a={:.2f}, D={:.2f}", _config->aPed, _config->dPed);
    }
    //force_wall
    if(xModelPara->FirstChild("force_wall")) {
        if(xModelPara->FirstChildElement("force_wall")->Attribute("a")) {
            std::string a  = xModelPara->FirstChildElement("force_wall")->Attribute("a");
            _config->aWall = std::stod(a);
        }

        if(xModelPara->FirstChildElement("force_wall")->Attribute("D")) {
            std::string D  = xModelPara->FirstChildElement("force_wall")->Attribute("D");
            _config->dWall = std::stod(D);
        }
        LOG_INFO("Frep_wall a={:.2f}, D={:.2f}", _config->aWall, _config->dWall);
    }

    //Parsing the agent parameters
    TiXmlNode * xAgentDistri = xMainNode->FirstChild("agents")->FirstChild("agents_distribution");
    ParseAgentParameters(xVelocity, xAgentDistri);
    _config->operationalModel = OperationalModelType::VELOCITY;

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

            auto agentParameters = std::make_shared<AgentsParameters>(para_id, _config->seed);
            _config->agentsParameters[para_id] = agentParameters;
            std::string attrib                 = "v0"; // avoid repetitions and evtl. typos
            //desired speed
            if(xAgentPara->FirstChild(attrib)) {
                double mu    = xmltof(xAgentPara->FirstChildElement(attrib)->Attribute("mu"));
                double sigma = xmltof(xAgentPara->FirstChildElement(attrib)->Attribute("sigma"));
                agentParameters->InitV0(mu, sigma);
                LOG_INFO("Desired speed mu={} , sigma={}", mu, sigma);
            }
            attrib = "v0_upstairs";
            if(xAgentPara->FirstChild(attrib)) {
                double mu    = xmltof(xAgentPara->FirstChildElement(attrib)->Attribute("mu"));
                double sigma = xmltof(xAgentPara->FirstChildElement(attrib)->Attribute("sigma"));
                double c     = xmltof(xAgentPara->FirstChildElement(attrib)->Attribute("c"), 15);
                agentParameters->InitV0UpStairs(mu, sigma, c);
                LOG_INFO("Desired speed upstairs mu={} , sigma={}, c={}", mu, sigma, c);
            }
            attrib = "v0_downstairs";
            if(xAgentPara->FirstChild(attrib)) {
                double mu    = xmltof(xAgentPara->FirstChildElement(attrib)->Attribute("mu"));
                double sigma = xmltof(xAgentPara->FirstChildElement(attrib)->Attribute("sigma"));
                double c     = xmltof(xAgentPara->FirstChildElement(attrib)->Attribute("c"), 15);
                agentParameters->InitV0DownStairs(mu, sigma, c);
                LOG_INFO("Desired speed downstairs mu={} , sigma={}, c={}", mu, sigma, c);
            } //------------------------------------------------------------------------
            attrib = "escalator_upstairs";
            if(xAgentPara->FirstChild(attrib)) {
                double mu    = xmltof(xAgentPara->FirstChildElement(attrib)->Attribute("mu"));
                double sigma = xmltof(xAgentPara->FirstChildElement(attrib)->Attribute("sigma"));
                double c     = xmltof(xAgentPara->FirstChildElement(attrib)->Attribute("c"), 15);
                agentParameters->InitEscalatorUpStairs(mu, sigma, c);
                LOG_INFO("speed of escalator upstairs mu={} , sigma={}, c={}", mu, sigma, c);
            }
            attrib = "escalator_downstairs";
            if(xAgentPara->FirstChild(attrib)) {
                double mu    = xmltof(xAgentPara->FirstChildElement(attrib)->Attribute("mu"));
                double sigma = xmltof(xAgentPara->FirstChildElement(attrib)->Attribute("sigma"));
                double c     = xmltof(xAgentPara->FirstChildElement(attrib)->Attribute("c"), 15);
                agentParameters->InitEscalatorDownStairs(mu, sigma, c);
                LOG_INFO("speed of escalator downstairs mu={} , sigma={}, c={}", mu, sigma, c);
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
                _config->distEffMaxPed =
                    max_Eb + agentParameters->GetT() * agentParameters->GetV0();
                _config->distEffMaxWall = _config->distEffMaxPed;
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
    std::set<int> usedRouter;
    for(TiXmlElement * e = agentsDistri->FirstChildElement("group"); e;
        e                = e->NextSiblingElement("group")) {
        int router = -1;
        if(e->Attribute("router_id")) {
            router = atoi(e->Attribute("router_id"));
            if(std::find(usedRouter.begin(), usedRouter.end(), router) == usedRouter.end()) {
                usedRouter.insert(router);
            }
        }
    }
    for(TiXmlElement * e = routingNode->FirstChildElement("router"); e;
        e                = e->NextSiblingElement("router")) {
        const auto * strategyAsString = e->Attribute("description");
        const auto strategy           = from_string<RoutingStrategy>(strategyAsString);
        const int id                  = atoi(e->Attribute("router_id"));

        if(usedRouter.count(id) == 0) {
            LOG_WARNING(
                "Routing strategy {} with id {} will be skipped because it is unused.",
                strategyAsString,
                id);
            continue;
        }

        // TODO(kkratz) this code is senitive for the order in which sections are parsed.
        // This is a hidden dependency and needs to be addressed
        if(strategy == RoutingStrategy::ROUTING_FF_GLOBAL_SHORTEST &&
           _config->directionStrategyType != DirectionStrategyType::LOCAL_FLOORFIELD) {
            LOG_WARNING("Routing strategy used is ff_gloabl_shortest. Using exit strategy 8 "
                        "recommended!");
        }

        if(strategy == RoutingStrategy::UNKNOWN) {
            LOG_ERROR("Unknown routing strategy: {}", strategyAsString);
            return false;
        }

        const auto params = ParseGlobalRouterParmeters(e);
        if(const auto [_, success] =
               _config->routingStrategies.try_emplace(id, std::make_tuple(strategy, params));
           !success) {
            LOG_ERROR("Duplicated router id found: {}", id);
            return false;
        }
    }
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
            _config->linkedCellSize = std::stod(cell_size);
            LOG_INFO("Linked cells enabled with size  <{:.2f}>", _config->linkedCellSize);
            return true;
        } else {
            _config->linkedCellSize = -1.0;
            LOG_WARNING("Invalid parameters for linkedcells");
            return false;
        }
    }
    return false;
}

bool IniFileParser::ParseStepSize(const TiXmlNode & stepNode)
{
    if(stepNode.FirstChild("stepsize")) {
        const char * stepsize = stepNode.FirstChild("stepsize")->FirstChild()->Value();
        if(stepsize) {
            double tmp         = 1. / _config->fps;
            double stepsizeDBL = atof(stepsize);
            if((stepNode.FirstChildElement("stepsize")->Attribute("fix")) &&
               (std::string(stepNode.FirstChildElement("stepsize")->Attribute("fix")) == "yes")) {
                _config->dT = atof(stepsize);
                LOG_INFO("Stepsize <{}>", _config->dT);
                if(tmp < _config->dT) {
                    LOG_WARNING("Stepsize dt = {} > {} = frameinterval.", _config->dT, tmp);
                    LOG_WARNING("You should decrease stepsize or fps!");
                }
                return true;
            }
            //find a stepsize, that can be multiplied by (int) to get framerate
            for(int i = 1; i < 2000; ++i) {
                if((tmp / i) <= stepsizeDBL) {
                    _config->dT = tmp / i;
                    if((tmp / i) < stepsizeDBL) {
                        LOG_WARNING(
                            "Decreased stepsize from <{}> to <{}> to match fps",
                            stepsizeDBL,
                            (tmp / i));
                    }
                    LOG_INFO("Stepsize <{}>", _config->dT);
                    return true;
                }
            }
            //below should never execute
            _config->dT = stepsizeDBL;
            LOG_INFO("Stepsize <{}>", _config->dT);
            if(tmp < _config->dT) {
                LOG_WARNING("Stepsize dt = {} > {} = frameinterval.", _config->dT, tmp);
                LOG_WARNING("You should decrease stepsize or fps!");
            }
            return true;
        }
    }
    return false;
}

bool IniFileParser::ParseStrategyNodeToObject(const TiXmlNode & strategyNode)
{
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
                       router_descr == "ff_global_shortest") {
                        pExitStrategy = 8;
                        LOG_WARNING("Changing Exit Strategie to work with floorfield!");
                    }
                }
            }

            switch(pExitStrategy) {
                case 1:
                    _config->directionStrategyType = DirectionStrategyType::MIDDLE_POINT;
                    break;
                case 2:
                    _config->directionStrategyType =
                        DirectionStrategyType::MIN_SEPERATION_SHORTER_LINE;
                    break;
                case 3:
                    _config->directionStrategyType = DirectionStrategyType::IN_RANGE_BOTTLENECK;
                    break;
                case 8:
                    _config->directionStrategyType = DirectionStrategyType::LOCAL_FLOORFIELD;
                    if(!ParseFfOpts(strategyNode)) {
                        return false;
                    };
                    break;
                default:
                    LOG_ERROR("Unknown exit_crossing_strategy <{}>", pExitStrategy);
                    LOG_WARNING("The default exit_crossing_strategy <2> will be used");
                    break;
            }
        } else {
            return false;
        }
        LOG_INFO("exit_crossing_strategy <{}>", pExitStrategy);
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
                        _config->waitingStrategyType = WaitingStrategyType::MIDDLE;
                        break;
                    case 2:
                        _config->waitingStrategyType = WaitingStrategyType::RANDOM;
                        break;
                    default:
                        _config->waitingStrategyType = WaitingStrategyType::RANDOM;
                        LOG_ERROR("Unknown waiting_strategy <{}>", waitingStrategyIndex);
                        LOG_WARNING("The default waiting_strategy <2> will be used");
                }
            }
        }
    }
    return true;
}

bool IniFileParser::ParseFfOpts(const TiXmlNode & strategyNode)
{
    std::string query = "delta_h";
    if(strategyNode.FirstChild(query.c_str())) {
        const char * tmp = strategyNode.FirstChild(query.c_str())->FirstChild()->Value();
        double pDeltaH   = atof(tmp);
        _config->deltaH  = pDeltaH;
        LOG_INFO("deltaH: {}", pDeltaH);
    }

    query = "wall_avoid_distance";
    if(strategyNode.FirstChild(query.c_str())) {
        const char * tmp           = strategyNode.FirstChild(query.c_str())->FirstChild()->Value();
        double pWallAvoidance      = atof(tmp);
        _config->wallAvoidDistance = pWallAvoidance;
        LOG_INFO("Wall avoidance: {}", pWallAvoidance);
    }

    query = "use_wall_avoidance";
    if(strategyNode.FirstChild(query.c_str())) {
        std::string tmp           = strategyNode.FirstChild(query.c_str())->FirstChild()->Value();
        bool pUseWallAvoidance    = !(tmp == "false");
        _config->useWallAvoidance = pUseWallAvoidance;
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
            _config->projectRootDir /
            mainNode.FirstChild("traffic_constraints")->FirstChild("file")->FirstChild()->Value();
        _config->trafficContraintFile = fs::weakly_canonical(trafficFile);
    }

    // read external goals file name
    if(mainNode.FirstChild("routing") && mainNode.FirstChild("routing")->FirstChild("goals") &&
       mainNode.FirstChild("routing")->FirstChild("goals")->FirstChild("file")) {
        fs::path goalFile = _config->projectRootDir / mainNode.FirstChild("routing")
                                                          ->FirstChild("goals")
                                                          ->FirstChild("file")
                                                          ->FirstChild()
                                                          ->Value();
        _config->goalFile = fs::weakly_canonical(goalFile);
    }

    // read external sources file name
    if(mainNode.FirstChild("agents") &&
       mainNode.FirstChild("agents")->FirstChild("agents_sources") &&
       mainNode.FirstChild("agents")->FirstChild("agents_sources")->FirstChild("file")) {
        fs::path sourceFile = _config->projectRootDir / mainNode.FirstChild("agents")
                                                            ->FirstChild("agents_sources")
                                                            ->FirstChild("file")
                                                            ->FirstChild()
                                                            ->Value();
        _config->sourceFile = fs::weakly_canonical(sourceFile);
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
        _config->eventFile = fs::weakly_canonical(_config->projectRootDir / eventFile);
        LOG_INFO("Events are read from: <{}>", _config->eventFile.value().string());
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
        _config->scheduleFile = fs::weakly_canonical(_config->projectRootDir / scheduleFile);
        LOG_INFO("Schedule is read from: <{}>", _config->scheduleFile.value().string());
    }

    // read train type
    fs::path trainTypeFile;
    if(mainNode.FirstChild("train_constraints") &&
       mainNode.FirstChild("train_constraints")->FirstChild("train_types")) {
        trainTypeFile = mainNode.FirstChild("train_constraints")
                            ->FirstChild("train_types")
                            ->FirstChild()
                            ->Value();
    } else {
        LOG_INFO("No train type file given.");
    }
    if(!trainTypeFile.empty()) {
        _config->trainTypeFile = fs::weakly_canonical(_config->projectRootDir / trainTypeFile);
        LOG_INFO("Train type is read from: <{}>", _config->trainTypeFile.string());
    }

    // read train time table
    fs::path trainTimeTableFile;
    if(mainNode.FirstChild("train_constraints") &&
       mainNode.FirstChild("train_constraints")->FirstChild("train_time_table")) {
        trainTimeTableFile = mainNode.FirstChild("train_constraints")
                                 ->FirstChild("train_time_table")
                                 ->FirstChild()
                                 ->Value();
    } else {
        LOG_INFO("No train type file given.");
    }
    if(!trainTimeTableFile.empty()) {
        _config->trainTimeTableFile =
            fs::weakly_canonical(_config->projectRootDir / trainTimeTableFile);
        LOG_INFO("Train time table is read from: <{}>", _config->trainTimeTableFile.string());
    }

    return true;
}


std::optional<GlobalRouterParameters>
IniFileParser::ParseGlobalRouterParmeters(const TiXmlElement * e)
{
    const std::string strategy = e->Attribute("description");
    if(strategy != "global_shortest") {
        return std::nullopt;
    }

    GlobalRouterParameters result{};
    if(const auto * parameters = e->FirstChild("parameters")) {
        if(const auto * navigation_lines = parameters->FirstChildElement("navigation_lines")) {
            const std::filesystem::path navLineFile = navigation_lines->Attribute("file");
            result.optionalNavLines                 = parseNavLines(navLineFile);
        }

        if(const auto * navigation_mesh = parameters->FirstChildElement("navigation_mesh")) {
            const std::string local_planing =
                xmltoa(navigation_mesh->Attribute("use_for_local_planning"), "false");
            result.useMeshForLocalNavigation = local_planing == "true";

            std::string method = xmltoa(navigation_mesh->Attribute("method"), "");
            if(method == "triangulation") {
                result.generateNavigationMesh = true;
            } else {
                LOG_WARNING(
                    "only triangulation is supported for the mesh. You supplied "
                    "[{}]",
                    method);
            }
            result.minDistanceBetweenTriangleEdges =
                xmltof(navigation_mesh->Attribute("minimum_distance_between_edges"), -FLT_MAX);
            result.minAngleInTriangles =
                xmltof(navigation_mesh->Attribute("minimum_angle_in_triangles"), -FLT_MAX);
        }
    }
    return result;
}

Configuration ParseIniFile(const std::filesystem::path & path)
{
    Configuration config{};
    IniFileParser p(&config);
    p.Parse(path);
    return config;
}
