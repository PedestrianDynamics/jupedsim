// Copyright (c) 1997-2021
// Utrecht University (The Netherlands),
// ETH Zurich (Switzerland),
// INRIA Sophia-Antipolis (France),
// Max-Planck-Institute Saarbruecken (Germany),
// and Tel-Aviv University (Israel).
// GeometryFactory (France)
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/v5.6/Intersections_3/include/CGAL/Intersections_3/Plane_3_Plane_3_Plane_3.h $
// $Id: Plane_3_Plane_3_Plane_3.h 3a4e230 2022-11-22T12:22:42+01:00 Mael Rouxel-Labbé
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Geert-Jan Giezeman,
//                 Andreas Fabri
//

#ifndef CGAL_INTERSECTIONS_3_PLANE_3_PLANE_3_PLANE_3_H
#define CGAL_INTERSECTIONS_3_PLANE_3_PLANE_3_PLANE_3_H

#include <CGAL/Intersection_traits_3.h>
#include <CGAL/Intersections_3/internal/Plane_3_Plane_3_Plane_3_do_intersect.h>
#include <CGAL/Intersections_3/internal/Plane_3_Plane_3_Plane_3_intersection.h>

#include <CGAL/Plane_3.h>

namespace CGAL {

template <class K>
inline
typename K::Boolean
do_intersect(const Plane_3<K>& plane1,
             const Plane_3<K>& plane2,
             const Plane_3<K>& plane3)
{
  return K().do_intersect_3_object()(plane1, plane2, plane3);
}

// the special plane_3 function
template <class K>
inline
decltype(auto)
intersection(const Plane_3<K>& plane1,
             const Plane_3<K>& plane2,
             const Plane_3<K>& plane3)
{
  return K().intersect_3_object()(plane1, plane2, plane3);
}

template <class K>
inline
boost::optional<typename K::Point_3>
intersection_point_for_polyhedral_envelope(const Plane_3<K>& p0,
                                           const Plane_3<K>& p1,
                                           const Plane_3<K>& p2)
{
  return K().intersect_point_3_for_polyhedral_envelope_object()(p0, p1, p2);
}

} // namespace CGAL

#endif // CGAL_INTERSECTIONS_3_PLANE_3_PLANE_3_PLANE_3_H
