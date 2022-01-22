#pragma once

#include "OldEvent.hpp"

#include <string>
//============================================================================
// DoorCloseEvent
//============================================================================
class DoorCloseEvent : public OldEvent
{
private:
    Building * _building;
    int _doorID;

public:
    DoorCloseEvent(Building * building, int doorID, double time);
    void Process() override;
    [[nodiscard]] std::string ToString() const override;
};

//============================================================================
// DoorTempCloseEvent
//============================================================================
class DoorTempCloseEvent : public OldEvent
{
private:
    Building * _building;
    int _doorID;

public:
    DoorTempCloseEvent(Building * building, int doorID, double time);
    void Process() override;
    [[nodiscard]] std::string ToString() const override;
};

//============================================================================
// DoorOpenEvent
//============================================================================
class DoorOpenEvent : public OldEvent
{
private:
    Building * _building;
    int _doorID;

public:
    DoorOpenEvent(Building * building, int doorID, double time);
    void Process() override;
    [[nodiscard]] std::string ToString() const override;
};

//============================================================================
// DoorResetEvent
//============================================================================
class DoorResetEvent : public OldEvent
{
private:
    Building * _building;
    int _doorID;

public:
    DoorResetEvent(Building * building, int doorID, double time);
    void Process() override;
    [[nodiscard]] std::string ToString() const override;
};
