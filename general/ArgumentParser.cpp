/**
 * \file        ArgumentParser.cpp
 * \date        Apr 20, 2009
 * \version     v0.6
 * \copyright   <2009-2014> Forschungszentrum Jülich GmbH. All rights reserved.
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
// #ifndef _MSC_VER
// #include <getopt.h>
// #endif
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <sstream>
#include <ctime>
#include <limits.h>
#include <stdlib.h>

#ifdef _OPENMP
#include <omp.h>

#else
#define omp_get_thread_num() 0
#define omp_get_max_threads()  1
#endif

#include "../tinyxml/tinyxml.h"
#include "../IO/OutputHandler.h"
#include "ArgumentParser.h"
#include "../pedestrian/AgentsParameters.h"
#include "../routing/GlobalRouter.h"
#include "../routing/QuickestPathRouter.h"
#include "../routing/GraphRouter.h"
#include "../routing/MeshRouter.h"
#include "../routing/DummyRouter.h"
#include "../routing/SafestPathRouter.h"
#include "../routing/CognitiveMapRouter.h"
#include "../math/GompertzModel.h"
#include "../math/GCFMModel.h"

using namespace std;

//TODO: print the new Usage with ini.xml and building.xml file because the actual implementation is obsolete
void ArgumentParser::Usage()
{

    fprintf(stderr,
            "Usage: program options\n\n"
            "with the following options (default values in parenthesis):\n\n"
            "  [-q/--inifile <filepath>]        initialisation file( ini.xml found in the actual directory)\n"
            "  [-n/--number <filepath>]         file with the pedestrian distribution in room(./Inputfiles/persons.xml)\n"
            "  [-t/--tmax <double>]             maximal simulation time (500)\n"
            "  [-d/--dt <double>]               time step (0.001)\n"
            "  [--fps <double>]                 framerate (1.0 fps)\n"
            "  [-s/--solver <int>]              type of solver, if needed (euler)\n"
            "                                       1: euler\n"
            "                                       2: verlet\n"
            "                                       3: leapfrog\n"
            "  [-g/--geometry <string>]         path to the geometry file (./Inputfiles/geo.xml)\n"
            "  [-e/--exitstrategy <int>]        strategy how the direction to the exit is computed (3).\n"
            "                                       1: Middlepoint of the exit line\n"
            "                                       2: Shortest distance point of the exit line\n"
            "                                       3: Shortest distance point of the exit line, but exit is 20 cm  shorter\n"
            "  [-l/--linkedcells [<double>]]    use of linked-cells with optional cell size (default cellsize = 2.2m)\n"
            "  [-R/--routing <int> [<string>]]             routing strategy (1):\n"
            "                                       1: local shortest path\n"
            "                                       2: global shortest path\n"
            "                                       3: quickest path\n"
            "  [-v/--v0mu <double>]             mu for normal distribution of v0, desired velocity (1.24)\n"
            "  [-V/--v0sigma <double>]          sigma for normal distribution of v0, desired velocity (0.26)\n"
            "  [-a/--ataumu <double>]           mu for normal distribution of a_tau, factor for velocity 1st axis (0.53)\n"
            "  [-A/--atausigma <double>]        sigma for normal distribution of a_tau, factor for velocity 1st axis (0.001)\n"
            "  [-z/--aminmu <double>]           mu for normal distribution of a_min, minimal size 1st axis (0.18)\n"
            "  [-Z/--aminsigma <double>]        sigma for normal distribution of a_min, minimal size 1st axis (0.001)\n"
            "  [-b/--bmaxmu <double>]           mu for normal distribution of b_max, maximal size 2nd axis (0.25)\n"
            "  [-B/--bmaxsigma <double>]        sigma for normal distribution of b_max, maximal size 2nd axis (0.001)\n"
            "  [-y/--bminmu <double>]           mu for normal distribution of b_min, minimal size 2nd axis (0.2)\n"
            "  [-Y/--bminsigma <double>]        sigma for normal distribution of b_min, minimal size 2nd axis (0.001)\n"
            "  [-c/--taumu <double>]            mu for normal distribution of tau, reaction time (0.5)\n"
            "  [-C/--tausigma <double>]         sigma for normal distribution of tau, reaction time (0.001)\n"
            "  [-x/--nuped <double>]            strength of forces between pedestrians, factor (0.3)\n"
            "  [-X/--nuwall <double>]           strength of wall forces, factor (0.2)\n"
            "  [-i/--intpwidthped <double>]     interpolation width of pedestrian forces (0.1)\n"
            "  [-I/--intpwidthwall <double>]    interpolation width of wall forces (0.1)\n"
            "  [-m/--maxfped <double>]          interpolation parameter of pedestrian forces (3)\n"
            "  [-M/--maxfwall <double>]         interpolation parameter of wall forces (3)\n"
            "  [-f/--disteffmaxped  <double>]   maximal effective distance to pedestrians (2)\n"
            "  [-F/--disteffmaxwall <double>]   maximal effective distance to walls (2)\n"
            "  [--pathway <filepath>]           file for saving pedestrians path\n"
            "  [-L/--log <int>]                 log output (0):\n"
            "                                       0: no output\n"
            "                                       1: log output to stdout\n"
            "                                       2: log output to file ./outputfiles/log.txt\n"
            "  [-T/--output-file <string>]      file to write the trajectories ( no trajectories are written if omited0):\n"
            "  [-P/--streaming-port <int>]      stream the ouput to the specified address/port\n"
            "  [-O/--streaming-ip <string>]     stream the ouput to the specified address/port\n"
            "  [-N/--generate-mesh <string>]    write the navigation mesh to the given file\n"
            "  [-h/--help]                      this manual output\n"
            "\n");
    exit(EXIT_SUCCESS);
}

ArgumentParser::ArgumentParser()
{
    // Default parameter values
    //pNumberFilename = "inputfiles/persons.xml";
    pSolver = 1;
    _projectFile = "";
    pTmax = 500;
    pfps = 1.0;
    pdt = 0.01;
    pExitStrategy = 2;
    pLinkedCells = false;
    pLinkedCellSize = 2.2;
    pV0Mu = 1.24;
    pV0Sigma = 0.26;
    pBmaxMu = 0.25;
    pBmaxSigma = 0.001;
    pBminMu = 0.2;
    pBminSigma = 0.001;
    pAtauMu = 0.53;
    pAtauSigma = 0.001;
    pAminMu = 0.18;
    pAminSigma = 0.001;
    pNuPed = 0.4;
    pNuWall = 0.2;
    pIntPWidthPed = 0.1;
    pIntPWidthWall = 0.1;
    pMaxFPed = 3;
    pMaxFWall = 3;
    pDistEffMaxPed = 2; //0.8
    pDistEffMaxWall = 2;
    pTauMu = 0.5;
    pTauSigma = 0.001;
    paPed=1;
    pbPed=0.25;
    pcPed=3;
    paWall=1;
    pbWall=0.7;
    pcWall=3;
    pLog = 0;
    pModel=MODEL_GFCM;
    pErrorLogFile = "./Logfile.dat";
    pNavMeshFilename = "";
    pSeed = 0;
    pFormat = FORMAT_XML_PLAIN;
    pPort = -1;
    pHostname = "localhost";
    _embedMesh = 0;
    pMaxOpenMPThreads = omp_get_thread_num();
    _profilingFlag = false;
    _hpcFlag = 0;
    _agentsParameters= std::map<int, AgentsParameters*>();
    p_routingengine = std::shared_ptr<RoutingEngine>(new RoutingEngine());
}

void ArgumentParser::ParseArgs(int argc, char **argv)
{
    //special case of the default configuration ini.xml
    if (argc == 1)
    {
        Log->Write(
                "INFO: \tTrying to load the default configuration from the file <ini.xml>");
        ParseIniFile("ini.xml");
        return;
    }

    string argument = argv[1];
    if (argument == "-h" && argc == 2) {
        Usage();
        return;
    }



    // other special case where a single configuration file is submitted
    //check if inifile options are given
    if (argc == 2)
    {
        string prefix1 = "--ini=";
        string prefix2 = "--inifile=";

        if (!argument.compare(0, prefix2.size(), prefix2)) {
            argument.erase(0, prefix2.size());
        } else if (!argument.compare(0, prefix1.size(), prefix1)) {
            argument.erase(0, prefix1.size());
        }
        ParseIniFile(argument);

    }
}

const std::map<int, AgentsParameters*>& ArgumentParser::GetAgentsParameters() const
{
    return _agentsParameters;
}

void ArgumentParser::SetAgentsParameters(const std::map<int, AgentsParameters*>& agentsParameters)
{
    _agentsParameters = agentsParameters;
}

void ArgumentParser::ParseIniFile(string inifile)
{

    Log->Write("INFO: \tLoading and parsing the project file <%s>",
            inifile.c_str());
    _projectFile = inifile;

    //extract and set the project root dir
    size_t found = inifile.find_last_of("/\\");
    if (found != string::npos)
        _projectRootDir = inifile.substr(0, found) + "/";

    // cout<<inifile<<endl;
    // cout<<_projectRootDir<<endl; exit(0);
    // char *real_path = _realpath(path, NULL);
    // use real_path
    // free(real_path);

    TiXmlDocument doc(inifile);
    if (!doc.LoadFile())
    {
        Log->Write("ERROR: \t%s", doc.ErrorDesc());
        Log->Write("ERROR: \t could not parse the project file");
        exit(EXIT_FAILURE);
    }

    // everything is fine. proceed with parsing

    TiXmlElement* xMainNode = doc.RootElement();
    if (!xMainNode)
    {
        Log->Write("ERROR:\tRoot element does not exist");
        exit(EXIT_FAILURE);
    }

    if (xMainNode->ValueStr() != "JuPedSim")
    {
        Log->Write("ERROR:\tRoot element value is not 'JuPedSim'.");
        exit(EXIT_FAILURE);
    }

    //check the header version
    if (!xMainNode->Attribute("version"))
    {
        Log->Write("WARNING:\t There is no header version. I am assuming %s",
                JPS_VERSION);
    }
    else if (string(xMainNode->Attribute("version")) != JPS_VERSION)
    {
        Log->Write(
                "ERROR:\t Wrong header version. Only version %s is supported.",
                JPS_VERSION);
        exit(EXIT_FAILURE);
    }

    //seed
    if (xMainNode->FirstChild("seed"))
    {
        TiXmlNode* seedNode = xMainNode->FirstChild("seed")->FirstChild();
        if (seedNode)
        {
            const char* seedValue = seedNode->Value();
            pSeed = atoi(seedValue);
            srand(pSeed);
            Log->Write("INFO: \tseed <%d>", pSeed);
        }
        else
        {
            pSeed = time(NULL);
            srand(pSeed);
            Log->Write("INFO: \trandom seed <%d>", pSeed);
        }
    }

    // max simulation time
    if (xMainNode->FirstChild("max_sim_time"))
    {
        const char* tmax =
                xMainNode->FirstChildElement("max_sim_time")->FirstChild()->Value();
        //const char* unit=xMainNode->FirstChildElement("max_sim_time")->Attribute("unit");
        pTmax = atof(tmax);
        Log->Write("INFO: \tMaxmimal simulation time <%.2f> seconds",pTmax);
    }
    int max_cpus = 1; 
#ifdef _OPENMP
    max_cpus = omp_get_max_threads();
#endif
    //max CPU
    if(xMainNode->FirstChild("numCPU")) {
        TiXmlNode* seedNode = xMainNode->FirstChild("numCPU")->FirstChild();
        int n = 1;
        if(seedNode){
            const char* cpuValue = seedNode->Value();
            n = atoi(cpuValue);
            if (n > max_cpus) n = max_cpus;
        }
        else {
            n = max_cpus;
        }
        pMaxOpenMPThreads = n;
        Log->Write("INFO: \tnumCPU <%d>", pMaxOpenMPThreads);
#ifdef _OPENMP
        if(n < omp_get_max_threads() )
            omp_set_num_threads(pMaxOpenMPThreads);
#endif
    }
    else { // no numCPU tag
        pMaxOpenMPThreads = max_cpus;
#ifdef _OPENMP
        omp_set_num_threads(pMaxOpenMPThreads);
#endif
        Log->Write("INFO: \t Default numCPU <%d>", pMaxOpenMPThreads);
    }
    //logfile
    if (xMainNode->FirstChild("logfile"))
    {
        pErrorLogFile = _projectRootDir
                + xMainNode->FirstChild("logfile")->FirstChild()->Value();
        pLog = 2;
        Log->Write("INFO: \tlogfile <" + (pErrorLogFile) + ">");
    }

    //trajectories
    TiXmlNode* xTrajectories = xMainNode->FirstChild("trajectories");
    if (xTrajectories)
    {

        xMainNode->FirstChildElement("trajectories")->Attribute("fps", &pfps);

        string format =
                xMainNode->FirstChildElement("trajectories")->Attribute(
                        "format") ?
                                xMainNode->FirstChildElement("trajectories")->Attribute(
                                        "format") :
                                        "xml-plain";
        if (xMainNode->FirstChildElement("trajectories")->Attribute(
                "embed_mesh"))
            _embedMesh =
                    string(
                            xMainNode->FirstChildElement("trajectories")->Attribute(
                                    "embed_mesh")) == "true" ? 1 : 0;

        if (format == "xml-plain")
            pFormat = FORMAT_XML_PLAIN;
        if (format == "xml-plain" && _embedMesh == 1)
            pFormat = FORMAT_XML_PLAIN_WITH_MESH;
        if (format == "xml-bin")
            pFormat = FORMAT_XML_BIN;
        if (format == "plain")
            pFormat = FORMAT_PLAIN;
        if (format == "vtk")
            pFormat = FORMAT_VTK;

        //a file descriptor was given
        if (xTrajectories->FirstChild("file"))
        {
            const char* tmp =
                    xTrajectories->FirstChildElement("file")->Attribute(
                            "location");
            if (tmp)
                pTrajectoriesFile = _projectRootDir + tmp;
            Log->Write(
                    "INFO: \toutput file  <" + string(pTrajectoriesFile) + ">");
            Log->Write("INFO: \tin format <%s> at <%f> frames per seconds",
                    format.c_str(), pfps);
        }

        if (xTrajectories->FirstChild("socket"))
        {
            const char* tmp =
                    xTrajectories->FirstChildElement("socket")->Attribute("hostname");
            if (tmp)
                pHostname = tmp;
            xTrajectories->FirstChildElement("socket")->Attribute("port", &pPort);
            Log->Write("INFO: \tStreaming results to output [%s:%d] ",
                    pHostname.c_str(), pPort);
        }
    }

    //pick up which model to use
    //get the wanted ped model id
    pModel=xmltoi(xMainNode->FirstChildElement("agents")->Attribute("operational_model_id"),-1);
    if( (pModel==-1) /*|| ( (pModel!=MODEL_GFCM) && pModel!=MODEL_GOMPERTZ) */)
    {
        Log->Write("ERROR: \tmissing operational_model_id attribute in the agent section. ");
        Log->Write("ERROR: \tplease specify the model id to use");
        exit(EXIT_FAILURE);
    }


    bool parsingModelSuccessful=false;

    for (TiXmlElement* xModel = xMainNode->FirstChild("operational_models")->FirstChildElement(
            "model"); xModel;
            xModel = xModel->NextSiblingElement("model"))
    {

        if(!xModel->Attribute("description") )
        {
            Log->Write("ERROR: \t missing attribute description in models ?");
            exit(EXIT_FAILURE);
        }

        string modelName = string(xModel->Attribute("description"));
        int model_id  = xmltoi(xModel->Attribute("operational_model_id"),-1);

        if ((pModel == MODEL_GFCM) && (model_id==MODEL_GFCM))
        {
            if (modelName != "gcfm")
            {
                Log->Write("ERROR: \t mismatch model ID and description. Did you mean gcfm ?");
                exit(EXIT_FAILURE);
            }
            ParseGCFMModel(xModel);
            parsingModelSuccessful=true;
            //only parsing one model
            break;
        }
        else if ((pModel == MODEL_GOMPERTZ) && (model_id==MODEL_GOMPERTZ))
        {
            if (modelName != "gompertz")
            {
                Log->Write("ERROR: \t mismatch model ID and description. Did you mean gompertz ?");
                exit(EXIT_FAILURE);
            }
            //only parsing one model
            ParseGompertzModel(xModel);
            parsingModelSuccessful=true;
            break;
        }
    }

    if( parsingModelSuccessful==false)
    {
        Log->Write("ERROR: \tWrong model id [%d]. Choose 1 (GCFM) or 2 (Gompertz)", pModel);
        Log->Write("ERROR: \tPlease make sure that all models are specified in the operational_models section");
        Log->Write("ERROR: \tand make sure to use the same ID in th agent section");
        exit(EXIT_FAILURE);
    }

    //route choice strategy
    TiXmlNode* xRouters = xMainNode->FirstChild("route_choice_models");
    parseRoutingStrategies(xRouters);

    Log->Write("INFO: \tParsing the project file completed");
}

void ArgumentParser::ParseGCFMModel(TiXmlElement* xGCFM)
{

    Log->Write("\nINFO:\tUsing the GCFM model");
    Log->Write("INFO:\tParsing the model parameters");

    TiXmlNode* xModelPara = xGCFM->FirstChild("model_parameters");
    if(!xModelPara){
        Log->Write("ERROR: \t !!!! Changes in the operational model section !!!");
        Log->Write("ERROR: \t !!!! The new version is in inputfiles/ship_msw/ini_ship2.xml !!!");
        exit(EXIT_FAILURE);
    }

    // For convenience. This moved to the header as it is not model specific
    if (xModelPara->FirstChild("tmax"))
    {
        Log->Write(
                "ERROR: \tthe maximal simulation time section moved to the header!!!");
        Log->Write("ERROR: \t\t <max_sim_time> </max_sim_time>\n");
        exit(EXIT_FAILURE);
    }

    //solver
    parseNodeToSolver(*xModelPara);

    //stepsize
    parseStepSize(*xModelPara);

    //exit crossing strategy
    parseStrategyNodeToObject(*xModelPara);

    //linked-cells
    parseLinkedCells(*xModelPara);

    //force_ped
    if (xModelPara->FirstChild("force_ped"))
    {
        string nu = xModelPara->FirstChildElement("force_ped")->Attribute("nu");
        string dist_max = xModelPara->FirstChildElement("force_ped")->Attribute(
                "dist_max");
        string disteff_max =
                xModelPara->FirstChildElement("force_ped")->Attribute(
                     "disteff_max"); // @todo: rename disteff_max to force_max
        string interpolation_width =
                xModelPara->FirstChildElement("force_ped")->Attribute(
                        "interpolation_width");

        pMaxFPed = atof(dist_max.c_str());
        pNuPed = atof(nu.c_str());
        pDistEffMaxPed = atof(disteff_max.c_str());
        pIntPWidthPed = atof(interpolation_width.c_str());
        Log->Write(
                "INFO: \tfrep_ped mu=" + nu + ", dist_max=" + dist_max
                + ", disteff_max=" + disteff_max
                + ", interpolation_width=" + interpolation_width);
    }

    //force_wall
    if (xModelPara->FirstChild("force_wall"))
    {
        string nu = xModelPara->FirstChildElement("force_wall")->Attribute("nu");
        string dist_max = xModelPara->FirstChildElement("force_wall")->Attribute(
                "dist_max");
        string disteff_max =
                xModelPara->FirstChildElement("force_wall")->Attribute(
                        "disteff_max");
        string interpolation_width =
                xModelPara->FirstChildElement("force_wall")->Attribute(
                        "interpolation_width");
        pMaxFWall = atof(dist_max.c_str());
        pNuWall = atof(nu.c_str());
        pDistEffMaxWall = atof(disteff_max.c_str());
        pIntPWidthWall = atof(interpolation_width.c_str());
        Log->Write(
                "INFO: \tfrep_wall mu=" + nu + ", dist_max=" + dist_max
                + ", disteff_max=" + disteff_max
                + ", interpolation_width=" + interpolation_width);
    }

    //Parsing the agent parameters
    ParseAgentParameters(xGCFM);
    p_op_model = std::shared_ptr<OperationalModel>(new GCFMModel(p_exit_strategy.get(), this->GetNuPed(),
            this->GetNuWall(), this->GetDistEffMaxPed(),
            this->GetDistEffMaxWall(), this->GetIntPWidthPed(),
            this->GetIntPWidthWall(), this->GetMaxFPed(),
            this->GetMaxFWall()));
}


void ArgumentParser::ParseGompertzModel(TiXmlElement* xGompertz)
{
    //parsing the model parameters
    Log->Write("\nINFO:\tUsing the Gompertz model");

    Log->Write("INFO:\tParsing the model parameters");

    TiXmlNode* xModelPara = xGompertz->FirstChild("model_parameters");
    if(!xModelPara){
        Log->Write("ERROR: \t !!!! Changes in the operational model section !!!");
        Log->Write("ERROR: \t !!!! The new version is in inputfiles/ship_msw/ini_ship3.xml !!!");
        exit(EXIT_FAILURE);
    }

    // For convenience. This moved to the header as it is not model specific
    if (xModelPara->FirstChild("tmax"))
    {
        Log->Write("ERROR: \tthe maximal simulation time section moved to the header!!!");
        Log->Write("ERROR: \t\t <max_sim_time> </max_sim_time>\n");
        exit(EXIT_FAILURE);
    }

    //solver
    parseNodeToSolver(*xModelPara);

    //stepsize
    parseStepSize(*xModelPara);

    //exit crossing strategy
    parseStrategyNodeToObject(*xModelPara);


    //linked-cells
    parseLinkedCells(*xModelPara);

    //force_ped
    if (xModelPara->FirstChild("force_ped"))
    {
        string nu = xModelPara->FirstChildElement("force_ped")->Attribute("nu");
        pNuPed = atof(nu.c_str());

        if (!xModelPara->FirstChildElement("force_ped")->Attribute("a"))
            paPed = 1.0; // default value
        else
        {
            string a = xModelPara->FirstChildElement("force_ped")->Attribute("a");
            paPed = atof(a.c_str());
        }
        if (!xModelPara->FirstChildElement("force_ped")->Attribute("b"))
            pbPed = 0.25; // default value
        else
        {
            string b = xModelPara->FirstChildElement("force_ped")->Attribute("b");
            pbPed = atof(b.c_str());
        }
        if (!xModelPara->FirstChildElement("force_ped")->Attribute("c"))
            pcPed = 3.0; // default value
        else
        {
            string c = xModelPara->FirstChildElement("force_ped")->Attribute("c");
            pcPed = atof(c.c_str());
        }
        Log->Write("INFO: \tfrep_ped mu=%s, a=%0.2f, b=%0.2f c=%0.2f",nu.c_str(),paPed,pbPed,pcPed);
    }
    //force_wall
    if (xModelPara->FirstChild("force_wall"))
    {
        string nu = xModelPara->FirstChildElement("force_wall")->Attribute("nu");
        pNuWall = atof(nu.c_str());
        if (!xModelPara->FirstChildElement("force_wall")->Attribute("a"))
            paWall = 1.0; // default value
        else
        {
            string a = xModelPara->FirstChildElement("force_wall")->Attribute("a");
            paWall = atof(a.c_str());
        }
        if (!xModelPara->FirstChildElement("force_wall")->Attribute("b"))
            pbWall = 0.7; // default value
        else
        {
            string b = xModelPara->FirstChildElement("force_wall")->Attribute("b");
            pbWall = atof(b.c_str());
        }
        if (!xModelPara->FirstChildElement("force_wall")->Attribute("c"))
            pcWall = 3.0; // default value
        else
        {
            string c = xModelPara->FirstChildElement("force_wall")->Attribute("c");
            pcWall = atof(c.c_str());
        }

        Log->Write("INFO: \tfrep_wall mu=%s, a=%0.2f, b=%0.2f c=%0.2f",nu.c_str(),paWall,pbWall,pcWall);
    }

    //Parsing the agent parameters
    ParseAgentParameters(xGompertz);
    p_op_model = std::shared_ptr<OperationalModel>(new GompertzModel(p_exit_strategy.get(), this->GetNuPed(),
            this->GetaPed(), this->GetbPed(), this->GetcPed(),
            this->GetNuWall(), this->GetaWall(), this->GetbWall(),
            this->GetcWall()));
}

void ArgumentParser::ParseAgentParameters(TiXmlElement* operativModel)
{
    //Parsing the agent parameters
    Log->Write("\nINFO:\tParsing agents  parameters");
    for(TiXmlElement* xAgentPara = operativModel->FirstChildElement("agent_parameters"); xAgentPara;
            xAgentPara = xAgentPara->NextSiblingElement("agent_parameters")) {

        //get the group ID
        int para_id= xmltoi(xAgentPara->Attribute("agent_parameter_id"),-1);
        Log->Write("INFO: \tParsing the group parameter id [%d]",para_id);
        AgentsParameters* agentParameters = new AgentsParameters(para_id,pSeed);
        _agentsParameters[para_id]=agentParameters;

        //desired speed
        if (xAgentPara->FirstChild("v0"))
        {
            double mu = xmltof(xAgentPara->FirstChildElement("v0")->Attribute("mu"),pV0Mu);
            double sigma = xmltof(xAgentPara->FirstChildElement("v0")->Attribute("sigma"),pV0Sigma);
            agentParameters->InitV0(mu,sigma);
            Log->Write("INFO: \tdesired velocity mu=%f , sigma=%f",mu,sigma);
        }

        //bmax
        if (xAgentPara->FirstChild("bmax"))
        {
            double mu = xmltof(xAgentPara->FirstChildElement("bmax")->Attribute("mu"),pBmaxMu);
            double sigma = xmltof(xAgentPara->FirstChildElement("bmax")->Attribute("sigma"),pBmaxSigma);
            agentParameters->InitBmax(mu,sigma);
            Log->Write("INFO: \ttBmax mu=%f , sigma=%f",mu,sigma);
        }

        //bmin
        if (xAgentPara->FirstChild("bmin"))
        {
            double mu = xmltof(xAgentPara->FirstChildElement("bmin")->Attribute("mu"),pBminMu);
            double sigma = xmltof(xAgentPara->FirstChildElement("bmin")->Attribute("sigma"),pBminSigma);
            agentParameters->InitBmin(mu,sigma);
            Log->Write("INFO: \ttBmin mu=%f , sigma=%f",mu,sigma);
        }

        //amin
        if (xAgentPara->FirstChild("amin"))
        {
            double mu = xmltof(xAgentPara->FirstChildElement("amin")->Attribute("mu"),pAminMu);
            double sigma = xmltof(xAgentPara->FirstChildElement("amin")->Attribute("sigma"),pAminSigma);
            agentParameters->InitAmin(mu,sigma);
            Log->Write("INFO: \ttAmin mu=%f , sigma=%f",mu,sigma);
        }
        //tau
        if (xAgentPara->FirstChild("tau"))
        {
            double mu = xmltof(xAgentPara->FirstChildElement("tau")->Attribute("mu"),pTauMu);
            double sigma = xmltof(xAgentPara->FirstChildElement("tau")->Attribute("sigma"),pTauSigma);
            agentParameters->InitTau(mu,sigma);
            Log->Write("INFO: \ttTau mu=%f , sigma=%f",mu,sigma);
        }
        //atau
        if (xAgentPara->FirstChild("atau"))
        {
            double mu = xmltof(xAgentPara->FirstChildElement("atau")->Attribute("mu"),pAtauMu);
            double sigma = xmltof(xAgentPara->FirstChildElement("atau")->Attribute("sigma"),pAtauSigma);
            agentParameters->InitAtau(mu,sigma);
            Log->Write("INFO: \ttAtau mu=%f , sigma=%f",mu,sigma);
        }
        if(pModel == 2) { //  Gompertz
             double beta_c = 2; /// @todo quick and dirty
             double max_Ea = agentParameters->GetAmin() + agentParameters->GetAtau()*agentParameters->GetV0();
             double max_Eb = 0.5*(agentParameters->GetBmin() + 0.49) ; /// @todo hard-coded value should be the same as in pedestrians GetEB
             double max_Ea_Eb = (max_Ea>max_Eb)?max_Ea:max_Eb;
             pDistEffMaxPed = 2 * beta_c * max_Ea_Eb;
             pDistEffMaxWall  = pDistEffMaxPed;
        }
    }
}

void ArgumentParser::parseRoutingStrategies(TiXmlNode *routingNode)
{
    if (!routingNode)
    {
        Log->Write("ERROR:\tNo routers found.");
        exit(EXIT_FAILURE);
    }
    for (TiXmlElement* e = routingNode->FirstChildElement("router"); e;
         e = e->NextSiblingElement("router")) {

        string strategy = e->Attribute("description");
        int id = atoi(e->Attribute("router_id"));

        if (strategy == "local_shortest") {
            pRoutingStrategies.push_back(make_pair(id, ROUTING_LOCAL_SHORTEST));
            Router *r = new GlobalRouter(id, ROUTING_LOCAL_SHORTEST);
            p_routingengine->AddRouter(r);
        }
        else if (strategy == "global_shortest") {
            pRoutingStrategies.push_back(make_pair(id, ROUTING_GLOBAL_SHORTEST));
            Router *r = new GlobalRouter(id, ROUTING_GLOBAL_SHORTEST);
            p_routingengine->AddRouter(r);
        }
        else if (strategy == "quickest") {
            pRoutingStrategies.push_back(make_pair(id, ROUTING_QUICKEST));
            Router *r = new QuickestPathRouter(id, ROUTING_QUICKEST);
            p_routingengine->AddRouter(r);
        }
        else if (strategy == "dynamic") {
            pRoutingStrategies.push_back(make_pair(id, ROUTING_DYNAMIC));
            Router *r = new GraphRouter(id, ROUTING_DYNAMIC);
            p_routingengine->AddRouter(r);
        }
        else if (strategy == "nav_mesh") {
            pRoutingStrategies.push_back(make_pair(id, ROUTING_NAV_MESH));
            Router *r = new MeshRouter(id, ROUTING_NAV_MESH);
            p_routingengine->AddRouter(r);
        }
        else if (strategy == "dummy") {
            pRoutingStrategies.push_back(make_pair(id, ROUTING_DUMMY));
            Router *r = new DummyRouter(id, ROUTING_DUMMY);
            p_routingengine->AddRouter(r);
        }
        else if (strategy == "global_safest") {
            pRoutingStrategies.push_back(make_pair(id, ROUTING_SAFEST));
            Router *r = new SafestPathRouter(id, ROUTING_SAFEST);
            p_routingengine->AddRouter(r);
        }
        else if (strategy == "cognitive_map") {
            pRoutingStrategies.push_back(make_pair(id, ROUTING_COGNITIVEMAP));
            Router *r = new CognitiveMapRouter(id, ROUTING_COGNITIVEMAP);
            p_routingengine->AddRouter(r);
        }
        else {
            Log->Write("ERROR: \twrong value for routing strategy [%s]!!!\n",
                    strategy.c_str());
            exit(EXIT_FAILURE);
        }
    }
}

void ArgumentParser::parseStrategyNodeToObject(const TiXmlNode &strategyNode)
{
    if (strategyNode.FirstChild("exitCrossingStrategy")) {
        const char *tmp =
                strategyNode.FirstChild("exitCrossingStrategy")->FirstChild()->Value();
        if (tmp) {
            pExitStrategy = atoi(tmp);
            switch (pExitStrategy) {
                case 1:
                    p_exit_strategy = std::shared_ptr<DirectionStrategy>(new DirectionMiddlePoint());
                    break;
                case 2:
                    p_exit_strategy = std::shared_ptr<DirectionStrategy>(new DirectionMinSeperationShorterLine());
                    break;
                case 3:
                    p_exit_strategy = std::shared_ptr<DirectionStrategy>(new DirectionInRangeBottleneck());
                    break;
                case 4:
                    p_exit_strategy = std::shared_ptr<DirectionStrategy>(new DirectionGeneral());
                    break;
                default:
                    p_exit_strategy = std::shared_ptr<DirectionStrategy>(new DirectionMinSeperationShorterLine());
                    break;
            }
        }
        Log->Write("INFO: \texitCrossingStrategy < %d >", pExitStrategy);
    }
}

void ArgumentParser::parseNodeToSolver(const TiXmlNode &solverNode)
{
    if (solverNode.FirstChild("solver"))
    {
        string solver = solverNode.FirstChild("solver")->FirstChild()->Value();
        if (solver == "euler")
        {
            pSolver = 1;
        }
        else if (solver == "verlet")
            pSolver = 2;
        else if (solver == "leapfrog")
            pSolver = 3;
        else
        {
            Log->Write("ERROR: \twrong value for solver type!!!\n");
            exit(EXIT_FAILURE);
        }
        Log->Write("INFO: \tpSolver <" + string(solver) + ">");
    }
}

shared_ptr<OperationalModel> ArgumentParser::GetModel() const {
    return p_op_model;
}
const FileFormat& ArgumentParser::GetFileFormat() const
{
    return pFormat;
}
const string& ArgumentParser::GetHostname() const
{
    return pHostname;
}
void ArgumentParser::SetHostname(const string& hostname)
{
    pHostname = hostname;
}
int ArgumentParser::GetPort() const
{
    return pPort;
}
void ArgumentParser::SetPort(int port)
{
    pPort = port;
}
int ArgumentParser::GetSolver() const
{
    return pSolver;
}

double ArgumentParser::GetTmax() const
{
    return pTmax;
}

double ArgumentParser::Getdt() const
{
    return pdt;
}

double ArgumentParser::Getfps() const
{
    return pfps;
}

const string& ArgumentParser::GetProjectFile() const
{
    return _projectFile;
}

/// @deprecated
const string& ArgumentParser::GetNavigationMesh() const
{
    return pNavMeshFilename;
}

std::shared_ptr<DirectionStrategy> ArgumentParser::GetExitStrategy() const
{
    return p_exit_strategy;
}

bool ArgumentParser::GetLinkedCells() const
{
    return pLinkedCells;
}

std::shared_ptr<RoutingEngine> ArgumentParser::GetRoutingEngine() const
{
    return p_routingengine;
}

vector<pair<int, RoutingStrategy> > ArgumentParser::GetRoutingStrategy() const
{
    return pRoutingStrategies;
}

double ArgumentParser::GetV0Mu() const
{
    return pV0Mu;
}

double ArgumentParser::GetV0Sigma() const
{
    return pV0Sigma;
}

double ArgumentParser::GetBmaxMu() const
{
    return pBmaxMu;
}

double ArgumentParser::GetBmaxSigma() const
{
    return pBmaxSigma;
}

double ArgumentParser::GetBminMu() const
{
    return pBminMu;
}

double ArgumentParser::GetBminSigma() const
{
    return pBminSigma;
}

double ArgumentParser::GetAtauMu() const
{
    return pAtauMu;
}

double ArgumentParser::GetAtauSigma() const
{
    return pAtauSigma;
}

double ArgumentParser::GetAminMu() const
{
    return pAminMu;
}

double ArgumentParser::GetAminSigma() const
{
    return pAminSigma;
}

double ArgumentParser::GetNuPed() const
{
    return pNuPed;
}

double ArgumentParser::GetaPed() const
{
    return paPed;
}

double ArgumentParser::GetbPed() const
{
    return pbPed;
}

double ArgumentParser::GetcPed() const
{
    return pcPed;
}

double ArgumentParser::GetNuWall() const
{
    return pNuWall;
}

double ArgumentParser::GetaWall() const
{
    return paWall;
}

double ArgumentParser::GetbWall() const
{
    return pbWall;
}

double ArgumentParser::GetcWall() const
{
    return pcWall;
}

double ArgumentParser::GetIntPWidthPed() const
{
    return pIntPWidthPed;
}

double ArgumentParser::GetIntPWidthWall() const
{
    return pIntPWidthWall;
}

double ArgumentParser::GetMaxFPed() const
{
    return pMaxFPed;
}

double ArgumentParser::GetMaxFWall() const
{
    return pMaxFWall;
}

double ArgumentParser::GetDistEffMaxPed() const
{
    return pDistEffMaxPed;
}

double ArgumentParser::GetDistEffMaxWall() const
{
    return pDistEffMaxWall;
}

double ArgumentParser::GetTauMu() const
{
    return pTauMu;
}

double ArgumentParser::GetTauSigma() const
{
    return pTauSigma;
}

int ArgumentParser::GetLog() const
{
    return pLog;
}

double ArgumentParser::GetLinkedCellSize() const
{
    return pLinkedCellSize;
}

unsigned int ArgumentParser::GetSeed() const
{
    return pSeed;
}

int ArgumentParser::GetEmbededMesh() const
{
    return _embedMesh;
}

const string& ArgumentParser::GetErrorLogFile() const
{
    return pErrorLogFile;
}

int ArgumentParser::GetMaxOpenMPThreads() const
{
    return pMaxOpenMPThreads;
}
const string& ArgumentParser::GetTrajectoriesFile() const
{
    return pTrajectoriesFile;
}

void ArgumentParser::SetTrajectoriesFile(const string& trajectoriesFile)
{
    pTrajectoriesFile = trajectoriesFile;
}

const string& ArgumentParser::GetProjectRootDir() const
{
    return _projectRootDir;
}

bool ArgumentParser::GetProfileFlag()
{
    return _profilingFlag;
}

int ArgumentParser::GetHPCFlag() const
{
    return _hpcFlag;
}

void ArgumentParser::parseLinkedCells(const TiXmlNode &linkedCellNode) {
    if (linkedCellNode.FirstChild("linkedcells"))
    {
        string linkedcells = linkedCellNode.FirstChildElement("linkedcells")->Attribute(
                "enabled");
        string cell_size = linkedCellNode.FirstChildElement("linkedcells")->Attribute(
                "cell_size");

        if (linkedcells == "true")
        {
            pLinkedCells = true;
            pLinkedCellSize = atof(cell_size.c_str());
            Log->Write(
                    "INFO: \tlinked cells enabled with size  <" + cell_size + ">");
        }
        else
        {
            Log->Write("WARNING: \tinvalid parameters for linkedcells");
        }
    }
}

void ArgumentParser::parseStepSize(TiXmlNode &stepNode) {
    if (stepNode.FirstChild("stepsize"))
    {
        const char* stepsize = stepNode.FirstChild("stepsize")->FirstChild()->Value();
        if (stepsize)
            pdt = atof(stepsize);
        Log->Write("INFO: \tstepsize <%f>", pdt);
    }
}
