// Copyright (c) 2019 GeometryFactory(France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/v6.0.1/Intersections_3/include/CGAL/Intersections_3/internal/Plane_3_Point_3_do_intersect.h $
// $Id: include/CGAL/Intersections_3/internal/Plane_3_Point_3_do_intersect.h 50cfbde3b84 $
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Maxime Gimeno

#ifndef CGAL_INTERNAL_INTERSECTIONS_3_PLANE_3_POINT_3_DO_INTERSECT_H
#define CGAL_INTERNAL_INTERSECTIONS_3_PLANE_3_POINT_3_DO_INTERSECT_H

namespace CGAL {
namespace Intersections {
namespace internal {

template <class K>
inline typename K::Boolean
do_intersect(const typename K::Point_3& pt,
             const typename K::Plane_3& plane,
             const K& k)
{
  return k.has_on_3_object()(plane,pt);
}

template <class K>
inline typename K::Boolean
do_intersect(const typename K::Plane_3& plane,
             const typename K::Point_3& pt,
             const K& k)
{
  return k.has_on_3_object()(plane, pt);
}

} // namespace internal
} // namespace Intersections
} // namespace CGAL

#endif // CGAL_INTERNAL_INTERSECTIONS_3_PLANE_3_POINT_3_DO_INTERSECT_H
