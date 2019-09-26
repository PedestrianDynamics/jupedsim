///**
// * \file        IODispatcher.h
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
//#pragma once
//
//#include "geometry/Building.h"
//#include "IO/Trajectories.h"
//#include <map>
//#include <memory>
//class OutputHandler;
//
//class  AgentsSource;
//
//class IODispatcher
//{
//private:
////     Trajectories& _outputHandler;
//     std::unique_ptr<Trajectories> _outputHandler;
//
//    std::unique_ptr<Trajectories> Init(FileFormat format);
////    Trajectories& Init(FileFormat format);
//public:
//     IODispatcher(FileFormat format) : _outputHandler(Init(format)) {};
////     IODispatcher(FileFormat format) : _outputHandler(format == FileFormat::XML ? TrajectoriesJPSV05() : TrajectoriesFLAT()) {};
//     virtual ~IODispatcher() = default;
//
////     void AddIO(FileFormat format);
////     void AddIO(Trajectories* ioh);
////     const std::vector<Trajectories*>& GetIOHandlers();
//     void WriteHeader(long nPeds, double fps, Building* building, int seed, int count);
//     void WriteGeometry(Building* building);
//     void WriteFrame(int frameNr, Building* building);
//     void WriteFooter();
//     void WriteSources(std::vector<std::shared_ptr<AgentsSource>>&);
//};
