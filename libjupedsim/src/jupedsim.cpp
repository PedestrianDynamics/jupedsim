#include "jupedsim/jupedsim.h"

#include "CollisionGeometry.hpp"
#include "ErrorMessage.hpp"
#include "Logger.hpp"

#include <Area.hpp>
#include <GCFMModel.hpp>
#include <Geometry.hpp>
#include <GeometryBuilder.hpp>
#include <Logger.hpp>
#include <OperationalModel.hpp>
#include <OperationalModelType.hpp>
#include <Pedestrian.hpp>
#include <Point.hpp>
#include <RoutingEngine.hpp>
#include <Simulation.hpp>
#include <VelocityModel.hpp>

#include <cassert>
#include <exception>
#include <memory>
#include <string>
#include <vector>

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Logging
////////////////////////////////////////////////////////////////////////////////////////////////////
JUPEDSIM_API void JPS_Logging_SetDebugCallback(JPS_LoggingCallBack callback)
{
    if(callback) {
        Logging::Logger::Instance().SetDebugCallback(
            [callback](const std::string& msg) { callback(msg.c_str()); });
    } else {
        Logging::Logger::Instance().ClearDebugCallback();
    }
}

JUPEDSIM_API void JPS_Logging_SetInfoCallback(JPS_LoggingCallBack callback)
{
    if(callback) {
        Logging::Logger::Instance().SetInfoCallback(
            [callback](const std::string& msg) { callback(msg.c_str()); });
    } else {
        Logging::Logger::Instance().ClearInfoCallback();
    }
}

JUPEDSIM_API void JPS_Logging_SetWarningCallback(JPS_LoggingCallBack callback)
{
    if(callback) {
        Logging::Logger::Instance().SetWarningCallback(
            [callback](const std::string& msg) { callback(msg.c_str()); });
    } else {
        Logging::Logger::Instance().ClearWarningCallback();
    }
}

JUPEDSIM_API void JPS_Logging_SetErrorCallback(JPS_LoggingCallBack callback)
{
    if(callback) {
        Logging::Logger::Instance().SetErrorCallback(
            [callback](const std::string& msg) { callback(msg.c_str()); });
    } else {
        Logging::Logger::Instance().ClearErrorCallback();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// ErrorMessage
////////////////////////////////////////////////////////////////////////////////////////////////////
const char* JPS_ErrorMessage_GetMessage(JPS_ErrorMessage handle)
{
    JPS_ErrorMessage_t* msg = reinterpret_cast<JPS_ErrorMessage_t*>(handle);
    return msg->message.c_str();
}

void JPS_ErrorMessage_Free(JPS_ErrorMessage handle)
{
    delete reinterpret_cast<JPS_ErrorMessage_t*>(handle);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Operational Model
////////////////////////////////////////////////////////////////////////////////////////////////////
JPS_OperationalModel JPS_OperationalModel_Create_VelocityModel(
    double a_Ped,
    double D_Ped,
    double a_Wall,
    double D_Wall,
    JPS_ErrorMessage* errorMessage)
{
    JPS_OperationalModel model{};
    try {
        model =
            reinterpret_cast<JPS_OperationalModel>(new VelocityModel(a_Ped, D_Ped, a_Wall, D_Wall));
    } catch(const std::exception& ex) {
        if(errorMessage) {
            *errorMessage = reinterpret_cast<JPS_ErrorMessage>(new JPS_ErrorMessage_t{ex.what()});
        }
    } catch(...) {
        if(errorMessage) {
            *errorMessage = reinterpret_cast<JPS_ErrorMessage>(
                new JPS_ErrorMessage_t{"Unknown internal error."});
        }
    }
    return model;
}

JPS_OperationalModel JPS_OperationalModel_Create_GCFMModel(
    double nu_Ped,
    double nu_Wall,
    double dist_eff_Ped,
    double dist_eff_Wall,
    double intp_width_Ped,
    double intp_width_Wall,
    double maxf_Ped,
    double maxf_Wall,
    JPS_ErrorMessage* errorMessage)
{
    JPS_OperationalModel model{};
    try {
        model = reinterpret_cast<JPS_OperationalModel>(new GCFMModel(
            nu_Ped,
            nu_Wall,
            dist_eff_Ped,
            dist_eff_Wall,
            intp_width_Ped,
            intp_width_Wall,
            maxf_Ped,
            maxf_Wall));
    } catch(const std::exception& ex) {
        if(errorMessage) {
            *errorMessage = reinterpret_cast<JPS_ErrorMessage>(new JPS_ErrorMessage_t{ex.what()});
        }
    } catch(...) {
        if(errorMessage) {
            *errorMessage = reinterpret_cast<JPS_ErrorMessage>(
                new JPS_ErrorMessage_t{"Unknown internal error."});
        }
    }
    return model;
}

void JPS_OperationalModel_Free(JPS_OperationalModel handle)
{
    delete reinterpret_cast<OperationalModel*>(handle);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// GeometryBuilder
////////////////////////////////////////////////////////////////////////////////////////////////////

JPS_GeometryBuilder JPS_GeometryBuilder_Create()
{
    return reinterpret_cast<JPS_GeometryBuilder>(new GeometryBuilder{});
}

void JPS_GeometryBuilder_AddAccessibleArea(
    JPS_GeometryBuilder handle,
    double* points,
    size_t pointCount)
{
    assert(handle != nullptr);
    auto builder = reinterpret_cast<GeometryBuilder*>(handle);
    std::vector<Point> lineLoop{};
    lineLoop.reserve(pointCount);
    for(size_t pointIndex = 0; pointIndex < pointCount; ++pointIndex) {
        lineLoop.emplace_back(points[pointIndex * 2], points[pointIndex * 2 + 1]);
    }
    // TODO(kkratz): Consider adding a move version of 'AddAccessibleArea'
    builder->AddAccessibleArea(lineLoop);
}

void JPS_GeometryBuilder_ExcludeFromAccessibleArea(
    JPS_GeometryBuilder handle,
    double* points,
    size_t pointCount)
{
    assert(handle != nullptr);
    auto builder = reinterpret_cast<GeometryBuilder*>(handle);
    std::vector<Point> lineLoop{};
    lineLoop.reserve(pointCount);
    for(size_t pointIndex = 0; pointIndex < pointCount; ++pointIndex) {
        lineLoop.emplace_back(points[pointIndex * 2], points[pointIndex * 2 + 1]);
    }
    // TODO(kkratz): Consider adding a move version of 'AddAccessibleArea'
    builder->ExcludeFromAccessibleArea(lineLoop);
}

JPS_Geometry JPS_GeometryBuilder_Build(JPS_GeometryBuilder handle, JPS_ErrorMessage* errorMessage)
{
    assert(handle != nullptr);
    auto builder = reinterpret_cast<GeometryBuilder*>(handle);
    JPS_Geometry result{};
    try {
        result = reinterpret_cast<JPS_Geometry>(new Geometry(builder->Build()));
    } catch(const std::exception& ex) {
        if(errorMessage) {
            *errorMessage = reinterpret_cast<JPS_ErrorMessage>(new JPS_ErrorMessage_t{ex.what()});
        }
    } catch(...) {
        if(errorMessage) {
            *errorMessage = reinterpret_cast<JPS_ErrorMessage>(
                new JPS_ErrorMessage_t{"Unknown internal error."});
        }
    }
    return result;
}

void JPS_GeometryBuilder_Free(JPS_GeometryBuilder handle)
{
    delete reinterpret_cast<GeometryBuilder*>(handle);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Geometry
////////////////////////////////////////////////////////////////////////////////////////////////////
void JPS_Geometry_Free(JPS_Geometry handle)
{
    delete reinterpret_cast<Geometry*>(handle);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// AreasBuilder
////////////////////////////////////////////////////////////////////////////////////////////////////
JPS_AreasBuilder JPS_AreasBuilder_Create()
{
    return reinterpret_cast<JPS_AreasBuilder>(new AreasBuilder{});
}

void JPS_AreasBuilder_AddArea(
    JPS_AreasBuilder handle,
    uint64_t id,
    double* points,
    size_t pointCount,
    const char** tags,
    size_t tagCount)
{
    assert(handle);
    auto builder = reinterpret_cast<AreasBuilder*>(handle);
    std::vector<Point> lineLoop{};
    lineLoop.reserve(pointCount);
    for(size_t pointIndex = 0; pointIndex < pointCount; ++pointIndex) {
        lineLoop.emplace_back(points[pointIndex * 2], points[pointIndex * 2 + 1]);
    }
    std::vector<std::string> labels{};
    labels.reserve(tagCount);
    for(size_t tagIndex = 0; tagIndex < tagCount; ++tagIndex) {
        labels.emplace_back(tags[tagIndex]);
    }
    builder->AddArea(id, lineLoop, labels);
}

JPS_Areas JPS_AreasBuilder_Build(JPS_AreasBuilder handle, JPS_ErrorMessage* errorMessage)
{
    assert(handle);
    auto builder = reinterpret_cast<AreasBuilder*>(handle);
    JPS_Areas result{};
    try {
        result = reinterpret_cast<JPS_Areas>(new Areas(builder->Build()));
    } catch(const std::exception& ex) {
        if(errorMessage) {
            *errorMessage = reinterpret_cast<JPS_ErrorMessage>(new JPS_ErrorMessage_t{ex.what()});
        }
    } catch(...) {
        if(errorMessage) {
            *errorMessage = reinterpret_cast<JPS_ErrorMessage>(
                new JPS_ErrorMessage_t{"Unknown internal error."});
        }
    }
    return result;
}

void JPS_AreasBuilder_Free(JPS_AreasBuilder handle)
{
    delete reinterpret_cast<AreasBuilder*>(handle);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Areas
////////////////////////////////////////////////////////////////////////////////////////////////////
void JPS_Areas_Free(JPS_Areas handle)
{
    delete reinterpret_cast<Areas*>(handle);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Agent
////////////////////////////////////////////////////////////////////////////////////////////////////
double JPS_Agent_PositionX(JPS_Agent handle)
{
    assert(handle != nullptr);
    const auto agent = reinterpret_cast<Pedestrian*>(handle);
    return agent->GetPos().x;
}

double JPS_Agent_PositionY(JPS_Agent handle)
{
    assert(handle != nullptr);
    const auto agent = reinterpret_cast<Pedestrian*>(handle);
    return agent->GetPos().y;
}

double JPS_Agent_OrientationX(JPS_Agent handle)
{
    assert(handle != nullptr);
    const auto agent = reinterpret_cast<Pedestrian*>(handle);
    return agent->GetEllipse().GetCosPhi();
}

double JPS_Agent_OrientationY(JPS_Agent handle)
{
    assert(handle != nullptr);
    const auto agent = reinterpret_cast<Pedestrian*>(handle);
    return agent->GetEllipse().GetSinPhi();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Simulation
////////////////////////////////////////////////////////////////////////////////////////////////////
JPS_Simulation JPS_Simulation_Create(
    JPS_OperationalModel model,
    JPS_Geometry geometry,
    JPS_Areas areas,
    double dT,
    JPS_ErrorMessage* errorMessage)
{
    assert(model);
    assert(geometry);
    assert(areas);
    JPS_Simulation result{};
    try {
        auto geometryInternal = reinterpret_cast<Geometry*>(geometry);
        auto collisionGeometry =
            std::make_unique<CollisionGeometry>(*geometryInternal->collisionGeometry);
        auto routingEngine = geometryInternal->routingEngine->Clone();

        auto modelInternal = reinterpret_cast<OperationalModel*>(model);
        auto model = modelInternal->Clone();

        auto areasInternal = reinterpret_cast<Areas*>(areas);
        auto areas = std::make_unique<Areas>(*areasInternal);

        result = reinterpret_cast<JPS_Simulation>(new Simulation(
            std::move(model),
            std::move(collisionGeometry),
            std::move(routingEngine),
            std::move(areas),
            dT));
    } catch(const std::exception& ex) {
        if(errorMessage) {
            *errorMessage = reinterpret_cast<JPS_ErrorMessage>(new JPS_ErrorMessage_t{ex.what()});
        }
    } catch(...) {
        if(errorMessage) {
            *errorMessage = reinterpret_cast<JPS_ErrorMessage>(
                new JPS_ErrorMessage_t{"Unknown internal error."});
        }
    }
    return result;
}

JPS_AgentId JPS_Simulation_AddAgent(
    JPS_Simulation handle,
    JPS_AgentParameters parameters,
    JPS_ErrorMessage* errorMessage)
{
    assert(handle);
    auto simulation = reinterpret_cast<Simulation*>(handle);
    JPS_AgentId result{};
    try {
        result = simulation->AddAgent(
            Point(parameters.positionX, parameters.positionY),
            Point(parameters.orientationX, parameters.orientationY),
            parameters.Av,
            parameters.AMin,
            parameters.BMax,
            parameters.BMin,
            parameters.Tau,
            parameters.T,
            parameters.v0,
            parameters.destinationAreaId);
    } catch(const std::exception& ex) {
        if(errorMessage) {
            *errorMessage = reinterpret_cast<JPS_ErrorMessage>(new JPS_ErrorMessage_t{ex.what()});
        }
    } catch(...) {
        if(errorMessage) {
            *errorMessage = reinterpret_cast<JPS_ErrorMessage>(
                new JPS_ErrorMessage_t{"Unknown internal error."});
        }
    }
    return result;
}

JPS_Agent
JPS_Simulation_ReadAgent(JPS_Simulation handle, JPS_AgentId id, JPS_ErrorMessage* errorMessage)
{
    assert(handle);
    auto simulation = reinterpret_cast<Simulation*>(handle);
    JPS_Agent result{};
    try {
        result = reinterpret_cast<JPS_Agent>(simulation->AgentPtr(id));
    } catch(const std::exception& ex) {
        if(errorMessage) {
            *errorMessage = reinterpret_cast<JPS_ErrorMessage>(new JPS_ErrorMessage_t{ex.what()});
        }
    } catch(...) {
        if(errorMessage) {
            *errorMessage = reinterpret_cast<JPS_ErrorMessage>(
                new JPS_ErrorMessage_t{"Unknown internal error."});
        }
    }
    return result;
}

size_t JPS_Simulation_RemovedAgents(JPS_Simulation handle, const JPS_AgentId** data)
{
    assert(handle);
    auto simulation = reinterpret_cast<Simulation*>(handle);
    const auto& agents = simulation->RemovedAgents();
    *data = agents.data();
    return agents.size();
}

bool JPS_Simulation_Iterate(JPS_Simulation handle, JPS_ErrorMessage* errorMessage)
{
    assert(handle);
    auto simulation = reinterpret_cast<Simulation*>(handle);
    bool result = false;
    try {
        simulation->Iterate();
        result = true;
    } catch(const std::exception& ex) {
        if(errorMessage) {
            *errorMessage = reinterpret_cast<JPS_ErrorMessage>(new JPS_ErrorMessage_t{ex.what()});
        }
    } catch(...) {
        if(errorMessage) {
            *errorMessage = reinterpret_cast<JPS_ErrorMessage>(
                new JPS_ErrorMessage_t{"Unknown internal error."});
        }
    }
    return result;
}

size_t JPS_Simulation_AgentCount(JPS_Simulation handle)
{
    assert(handle);
    auto simulation = reinterpret_cast<Simulation*>(handle);
    return simulation->AgentCount();
}

uint64_t JPS_Simulation_IterationCount(JPS_Simulation handle)
{
    assert(handle);
    auto simulation = reinterpret_cast<Simulation*>(handle);
    return simulation->Iteration();
}

void JPS_Simulation_Free(JPS_Simulation handle)
{
    delete reinterpret_cast<Simulation*>(handle);
}
