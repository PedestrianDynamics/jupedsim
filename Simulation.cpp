/**
 * \file        Simulation.cpp
 * \date        Dec 15, 2010
 * \version     v0.8.1
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
 * The Simulation class represents a simulation of pedestrians
 * based on a certain model in a specific scenario. A simulation is defined by
 * various parameters and functions.
 *
 *
 **/

#include "Simulation.h"
#include "IO/progress_bar.hpp"
#include "routing/ff_router/ffRouter.h"
#include "math/GCFMModel.h"
#include "math/GompertzModel.h"
#include "math/GradientModel.h"
#include "pedestrian/AgentsQueue.h"
#include "pedestrian/AgentsSourcesManager.h"
#include "geometry/WaitingArea.h"
#include <filesystem>
namespace fs = std::filesystem;

#ifdef _OPENMP

#else
#define omp_get_thread_num() 0
#define omp_get_max_threads()  1
#endif

using namespace std;

OutputHandler* Log;
Trajectories* outputTXT;

Simulation::Simulation(Configuration* args)
        :_config(args)
{
     _countTraj = 0;
     _maxFileSize = 10; // MB
    _nPeds = 0;
    _seed = 8091983;
    _deltaT = 0;
    _building = nullptr;
    //_direction = NULL;
    _operationalModel = nullptr;
    _solver = nullptr;
    _iod = new IODispatcher();
    _fps = 1;
    _em = nullptr;
    _gotSources = false;
    _maxSimTime = 100;
//     _config = args;
}

Simulation::~Simulation()
{
    delete _solver;
    delete _iod;
    delete _em;
}

long Simulation::GetPedsNumber() const
{
    return _nPeds;
}

bool Simulation::InitArgs()
{
    char tmp[CLENGTH];
    string s = "Parameter:\n";

    switch (_config->GetLog()) {
    case 0: {

        break;
    }
    case 1: {
        if (Log)
            delete Log;
        Log = new STDIOHandler();
        break;
    }
    case 2: {
        char name[CLENGTH] = "";
        sprintf(name, "%s.txt", _config->GetErrorLogFile().c_str());
        if (Log)
            delete Log;
        Log = new FileHandler(name);
    }
        break;
    default:
        printf("Wrong option for Logfile!\n\n");
        return false;
    }

    if (_config->GetPort()!=-1) {
        switch (_config->GetFileFormat()) {
        case FORMAT_XML_PLAIN_WITH_MESH:
        case FORMAT_XML_PLAIN: {
            OutputHandler* travisto = new SocketHandler(_config->GetHostname(),
                    _config->GetPort());
            Trajectories* output = new TrajectoriesJPSV05();
            output->SetOutputHandler(travisto);
            _iod->AddIO(output);
            break;
        }
        case FORMAT_XML_BIN: {
            Log->Write(
                    "INFO: \tFormat xml-bin not yet supported in streaming\n");
            //exit(0);
            break;
        }
        case FORMAT_PLAIN: {
            Log->Write(
                    "INFO: \tFormat plain not yet supported in streaming\n");
            return false;
        }
        case FORMAT_VTK: {
            Log->Write(
                    "INFO: \tFormat vtk not yet supported in streaming\n");
            return false;
        }
        default: {
            return false;
        }
        }

        s.append("\tonline streaming enabled \n");
    }

    if (!_config->GetTrajectoriesFile().empty()) {
        switch (_config->GetFileFormat()) {
        case FORMAT_XML_PLAIN: {
            OutputHandler* tofile = new FileHandler(
                    _config->GetTrajectoriesFile().c_str());
            Trajectories* output = new TrajectoriesJPSV05();
            output->SetOutputHandler(tofile);
            _iod->AddIO(output);
            break;
        }
        case FORMAT_PLAIN: {
            OutputHandler* file = new FileHandler(
                    _config->GetTrajectoriesFile().c_str());
            outputTXT = new TrajectoriesFLAT();
            outputTXT->SetOutputHandler(file);
            _iod->AddIO(outputTXT);
            break;
        }
        case FORMAT_VTK: {
            Log->Write("INFO: \tFormat vtk not yet supported\n");
            OutputHandler* file = new FileHandler(
                    (_config->GetTrajectoriesFile()+".vtk").c_str());
            Trajectories* output = new TrajectoriesVTK();
            output->SetOutputHandler(file);
            _iod->AddIO(output);
            break;
        }

        case FORMAT_XML_PLAIN_WITH_MESH: {
            //OutputHandler* tofile = new FileHandler(args->GetTrajectoriesFile().c_str());
            //if(_iod) delete _iod;
            //_iod = new TrajectoriesXML_MESH();
            //_iod->AddIO(tofile);
            break;
        }
        case FORMAT_XML_BIN: {
            // OutputHandler* travisto = new SocketHandler(args->GetHostname(), args->GetPort());
            // Trajectories* output= new TrajectoriesJPSV06();
            // output->SetOutputHandler(travisto);
            // _iod->AddIO(output);
            break;
        }
        default: {
            break;
        }
        }
    }

    _operationalModel = _config->GetModel();
    s.append(_operationalModel->GetDescription());

    // ODE solver which is never used!
    auto solver = _config->GetSolver();
    sprintf(tmp, "\tODE Solver: %d\n", solver);
    s.append(tmp);

    sprintf(tmp, "\tnCPU: %d\n", _config->GetMaxOpenMPThreads());
    s.append(tmp);
    sprintf(tmp, "\tt_max: %f\n", _config->GetTmax());
    s.append(tmp);
    _deltaT = _config->Getdt();
    _maxSimTime = _config->GetTmax();
    sprintf(tmp, "\tdt: %f\n", _deltaT);
    _periodic = _config->IsPeriodic();
    sprintf(tmp, "\t periodic: %d\n", _periodic);
    s.append(tmp);

    _fps = _config->GetFps();
    sprintf(tmp, "\tfps: %f\n", _fps);
    s.append(tmp);
    //Log->Write(s.c_str());

    _routingEngine = _config->GetRoutingEngine();
    auto distributor = std::unique_ptr<PedDistributor>(new PedDistributor(_config));
    // IMPORTANT: do not change the order in the following..
    _building = std::shared_ptr<Building>(new Building(_config, *distributor));

    // Initialize the agents sources that have been collected in the pedestrians distributor
    _agentSrcManager.SetBuilding(_building.get());
    _agentSrcManager.SetMaxSimTime(GetMaxSimTime());
    _gotSources = (bool) distributor->GetAgentsSources().size(); // did we have any sources? false if no sources
    std::cout << "\nGot " << _gotSources  << " sources"<< std::endl ;

    for (const auto& src: distributor->GetAgentsSources()) {
        _agentSrcManager.AddSource(src);
        src->Dump();
    }




    //perform customs initialisation, like computing the phi for the gcfm
    //this should be called after the routing engine has been initialised
    // because a direction is needed for this initialisation.
    Log->Write("INFO:\t Init Operational Model starting ...");
    if (!_operationalModel->Init(_building.get()))
        return false;
    Log->Write("INFO:\t Init Operational Model done");

    // Give the DirectionStrategy the chance to perform some initialization.
    // This should be done after the initialization of the operationalModel
    // because then, invalid pedestrians have been deleted and FindExit()
    // has been called.

    //other initializations
    for (auto&& ped: _building->GetAllPedestrians()) {
        ped->Setdt(_deltaT);
    }
    _nPeds = _building->GetAllPedestrians().size();
    //_building->WriteToErrorLog();
    Log->Write("INFO:\t nPeds %d received", _nPeds);
    //get the seed
    _seed = _config->GetSeed();

    //size of the cells/GCFM/Gompertz
    if (_config->GetDistEffMaxPed()>_config->GetLinkedCellSize()) {
        Log->Write(
                "ERROR: the linked-cell size [%f] should be bigger than the force range [%f]",
                _config->GetLinkedCellSize(), _config->GetDistEffMaxPed());
        return false;
    }

    //read and initialize events
    _em = new EventManager(_building.get(), _config->GetSeed());
    if (!_em->ReadEventsXml()) {
        Log->Write("ERROR: \tCould not initialize events handling");
    }
    _em->ListEvents();

    //_building->SaveGeometry("test.sav.xml");

    //if(_building->SanityCheck()==false)
    //     return false;

    //everything went fine
    return true;
}

double Simulation::RunStandardSimulation(double maxSimTime)
{
    RunHeader(_nPeds+_agentSrcManager.GetMaxAgentNumber());
    double t = RunBody(maxSimTime);
    RunFooter();
    return t;
}

void Simulation::UpdateRoutesAndLocations()
{
     //pedestrians to be deleted
     //you should better create this in the constructor and allocate it once.
     set<Pedestrian*> pedsToRemove;
//     pedsToRemove.reserve(500); //just reserve some space

     // collect all pedestrians in the simulation.
     const vector<Pedestrian*>& allPeds = _building->GetAllPedestrians();
     const map<int, Goal*>& goals = _building->GetAllGoals();
     auto allRooms = _building->GetAllRooms();

//    for (signed int p = 0; p < allPeds.size(); ++p) {
//        Pedestrian* ped = allPeds[p];
//
//        std::cout << "FinalDestination of [" << ped->GetID() << "] in (" << ped->GetRoomID() << ", " << ped->GetSubRoomID() << "/" <<  ped->GetSubRoomUID() << "): " << ped->GetFinalDestination() << std::endl;
//    }

#pragma omp parallel for shared(pedsToRemove, allRooms)
     for (signed int p = 0; p < allPeds.size(); ++p) {
          auto ped = allPeds[p];
          Room* room = _building->GetRoom(ped->GetRoomID());
          SubRoom* sub0 = room->GetSubRoom(ped->GetSubRoomID());

          //set the new room if needed
          if ((ped->GetFinalDestination() == FINAL_DEST_OUT)
                    && (room->GetCaption() == "outside")) { //TODO Hier aendern fuer inside goals?
#pragma omp critical(Simulation_Update_pedsToRemove)
               pedsToRemove.insert(ped);
          } else if ((ped->GetFinalDestination() != FINAL_DEST_OUT)
                    && (goals.at(ped->GetFinalDestination())->Contains(
                              ped->GetPos()))&& (goals.at(ped->GetFinalDestination())->GetIsFinalGoal())) {
#pragma omp critical(Simulation_Update_pedsToRemove)
               pedsToRemove.insert(ped);
          }

          // reposition in the case the pedestrians "accidently left the room" not via the intended exit.
          // That may happen if the forces are too high for instance
          // the ped is removed from the simulation, if it could not be reassigned
          else if (!sub0->IsInSubRoom(ped))
          {

               bool assigned = false;
               std::function<void(const Pedestrian&)> f = std::bind(&Simulation::UpdateFlowAtDoors, this, std::placeholders::_1);
               assigned = ped->Relocate(f);
               //this will delete agents, that are pushed outside (maybe even if inside obstacles??)

               if (!assigned) {
#pragma omp critical(Simulation_Update_pedsToRemove)
                       pedsToRemove.insert(ped); //the agent left the old room
                    //actualize the eggress time for that room
#pragma omp critical(SetEgressTime)
                    allRooms.at(ped->GetRoomID())->SetEgressTime(ped->GetGlobalTime());

               }
          }
          // actualize routes for sources
          if(_gotSources)
               ped->FindRoute();
          //finally actualize the route
          if ( !_gotSources && ped->FindRoute() == -1 ) {
               //a destination could not be found for that pedestrian
               Log->Write("ERROR: \tCould not find a route for pedestrian %d in room %d and subroom %d",
                         ped->GetID(), ped->GetRoomID(), ped->GetSubRoomID());
               //ped->FindRoute(); //debug only, plz remove
               std::function<void(const Pedestrian&)> f = std::bind(&Simulation::UpdateFlowAtDoors, this, std::placeholders::_1);
               ped->Relocate(f);
               //exit(EXIT_FAILURE);
#pragma omp critical(Simulation_Update_pedsToRemove)
               {
                    pedsToRemove.insert(ped);
                    Log->incrementDeletedAgents();
               }
          }
     }


#ifdef _USE_PROTOCOL_BUFFER
     if (_hybridSimManager)
     {
          AgentsQueueOut::Add(pedsToRemove);    //this should be critical region (and it is)
     }
     else
#endif
    {

        // remove the pedestrians that have left the building
        for (auto p : pedsToRemove){
            UpdateFlowAtDoors(*p);
            _building->DeletePedestrian(p);
        }
        pedsToRemove.clear();
    }

    //    temporary fix for the safest path router
    //    if (dynamic_cast<SafestPathRouter*>(_routingEngine->GetRouter(1)))
    //    {
    //         SafestPathRouter* spr = dynamic_cast<SafestPathRouter*>(_routingEngine->GetRouter(1));
    //         spr->ComputeAndUpdateDestinations(_allPedestians);
    //    }
}

void Simulation::PrintStatistics(double simTime)
{
    Log->Write("\nRooms Egress. Simulation Time: %.2f", simTime);
    Log->Write("==================");
    Log->Write("id\tcaption\tegress time (s)");

    for (const auto& it:_building->GetAllRooms()) {
        auto&& room = it.second;
        if (room->GetCaption()!="outside")
            Log->Write("%d\t%s\t%.2f", room->GetID(), room->GetCaption().c_str(), room->GetEgressTime());
    }

    Log->Write("\nUsage of Exits");
    Log->Write("==========");
    for (const auto& itr : _building->GetAllTransitions()) {
        Transition* goal = itr.second;
        if (goal->GetDoorUsage()) {
            Log->Write(
                    "\nExit ID [%d] used by [%d] pedestrians. Last passing time [%0.2f] s",
                    goal->GetID(), goal->GetDoorUsage(),
                    goal->GetLastPassingTime());

            string statsfile = _config->GetTrajectoriesFile()+"_flow_exit_id_"+to_string(goal->GetID())+".dat";
            if(goal->GetOutflowRate() <  (std::numeric_limits<double>::max)())
            {
                 char tmp[50];
                 sprintf(tmp, "%.2f", goal->GetOutflowRate());
                 statsfile = _config->GetTrajectoriesFile()+"_flow_exit_id_"+to_string(goal->GetID())+"_rate_"+tmp+".dat";
            }
            Log->Write("More Information in the file: %s", statsfile.c_str());
            auto output = new FileHandler(statsfile.c_str());
            output->Write("#Simulation time: %.2f", simTime);
            output->Write("#Flow at exit "+goal->GetCaption()+"( ID "+to_string(goal->GetID())+" )");
            output->Write("#Time (s)  cummulative number of agents \n");
            output->Write(goal->GetFlowCurve());
        }
    }

        Log->Write("\nUsage of Crossings");
        Log->Write("==========");
        for (const auto& itr : _building->GetAllCrossings()) {
                Crossing* goal = itr.second;
                if (goal->GetDoorUsage()) {
                        Log->Write(
                                "\nCrossing ID [%d] in Room ID [%d] used by [%d] pedestrians. Last passing time [%0.2f] s",
                                goal->GetID(), itr.first/1000, goal->GetDoorUsage(),
                                goal->GetLastPassingTime());

                        string statsfile = _config->GetTrajectoriesFile() + "_flow_crossing_id_"
                                + to_string(itr.first/1000) + "_" + to_string(itr.first % 1000) +".dat";
                        Log->Write("More Information in the file: %s", statsfile.c_str());
                        auto output = new FileHandler(statsfile.c_str());
                        output->Write("#Simulation time: %.2f", simTime);
                        output->Write("#Flow at crossing " + goal->GetCaption() + "( ID " + to_string(goal->GetID())
                                + " ) in Room ( ID "+ to_string(itr.first / 1000) + " )");
                        output->Write("#Time (s)  cummulative number of agents \n");
                        output->Write(goal->GetFlowCurve());
                }
        }

    Log->Write("\n");
}

void Simulation::RunHeader(long nPed)
{
    // writing the header
    if (nPed==-1) nPed = _nPeds;
    _iod->WriteHeader(nPed, _fps, _building.get(), _seed, 0);// first trajectory
                                                             // count = 0
    _iod->WriteGeometry(_building.get());
    if( _gotSources)
         _iod->WriteSources( GetAgentSrcManager().GetSources());

    int writeInterval = (int) ((1./_fps)/_deltaT+0.5);
    writeInterval = (writeInterval<=0) ? 1 : writeInterval; // mustn't be <= 0
    int firstframe = (Pedestrian::GetGlobalTime()/_deltaT)/writeInterval;

    _iod->WriteFrame(firstframe, _building.get());
    //first initialisation needed by the linked-cells
    UpdateRoutesAndLocations();
    ProcessAgentsQueue();
}

double Simulation::RunBody(double maxSimTime)
{
    //needed to control the execution time PART 1
    //in the case you want to run in no faster than realtime
    //time_t starttime, endtime;
    //time(&starttime);

    //take the current time from the pedestrian
    double t = Pedestrian::GetGlobalTime();
    fs::path TrajectoryName(_config->GetTrajectoriesFile());// in case we
                                                                // may need to
                                                                // generate
                                                                // several small files
    //frame number. This function can be called many times,
    static int frameNr = (int) (1+t/_deltaT); // Frame Number

    //##########
    //PROBLEMATIC: time when frame should be printed out
    // possibly skipped when using the following lines
    // NEEDS TO BE FIXED!
    int writeInterval = (int) ((1./_fps)/_deltaT+0.5);
    writeInterval = (writeInterval<=0) ? 1 : writeInterval; // mustn't be <= 0
    // ##########

    //process the queue for incoming pedestrians
    //important since the number of peds is used
    //to break the main simulation loop
    ProcessAgentsQueue();
    _nPeds = _building->GetAllPedestrians().size();
    std::cout << "\n";
    std::string description = "Evacutation ";
    ProgressBar *bar = new ProgressBar(_nPeds, description);
    // bar->SetFrequencyUpdate(10);
#ifdef _WINDOWS
    bar->SetStyle("|","-");
#else
    bar->SetStyle("\u2588", "-"); //for linux
#endif
    int initialnPeds = _nPeds;

    // main program loop
    while ((_nPeds || (!_agentSrcManager.IsCompleted()&& _gotSources) ) && t<maxSimTime) {
        t = 0+(frameNr-1)*_deltaT;

        //process the queue for incoming pedestrians
        ProcessAgentsQueue();

        if (t>Pedestrian::GetMinPremovementTime()) {
            //update the linked cells
            _building->UpdateGrid();

            // update the positions
            _operationalModel->ComputeNextTimeStep(t, _deltaT, _building.get(), _periodic);

            //update the events
            _em->ProcessEvent();

            //here we could place router-tasks (calc new maps) that can use multiple cores AND we have 't'
            //update quickestRouter
            if (_routingEngine.get()->GetRouter(ROUTING_FF_QUICKEST)) {
                FFRouter* ffrouter = dynamic_cast<FFRouter*>(_routingEngine.get()->GetRouter(ROUTING_FF_QUICKEST));
                if (ffrouter->MustReInit()) {
                    ffrouter->ReInit();
                    ffrouter->SetRecalc(t);
                }
            }

            if (_routingEngine->isNeedUpdate()){
                 _routingEngine->UpdateRouter();
            }

            //update the routes and locations
            UpdateRoutesAndLocations();

            //other updates
            //someone might have left the building
            _nPeds = _building->GetAllPedestrians().size();
        }

        // update the global time
        Pedestrian::SetGlobalTime(t);

        // write the trajectories
        if (0==frameNr%writeInterval) {
            _iod->WriteFrame(frameNr/writeInterval, _building.get());
            fs::path p = _config->GetTrajectoriesFile();
            int sf = fs::file_size(p);
            if(sf>_maxFileSize*1024*1024)
            {
                 std::string extention = p.extension().string();
                 _countTraj++;
                 char tmp_traj_name[100];
                 sprintf(tmp_traj_name,"%s_%.4d_%s", TrajectoryName.stem().string().c_str(), _countTraj, extention.c_str());
                 _config->SetTrajectoriesFile(tmp_traj_name);
                 Log->Write("INFO:\tNew trajectory file <%s>", tmp_traj_name);
                 OutputHandler* file = new FileHandler(_config->GetTrajectoriesFile().c_str());
                 outputTXT->SetOutputHandler(file);

//_config->GetProjectRootDir()+"_1_"+_config->GetTrajectoriesFile());
                 // _config->SetTrajectoriesFile(name);
                 _iod->WriteHeader(_nPeds, _fps, _building.get(), _seed, _countTraj);
                 // _iod->WriteGeometry(_building.get());
            }
        }

        if(!_gotSources && !_periodic && _config->print_prog_bar())
              // Log->ProgressBar(initialnPeds, initialnPeds-_nPeds, t);
              bar->Progressed(initialnPeds-_nPeds);
        else
             if ((!_gotSources) &&
                 ((frameNr < 100 &&  frameNr % 10 == 0) ||
                  (frameNr > 100 &&  frameNr % 100 == 0)))
                  printf("time: %6.2f (%4.0f)  | Agents: %6ld / %d [%4.1f%%]\n",  t , maxSimTime, _nPeds, initialnPeds, (double)(initialnPeds-_nPeds)/initialnPeds*100);


        // needed to control the execution time PART 2
        // time(&endtime);
        // double timeToWait=t-difftime(endtime, starttime);
        // clock_t goal = timeToWait*1000 + clock();
        // while (goal > clock());
        ++frameNr;

        //Trigger JPSfire Toxicity Analysis
        //only executed every 3 seconds
        #ifdef JPSFIRE
        if( fmod(Pedestrian::GetGlobalTime(), 3) == 0 ) {
            for (auto&& ped: _building->GetAllPedestrians()) {
                ped->ConductToxicityAnalysis();
            }
        }
        #endif

        // here open transition that should be closed
        //        TODO fix, opens door everytime...
        for (auto& itr: _building->GetAllTransitions())
        {
             Transition* Trans = itr.second;
             if(Trans->IsTempClose())
             {
                  if ((Trans->GetMaxDoorUsage() != std::numeric_limits<int>::max()) ||
                    (Trans->GetOutflowRate() != std::numeric_limits<double>::max()) ){
//                        || (Trans->GetOutflowRate() != std::numeric_limits<double>::max)){
                      Trans->UpdateClosingTime( _deltaT);
                      if(Trans->GetClosingTime() <= _deltaT){
                          Trans->changeTemporaryState();
                      }
                      Log-> Write("INFO:\tReset state of door %d,  Time=%.2f", Trans->GetID(), Pedestrian::GetGlobalTime());
                  }
             }
        }
        if(frameNr % 1000 == 0)
        {
             Log->Write("INFO:\tUpdate door statistics at t=%.2f", t);
             PrintStatistics(t);
        }


    }// while time
    return t;
}

void Simulation::RunFooter()
{
    // writing the footer
    _iod->WriteFooter();
}

void Simulation::ProcessAgentsQueue()
{

     /* std::cout << "Call Simulation::ProcessAgentsQueue() at: " << Pedestrian::GetGlobalTime() << std::endl; */
     /* std::cout << KRED << " SIMULATION building " << _building << " size "  << _building->GetAllPedestrians().size() << "\n" << RESET; */
           /* for(auto pp: _building->GetAllPedestrians()) */
     /*           std::cout<< KBLU << "BUL: Simulation: " << pp->GetPos()._x << ", " << pp->GetPos()._y << RESET << std::endl; */

    //incoming pedestrians
    vector<Pedestrian*> peds;
    //  std::cout << ">>> peds " << peds.size() << RESET<< std::endl;

    AgentsQueueIn::GetandClear(peds);
    //std::cout << "SIMULATION BEFORE BOOL = " <<  _agentSrcManager.IsBuildingUpdated() << " peds size " << peds.size() << "\n" ;

    //_agentSrcManager.SetBuildingUpdated(true);
    /* std::cout << "SIMULATION AFTER BOOL = " <<  _agentSrcManager.IsBuildingUpdated() << "\n" ; */

    for (auto&& ped: peds) {
            /* std::cout << "Add to building : " << ped->GetPos()._x << ", " << ped->GetPos()._y << " t: "<< Pedestrian::GetGlobalTime() << std::endl; */
        _building->AddPedestrian(ped);
    }
    _building->UpdateGrid();
    //  for(auto pp: _building->GetAllPedestrians())
    //         std::cout<< KBLU << "BUL: Simulation: " << pp->GetPos()._x << ", " << pp->GetPos()._y  << " t: "<< Pedestrian::GetGlobalTime() <<RESET << std::endl;


    /* std::cout << "LEAVE Simulation::ProcessAgentsQueue() with " << " size "  << _building->GetAllPedestrians().size() << "\n" << RESET; */
}

void Simulation::UpdateDoorticks() const {
//    int softstateDecay = 1;
//    //Softstate of _lastTickTime is valid for X seconds as in (X/_deltaT); here it is 2s
//    auto& allCross = _building->GetAllCrossings();
//    for (auto& crossPair : allCross) {
//        crossPair.second->_refresh1 += 1;
//        crossPair.second->_refresh2 += 1;
//        if (crossPair.second->_refresh1 > (softstateDecay/_deltaT)) {
//            crossPair.second->_lastTickTime1 = 0;
//            crossPair.second->_refresh1 = 0;
//        }
//        if (crossPair.second->_refresh2 > (softstateDecay/_deltaT)) {
//            crossPair.second->_lastTickTime2 = 0;
//            crossPair.second->_refresh2 = 0;
//        }
//    }
//
//    auto& allTrans = _building->GetAllTransitions();
//    for (auto& transPair : allTrans) {
//        transPair.second->_refresh1 += 1;
//        transPair.second->_refresh2 += 1;
//        if (transPair.second->_refresh1 > (softstateDecay/_deltaT)) {
//            transPair.second->_lastTickTime1 = 0;
//            transPair.second->_refresh1 = 0;
//        }
//        if (transPair.second->_refresh2 > (softstateDecay/_deltaT)) {
//            transPair.second->_lastTickTime2 = 0;
//            transPair.second->_refresh2 = 0;
//        }
//    }
};

void Simulation::UpdateFlowAtDoors(const Pedestrian& ped) const
{
    if (_config->ShowStatistics()) {
        Transition* trans = _building->GetTransitionByUID(ped.GetExitIndex());
        if (trans) {
            //check if the pedestrian left the door correctly
            if (trans->DistTo(ped.GetPos())>0.5) {
                Log->Write("WARNING:\t pedestrian [%d] left room/subroom [%d/%d] in an unusual way. Please check",
                        ped.GetID(), ped.GetRoomID(), ped.GetSubRoomID());
                Log->Write("       :\t distance to last door (%d | %d) is %f. That should be smaller.",
                        trans->GetUniqueID(), ped.GetExitIndex(),
                        trans->DistTo(ped.GetPos()));
                Log->Write("       :\t correcting the door statistics");
                //ped.Dump(ped.GetID());

                //checking the history and picking the nearest previous destination
                double biggest = 0.3;
                bool success = false;
                for (const auto& dest:ped.GetLastDestinations()) {
                    if (dest!=-1) {
                        Transition* trans_tmp = _building->GetTransitionByUID(dest);
                        if (trans_tmp && trans_tmp->DistTo(ped.GetPos())<biggest) {
                            biggest = trans_tmp->DistTo(ped.GetPos());
                            trans = trans_tmp;
                            Log->Write("       :\t Best match found at door %d", dest);
                            success = true;//at least one door was found
                        }
                    }
                }

                if (!success) {
                    Log->Write("WARNING       :\t correcting the door statistics");
                    return; //todo we need to check if the ped is in a subroom neighboring the target. If so, no problems!
                }
            }
//#pragma omp critical

            trans->IncreaseDoorUsage(1, ped.GetGlobalTime());
            trans->IncreasePartialDoorUsage(1);
            // when <dn> agents pass <trans>, we start evaluating the flow
            // .. and maybe close the <trans>
            if( trans->GetPartialDoorUsage() ==  trans->GetDN() ) {
                 trans->regulateFlow(Pedestrian::GetGlobalTime());
                 trans->ResetPartialDoorUsage();
            }
        }

        Crossing* cross = _building->GetCrossingByUID(ped.GetExitIndex());
        if (cross) {
             cross->IncreaseDoorUsage(1, ped.GetGlobalTime());
        }
    }
}

AgentsSourcesManager& Simulation::GetAgentSrcManager()
{
    return _agentSrcManager;
}

Building* Simulation::GetBuilding()
{
    return _building.get();
}

int Simulation::GetMaxSimTime() const{
      return _maxSimTime;
}
