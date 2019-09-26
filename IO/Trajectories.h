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
         _outputHandler = std::move(outputHandler);
    }
};

class TrajectoriesXML: public Trajectories {

public:
    TrajectoriesXML()= default;

    void WriteHeader(long nPeds, double fps, Building* building, int seed, int count) override;
    void WriteGeometry(Building* building) override;
    void WriteFrame(int frameNr, Building* building) override;
    void WriteFooter() override;
    void WriteSources(const std::vector<std::shared_ptr<AgentsSource>>&) override;
};


class TrajectoriesTXT: public Trajectories
{

public:
    TrajectoriesTXT() = default;

    void WriteHeader(long nPeds, double fps, Building* building, int seed, int count) override;
    void WriteGeometry(Building* building) override;
    void WriteFrame(int frameNr, Building* building) override;
    void WriteFooter() override { };
    void WriteSources(const std::vector<std::shared_ptr<AgentsSource>>&) override {};
};