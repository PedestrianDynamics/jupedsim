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
// Created by laemmel on 23.03.16.
//

#ifndef JPSCORE_CONFIGURATION_H
#define JPSCORE_CONFIGURATION_H

#include <string>
#include <cstdlib>
#include <memory>
#include "Macros.h"

#ifdef _JPS_AS_A_SERVICE

#include "../hybrid/HybridSimulationManager.h"

#endif

#include "../routing/RoutingEngine.h"
#include "../math/OperationalModel.h"

//This class provides a data container for all configuration parameters.

class AgentsParameters;

#ifdef _JPS_AS_A_SERVICE

class HybridSimulationManager;

#endif

class Configuration {

public:

     Configuration()
     {
          _log = 0;
          _port = -1.0;
          _tMax = 20;
          _routingEngine = std::shared_ptr<RoutingEngine>(new RoutingEngine());
     }

     int GetSolver() const { return _solver; };

     void SetSolver(int solver) { _solver = solver; };

     std::shared_ptr<RoutingEngine> GetRoutingEngine() const { return _routingEngine; };

     //TODO: this is certainly not a config parameter but part of the model, we really should separate data and model [gl march '16]
     void SetRoutingEngine(std::shared_ptr<RoutingEngine> routingEngine) { _routingEngine = routingEngine; };

     int GetMaxOpenMPThreads() const { return _maxOpenMPThreads; };

     void SetMaxOpenMPThreads(int maxOpenMPThreads) { _maxOpenMPThreads = maxOpenMPThreads; };

     int GetLog() const { return _log; };

     void SetLog(int log) { _log = log; };

     int GetPort() const { return _port; };

     void SetPort(int port) { _port = port; };

     unsigned int GetSeed() const { return _seed; };

     void SetSeed(unsigned int seed) { _seed = seed; };

     double GetFps() const { return _fps; };

     void SetFps(double fps) { _fps = fps; };

     double GetLinkedCellSize() const { return _linkedCellSize; };

     void SetLinkedCellSize(double linkedCellSize) { _linkedCellSize = linkedCellSize; };

     std::shared_ptr<OperationalModel> GetModel() const { return _model; };

     void SetModel(std::shared_ptr<OperationalModel> model) { _model = model; };

     double GetTmax() const { return _tMax; };

     void SetTmax(double tMax) { _tMax = tMax; };

     double Getdt() const { return _dT; };

     void Setdt(double dT) { _dT = dT; };

     int IsPeriodic() const { return _isPeriodic; };

     void SetIsPeriodic(int isPeriodic) { _isPeriodic = isPeriodic; };

     double GetNuPed() const { return _nuPed; };

     void SetNuPed(double nuPed) { _nuPed = nuPed; };

     double GetNuWall() const { return _nuWall; };

     void SetNuWall(double nuWall) { _nuWall = nuWall; };

     double GetaPed() const { return _aPed; };

     void SetaPed(double aPed) { _aPed = aPed; };

     double GetbPed() const { return _bPed; };

     void SetbPed(double bPed) { _bPed = bPed; };

     double GetcPed() const { return _cPed; };

     void SetcPed(double cPed) { _cPed = cPed; };

     double GetaWall() const { return _aWall; };

     void SetaWall(double aWall) { _aWall = aWall; };

     double GetbWall() const { return _bWall; };

     void SetbWall(double bWall) { _bWall = bWall; };

     double GetcWall() const { return _cWall; };

     void SetcWall(double cWall) { _cWall = cWall; };

     double GetDWall() const { return _dWall; };

     void SetDWall(double dWall) { _dWall = dWall; };

     double GetDPed() const { return _dPed; };

     void SetDPed(double dPed) { _dPed = dPed; };

     double GetIntPWidthPed() const { return _intPWidthPed; };

     void SetIntPWidthPed(double intPWidthPed) { _intPWidthPed = intPWidthPed; };

     double GetIntPWidthWall() const { return _intPWidthWall; };

     void SetIntPWidthWall(double intPWidthWall) { _intPWidthWall = intPWidthWall; };

     double GetMaxFPed() const { return _maxFPed; };

     void SetMaxFPed(double maxFPed) { _maxFPed = maxFPed; };

     double GetMaxFWall() const { return _maxFWall; };

     void SetMaxFWall(double maxFWall) { _maxFWall = maxFWall; };

     double GetDistEffMaxPed() const { return _distEffMaxPed; };

     void SetDistEffMaxPed(double distEffMaxPed) { _distEffMaxPed = distEffMaxPed; };

     double GetDistEffMaxWall() const { return _distEffMaxWall; };

     void SetDistEffMaxWall(double distEffMaxWall) { _distEffMaxWall = distEffMaxWall; };

     const std::string& GetHostname() const { return _hostname; };

     void SetHostname(std::string hostname) { _hostname = hostname; };

     const std::string& GetTrajectoriesFile() const { return _trajectoriesFile; };

     void SetTrjectoriesFile(std::string trajectoriesFile) { _trajectoriesFile = trajectoriesFile; };

     const std::string& GetErrorLogFile() const { return _errorLogFile; };

     void SetErrorLogFile(std::string errorLogFile) { _errorLogFile = errorLogFile; };

     const std::string& GetProjectFile() const { return _projectFile; };

     void SetProjectFile(std::string projectFile) { _projectFile = projectFile; };

     const std::string& GetGeometryFile() const { return _geometryFile; };

     void SetGeometryFile(std::string geometryFile) { _geometryFile = geometryFile; };

     const std::string& GetProjectRootDir() const { return _projectRootDir; };

     void SetProjectRootDir(std::string projectRootDir) { _projectRootDir = projectRootDir; };

     bool ShowStatistics() const { return _showStatistics; };

     void SetShowStatistics(bool showStatistics) { _showStatistics = showStatistics; };

     const FileFormat& GetFileFormat() const { return _fileFormat; };

     void SetFileFormat(FileFormat fileFormat) { _fileFormat = fileFormat; };

     const std::map<int, std::shared_ptr<AgentsParameters> >& GetAgentsParameters() const { return _agentsParameters; };

     void AddAgentsParameters(std::shared_ptr<AgentsParameters> agentsParameters,
               int id) { _agentsParameters[id] = agentsParameters; };

#ifdef _JPS_AS_A_SERVICE

     const bool GetRunAsService() const { return _runAsService; };

     void SetRunAsService(bool runAsService) { _runAsService = runAsService; };

     const int GetServicePort() const { return _servicePort; };

     void SetServicePort(int servicePort) { _servicePort = servicePort; };

     std::shared_ptr<HybridSimulationManager> GetHybridSimulationManager() { return _hybridSimulationManager; };

     void SetHybridSimulationManager(std::shared_ptr<HybridSimulationManager> hybridSimulationManager)
     {
          _hybridSimulationManager = hybridSimulationManager;
     };

     const hybridsim::Scenario* GetScenario() const { return _scenario; };

     void SetScenario(const hybridsim::Scenario* scenario) { _scenario = scenario; };

     const bool GetDumpScenario() const { return _dumpScenario; };

     void SetDumpScenario(bool dumpScenario) { _dumpScenario = dumpScenario; };
#endif

private:
     int _solver;
     std::shared_ptr<RoutingEngine> _routingEngine;
     int _maxOpenMPThreads;
     int _log;
     int _port;
     unsigned int _seed;
     double _fps;
     double _linkedCellSize;
     std::shared_ptr<OperationalModel> _model;
     double _tMax;
     double _dT;
     int _isPeriodic;
     double _nuPed;
     double _nuWall;
     double _aPed;
     double _bPed;
     double _cPed;
     double _aWall;
     double _bWall;
     double _cWall;
     double _dWall;
     double _dPed;
     double _intPWidthPed;
     double _intPWidthWall;
     double _maxFPed;
     double _maxFWall;
     double _distEffMaxPed;
     double _distEffMaxWall;
     std::string _hostname;
     std::string _trajectoriesFile;
     std::string _errorLogFile;
     std::string _projectFile;
     std::string _geometryFile;
     std::string _projectRootDir;
     bool _showStatistics;
     FileFormat _fileFormat;
     std::map<int, std::shared_ptr<AgentsParameters> > _agentsParameters;
#ifdef _JPS_AS_A_SERVICE
     bool _runAsService;
     int _servicePort;
     std::shared_ptr<HybridSimulationManager> _hybridSimulationManager;
     const hybridsim::Scenario* _scenario;
     bool _dumpScenario;
#endif


};

#endif //JPSCORE_CONFIGURATION_H
