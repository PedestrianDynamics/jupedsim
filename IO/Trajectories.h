#pragma once

#include <vector>

#include "geometry/Building.h"
#include "pedestrian/AgentsSource.h"

class Trajectories
{
protected:
    std::shared_ptr<OutputHandler> _outputHandler;

public:
    Trajectories() = default;
    virtual ~Trajectories() = default;
    virtual void WriteHeader(long nPeds, double fps, Building* building, int seed, int count) = 0;
    virtual void WriteGeometry(Building* building) = 0;
    virtual void WriteFrame(int frameNr, Building* building) = 0;
    virtual void WriteFooter() = 0;
    virtual void WriteSources(const std::vector<std::shared_ptr<AgentsSource>>&) = 0;

    void Write(const std::string& str)
    {
         _outputHandler->Write(str);
    }

    void SetOutputHandler(std::shared_ptr<OutputHandler> outputHandler)
    {
         _outputHandler = outputHandler;
    }

//    template<typename A>
//    bool IsElementInVector(const std::vector<A> &vec, A& el)
//    {
//         typename std::vector<A>::const_iterator it;
//         it = std::find(vec.begin(), vec.end(), el);
//         if (it == vec.end())
//         {
//              return false;
//         }
//         else
//         {
//              return true;
//         }
//    }

};


//class TrajectoriesJPSV04: public Trajectories {
//
//public:
//    TrajectoriesJPSV04() = default;
////    virtual ~TrajectoriesJPSV04() = default;
//
//    virtual void WriteHeader(long nPeds, double fps, Building* building, int seed, int count);
//    virtual void WriteGeometry(Building* building);
//    virtual void WriteFrame(int frameNr, Building* building);
//    virtual void WriteFooter();
//    std::string  WritePed(Pedestrian* ped);
//};

class TrajectoriesJPSV05: public Trajectories {

public:
    TrajectoriesJPSV05()= default;
//    virtual ~TrajectoriesJPSV05(){};

    virtual void WriteHeader(long nPeds, double fps, Building* building, int seed, int count) override;
    virtual void WriteGeometry(Building* building) override;
    virtual void WriteFrame(int frameNr, Building* building) override;
    virtual void WriteFooter() override;
    virtual void WriteSources(const std::vector<std::shared_ptr<AgentsSource>>&) override;
};


class TrajectoriesFLAT: public Trajectories
{

public:
    TrajectoriesFLAT() = default;
//    virtual ~TrajectoriesFLAT()
//    {
//    }
//    ;

    virtual void WriteHeader(long nPeds, double fps, Building* building, int seed, int count) override;
    virtual void WriteGeometry(Building* building) override;
    virtual void WriteFrame(int frameNr, Building* building) override;
    virtual void WriteFooter() override { };
    virtual void WriteSources(const std::vector<std::shared_ptr<AgentsSource>>&) override {};

};

//class TrajectoriesVTK: public Trajectories
//{
//
//public:
//    TrajectoriesVTK() = default;
////    virtual ~TrajectoriesVTK()
////    {
////    }
////    ;
//
//    virtual void WriteHeader(long nPeds, double fps, Building* building, int seed, int count);
//    virtual void WriteGeometry(Building* building);
//    virtual void WriteFrame(int frameNr, Building* building);
//    virtual void WriteFooter();
////    virtual void WriteSources(const std::vector<std::shared_ptr<AgentsSource> >);
//
//};

//class TrajectoriesXML_MESH: public Trajectories
//{
//
//public:
//    TrajectoriesXML_MESH() = default;
////    virtual ~TrajectoriesXML_MESH()   {   }    ;
//
//    //virtual void WriteHeader(int nPeds, double fps, Building* building, int seed);
//    //virtual void WriteFrame(int frameNr, Building* building);
//    //virtual void WriteFooter();
//    virtual void WriteGeometry(Building* building);
//    virtual void WriteSources(const std::vector<std::shared_ptr<AgentsSource> >);
//};

//class TrajectoriesJPSV06: public Trajectories
//{
//
//public:
//    TrajectoriesJPSV06(){};
//    virtual ~TrajectoriesJPSV06(){ };
//
//    virtual void WriteHeader(long nPeds, double fps, Building* building, int seed, int count);
//    virtual void WriteGeometry(Building* building);
//    virtual void WriteFrame(int frameNr, Building* building);
//    virtual void WriteFooter();
//    virtual void WriteSources(const std::vector<std::shared_ptr<AgentsSource> >);
//
//};