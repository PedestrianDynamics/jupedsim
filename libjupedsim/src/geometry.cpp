// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "jupedsim/geometry.h"

#include "jupedsim/error.h"

#include "Conversion.hpp"
#include "ErrorMessage.hpp"

#include <CollisionGeometry.hpp>
#include <GeometryBuilder.hpp>

using jupedsim::detail::intoJPS_Point;
using jupedsim::detail::intoPoint;
using jupedsim::detail::intoTuple;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// GeometryBuilder
////////////////////////////////////////////////////////////////////////////////////////////////////

JPS_GeometryBuilder JPS_GeometryBuilder_Create()
{
    return reinterpret_cast<JPS_GeometryBuilder>(new GeometryBuilder{});
}

void JPS_GeometryBuilder_AddAccessibleArea(
    JPS_GeometryBuilder handle,
    const JPS_Point* polygon,
    size_t lenPolygon)
{
    assert(handle != nullptr);
    auto builder = reinterpret_cast<GeometryBuilder*>(handle);
    std::vector<Point> loop{};
    loop.reserve(lenPolygon);
    std::transform(polygon, polygon + lenPolygon, std::back_inserter(loop), intoPoint);
    builder->AddAccessibleArea(loop);
}

void JPS_GeometryBuilder_ExcludeFromAccessibleArea(
    JPS_GeometryBuilder handle,
    const JPS_Point* polygon,
    size_t lenPolygon)
{
    assert(handle != nullptr);
    auto builder = reinterpret_cast<GeometryBuilder*>(handle);
    std::vector<Point> loop{};
    loop.reserve(lenPolygon);
    std::transform(polygon, polygon + lenPolygon, std::back_inserter(loop), intoPoint);
    builder->ExcludeFromAccessibleArea(loop);
}

JPS_Geometry JPS_GeometryBuilder_Build(JPS_GeometryBuilder handle, JPS_ErrorMessage* errorMessage)
{
    assert(handle != nullptr);
    auto builder = reinterpret_cast<GeometryBuilder*>(handle);
    JPS_Geometry result{};
    try {
        result = reinterpret_cast<JPS_Geometry>(new CollisionGeometry(builder->Build()));
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
size_t JPS_Geometry_GetBoundarySize(JPS_Geometry handle)
{
    assert(handle);
    const auto geo = reinterpret_cast<CollisionGeometry const*>(handle);
    return std::get<0>(geo->AccessibleArea()).size();
}

const JPS_Point* JPS_Geometry_GetBoundaryData(JPS_Geometry handle)
{
    assert(handle);
    const auto geo = reinterpret_cast<CollisionGeometry const*>(handle);
    return reinterpret_cast<const JPS_Point*>(std::get<0>(geo->AccessibleArea()).data());
}

size_t JPS_Geometry_GetHoleCount(JPS_Geometry handle)
{
    assert(handle);
    const auto geo = reinterpret_cast<CollisionGeometry const*>(handle);
    return std::get<1>(geo->AccessibleArea()).size();
}

size_t
JPS_Geometry_GetHoleSize(JPS_Geometry handle, size_t hole_index, JPS_ErrorMessage* errorMessage)
{
    assert(handle);
    const auto geo = reinterpret_cast<CollisionGeometry const*>(handle);
    try {
        return std::get<1>(geo->AccessibleArea()).at(hole_index).size();
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
    return 0;
}

const JPS_Point*
JPS_Geometry_GetHoleData(JPS_Geometry handle, size_t hole_index, JPS_ErrorMessage* errorMessage)
{
    assert(handle);
    const auto geo = reinterpret_cast<CollisionGeometry const*>(handle);
    try {
        return reinterpret_cast<const JPS_Point*>(
            std::get<1>(geo->AccessibleArea()).at(hole_index).data());
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
    return nullptr;
}

void JPS_Geometry_Free(JPS_Geometry handle)
{
    delete reinterpret_cast<CollisionGeometry const*>(handle);
}
