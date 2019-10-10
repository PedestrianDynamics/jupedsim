#pragma once

#include "general/Macros.h"
#include "geometry/Building.h"
#include "pedestrian/AgentsSource.h"

#include <functional>
#include <vector>

class Trajectories
{
protected:
    std::shared_ptr<OutputHandler> _outputHandler;
    std::set<OptionalOutput> _optionalOutputOptions;
    std::map<OptionalOutput, std::function<std::string(Pedestrian *)>> _optionalOutput;
    std::map<OptionalOutput, std::string> _optionalOutputHeader;
    std::map<OptionalOutput, std::string> _optionalOutputInfo;

public:
    Trajectories()          = default;
    virtual ~Trajectories() = default;
    virtual void WriteHeader(long nPeds, double fps, Building * building, int seed, int count) = 0;
    virtual void WriteGeometry(Building * building)                                            = 0;
    virtual void WriteFrame(int frameNr, Building * building)                                  = 0;
    virtual void WriteFooter()                                                                 = 0;
    virtual void WriteSources(const std::vector<std::shared_ptr<AgentsSource>> &)              = 0;
    virtual void AddOptionalOutput(OptionalOutput option) { _optionalOutputOptions.insert(option); }
    virtual void SetOptionalOutput(std::set<OptionalOutput> options)
    {
        _optionalOutputOptions = options;
    }

    void Write(const std::string & str) { _outputHandler->Write(str); }

    void SetOutputHandler(std::shared_ptr<OutputHandler> outputHandler)
    {
        _outputHandler = std::move(outputHandler);
    }
};

class TrajectoriesXML : public Trajectories
{
public:
    TrajectoriesXML() = default;

    void WriteHeader(long nPeds, double fps, Building * building, int seed, int count) override;
    void WriteGeometry(Building * building) override;
    void WriteFrame(int frameNr, Building * building) override;
    void WriteFooter() override;
    void WriteSources(const std::vector<std::shared_ptr<AgentsSource>> &) override;
};


class TrajectoriesTXT : public Trajectories
{
public:
    TrajectoriesTXT();

    void WriteHeader(long nPeds, double fps, Building * building, int seed, int count) override;
    void WriteGeometry(Building * building) override;
    void WriteFrame(int frameNr, Building * building) override;
    void WriteFooter() override{};
    void WriteSources(const std::vector<std::shared_ptr<AgentsSource>> &) override{};
};