/**
 * \file        IODispatcher.h
 * \date        Nov 20, 2010
 * \version     v0.7
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
 *
 *
 **/
#pragma once



#include "geometry/Building.h"

#include <cstring>
#include <vector>

class OutputHandler;
extern OutputHandler* Log;

class Trajectories;

class  AgentsSource;

class IODispatcher
{
private:
     std::vector<Trajectories*> _outputHandlers;

public:
     IODispatcher();
     virtual ~IODispatcher();

     void AddIO(Trajectories* ioh);
     const std::vector<Trajectories*>& GetIOHandlers();
     void WriteHeader(long nPeds, double fps, Building* building, int seed, int count);
     void WriteGeometry(Building* building);
     void WriteFrame(int frameNr, Building* building);
     void WriteFooter();
     void WriteSources(const std::vector<std::shared_ptr<AgentsSource> >);

};

class Trajectories
{
public:
     Trajectories()
     : _outputHandler(nullptr) {}
     virtual ~Trajectories() = default;
     virtual void WriteHeader(long nPeds, double fps, Building* building, int seed, int count)=0;
     virtual void WriteGeometry(Building* building)=0;
     virtual void WriteFrame(int frameNr, Building* building)=0;
     virtual void WriteFooter()=0;
     virtual void WriteSources(const std::vector<std::shared_ptr<AgentsSource> >)=0;

    void Write(const std::string& str)
     {
          _outputHandler->Write(str);
     }
     void SetOutputHandler(std::shared_ptr<OutputHandler> outputHandler)
     {
          _outputHandler=outputHandler;
     }

     template<typename A>
         bool IsElementInVector(const std::vector<A> &vec, A& el)
         {
              typename std::vector<A>::const_iterator it;
              it = std::find(vec.begin(), vec.end(), el);
              if (it == vec.end())
              {
                   return false;
              }
              else
              {
                   return true;
              }
         }

protected:
     std::shared_ptr<OutputHandler> _outputHandler;
};


class TrajectoriesJPSV04: public Trajectories {

public:
     TrajectoriesJPSV04(){};
     virtual ~TrajectoriesJPSV04(){};

     virtual void WriteHeader(long nPeds, double fps, Building* building, int seed, int count);
     virtual void WriteGeometry(Building* building);
     virtual void WriteFrame(int frameNr, Building* building);
     virtual void WriteFooter();
     std::string  WritePed(Pedestrian* ped);
};

class TrajectoriesJPSV05: public Trajectories {

public:
     TrajectoriesJPSV05(){};
     virtual ~TrajectoriesJPSV05(){};

     virtual void WriteHeader(long nPeds, double fps, Building* building, int seed, int count);
     virtual void WriteGeometry(Building* building);
     virtual void WriteFrame(int frameNr, Building* building);
     virtual void WriteFooter();
     virtual void WriteSources(const std::vector<std::shared_ptr<AgentsSource> >);
};


class TrajectoriesFLAT: public Trajectories
{

public:
     TrajectoriesFLAT();
     virtual ~TrajectoriesFLAT()
     {
     }
     ;

     virtual void WriteHeader(long nPeds, double fps, Building* building, int seed, int count);
     virtual void WriteGeometry(Building* building);
     virtual void WriteFrame(int frameNr, Building* building);
     virtual void WriteFooter();
     virtual void WriteSources(const std::vector<std::shared_ptr<AgentsSource> >);

};

class TrajectoriesVTK: public Trajectories
{

public:
     TrajectoriesVTK();
     virtual ~TrajectoriesVTK()
     {
     }
     ;

     virtual void WriteHeader(long nPeds, double fps, Building* building, int seed, int count);
     virtual void WriteGeometry(Building* building);
     virtual void WriteFrame(int frameNr, Building* building);
     virtual void WriteFooter();
     virtual void WriteSources(const std::vector<std::shared_ptr<AgentsSource> >);

};

class TrajectoriesXML_MESH: public Trajectories
{

public:
     TrajectoriesXML_MESH()
     {
     }
     ;
     virtual ~TrajectoriesXML_MESH()
     {
     }
     ;

     //virtual void WriteHeader(int nPeds, double fps, Building* building, int seed);
     //virtual void WriteFrame(int frameNr, Building* building);
     //virtual void WriteFooter();
     virtual void WriteGeometry(Building* building);
     virtual void WriteSources(const std::vector<std::shared_ptr<AgentsSource> >);
};

class TrajectoriesJPSV06: public Trajectories
{

public:
     TrajectoriesJPSV06(){};
     virtual ~TrajectoriesJPSV06(){ };

     virtual void WriteHeader(long nPeds, double fps, Building* building, int seed, int count);
     virtual void WriteGeometry(Building* building);
     virtual void WriteFrame(int frameNr, Building* building);
     virtual void WriteFooter();
     virtual void WriteSources(const std::vector<std::shared_ptr<AgentsSource> >);

};
