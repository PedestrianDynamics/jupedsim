// Copyright (c) 2016  GeometryFactory (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/v6.0.1/Surface_mesh_parameterization/include/CGAL/Surface_mesh_parameterization/internal/kernel_traits.h $
// $Id: include/CGAL/Surface_mesh_parameterization/internal/kernel_traits.h 50cfbde3b84 $
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Mael Rouxel-Labbé

#ifndef CGAL_SURFACE_MESH_PARAMETERIZATION_INTERNAL_KERNEL_TRAITS_H
#define CGAL_SURFACE_MESH_PARAMETERIZATION_INTERNAL_KERNEL_TRAITS_H

#include <CGAL/license/Surface_mesh_parameterization.h>

#include <CGAL/boost/graph/properties.h>
#include <CGAL/Kernel_traits.h>

namespace CGAL {

namespace Surface_mesh_parameterization {

namespace internal {

template <class TM>
class Kernel_traits
{
public:
  typedef typename boost::property_map<TM, CGAL::vertex_point_t>::const_type  PPM;
  typedef typename boost::property_traits<PPM>::value_type                    Point_3;
  typedef typename CGAL::Kernel_traits<Point_3>::Kernel                       Kernel;
};

} // namespace internal

} // namespace Surface_mesh_parameterization

} // namespace CGAL

#endif // CGAL_SURFACE_MESH_PARAMETERIZATION_INTERNAL_KERNEL_TRAITS_H
