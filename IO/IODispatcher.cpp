///**
// * \file        IODispatcher.cpp
// * \date        Nov 20, 2010
// * \version     v0.7
// * \copyright   <2009-2015> Forschungszentrum Jülich GmbH. All rights reserved.
// *
// * \section License
// * This file is part of JuPedSim.
// *
// * JuPedSim is free software: you can redistribute it and/or modify
// * it under the terms of the GNU Lesser General Public License as published by
// * the Free Software Foundation, either version 3 of the License, or
// * any later version.
// *
// * JuPedSim is distributed in the hope that it will be useful,
// * but WITHOUT ANY WARRANTY; without even the implied warranty of
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// * GNU General Public License for more details.
// *
// * You should have received a copy of the GNU Lesser General Public License
// * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
// *
// * \section Description
// *
// *
// **/
//#include "IODispatcher.h"
//
//#include "OutputHandler.h"
//
//#include "general/Filesystem.h"
//#include "geometry/SubRoom.h"
//#include "mpi/LCGrid.h"
//#include "pedestrian/Pedestrian.h"
//#include "pedestrian/AgentsSource.h"
//#include "Trajectories.h"
//
//#include <tinyxml.h>
//
//#define _USE_MATH_DEFINES
//
//
////IODispatcher::IODispatcher(FileFormat format)
////{
////     switch (format) {
////     case FileFormat::XML:
////          TrajectoriesJPSV05 traj;
////          _outputHandlers[FileFormat::XML] = traj;
//////          _outputHandler = TrajectoriesJPSV05();
////          break;
////     case FileFormat::TXT:
////          TrajectoriesFLAT traj;
////          _outputHandlers[FileFormat::TXT] = traj;
//////          _outputHandler = TrajectoriesFLAT();
////          break;
////     default:
////          break;
////     }
////}
//
////IODispatcher::~IODispatcher()
////{
//////     if (_outputHandler){
//////          _outputHandler.reset();
//////     }
//////     for (int i = 0; i < (int) _outputHandlers.size(); ++i)
//////          delete _outputHandlers[i];
//////     _outputHandlers.clear();
////}
//
////void IODispatcher::AddIO(FileFormat format){
////     switch (format) {
////     case FileFormat::XML:
////          TrajectoriesJPSV05 traj;
////          _outputHandlers[FileFormat::XML] = traj;
//////          _outputHandler = TrajectoriesJPSV05();
////          break;
////     case FileFormat::TXT:
////          TrajectoriesFLAT traj;
////          _outputHandlers[FileFormat::TXT] = traj;
//////          _outputHandler = TrajectoriesFLAT();
////          break;
////     default:
////          break;
////     }
////}
//
////void IODispatcher::AddIO(Trajectories* ioh)
////{
////     _outputHandlers.push_back(ioh);
////}
//
//
////const std::vector<Trajectories*>& IODispatcher::GetIOHandlers()
////{
////     return _outputHandlers;
////}
//
//Trajectories& IODispatcher::Init(FileFormat format){
//     switch (format) {
//     case FileFormat::XML:
////          TrajectoriesJPSV05 traj;
//          return std::make_unique<TrajectoriesJPSV05>();
////          return traj;
//     case FileFormat::TXT:
////          TrajectoriesFLAT traj;
////          return traj;
////          _outputHandlers[FileFormat::TXT] = traj;
////          _outputHandler = TrajectoriesFLAT();
//          return std::make_unique<TrajectoriesFLAT>();
//     default:
//          break;
//     }
//
//}
//
//void IODispatcher::WriteHeader(long nPeds, double fps, Building* building, int seed, int count)
//{
//     _outputHandler.WriteHeader(nPeds, fps, building, seed, count);
////     for(auto[ff, output] : _outputHandlers) {
////          output.WriteHeader(nPeds, fps, building, seed, count);
////     }
//
////     for (auto const & it : _outputHandlers)
////     {
////          it->WriteHeader(nPeds, fps, building, seed, count);
////     }
//}
//void IODispatcher::WriteGeometry(Building* building)
//{
//     _outputHandler.WriteGeometry(building);
////     for(auto[ff, output] : _outputHandlers) {
////          output.WriteGeometry(building);
////     }
//
////     for(auto const & it : _outputHandlers)
////     {
////          it->WriteGeometry(building);
////     }
//}
//void IODispatcher::WriteFrame(int frameNr, Building* building)
//{
//     _outputHandler.WriteFrame(frameNr, building);
////     for(auto[ff, output] : _outputHandlers) {
////          output.WriteFrame(frameNr, building);
////     }
//
////     for (auto const & it : _outputHandlers)
////     {
////          it->WriteFrame(frameNr, building);
////     }
//}
//void IODispatcher::WriteFooter()
//{
//     _outputHandler.WriteFooter();
////     for(auto[ff, output] : _outputHandlers) {
////          output.WriteFooter();
////     }
//
////     for(auto const it : _outputHandlers)
////     {
////          it->WriteFooter();
////     }
//}
// void IODispatcher::WriteSources(const std::vector<std::shared_ptr<AgentsSource>>& sources)
// {
//      _outputHandler.WriteSources(sources);
////      for(auto[ff, output] : _outputHandlers) {
////           output.WriteSources(sources);
////      }
//
////      for(auto const it : _outputHandlers)
////      {
////           it->WriteSources(sources);
////      }
// }
