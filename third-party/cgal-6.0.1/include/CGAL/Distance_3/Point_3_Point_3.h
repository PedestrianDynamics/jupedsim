// Copyright (c) 1998-2021
// Utrecht University (The Netherlands),
// ETH Zurich (Switzerland),
// INRIA Sophia-Antipolis (France),
// Max-Planck-Institute Saarbruecken (Germany),
// and Tel-Aviv University (Israel).  All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/v6.0.1/Distance_3/include/CGAL/Distance_3/Point_3_Point_3.h $
// $Id: include/CGAL/Distance_3/Point_3_Point_3.h 50cfbde3b84 $
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Geert-Jan Giezeman, Andreas Fabri

#ifndef CGAL_DISTANCE_3_POINT_3_POINT_3_H
#define CGAL_DISTANCE_3_POINT_3_POINT_3_H

#include <CGAL/Point_3.h>

namespace CGAL {
namespace internal {

template <class K>
inline
typename K::FT
squared_distance(const typename K::Point_3& pt1,
                 const typename K::Point_3& pt2,
                 const K& k)
{
  return k.compute_squared_distance_3_object()(pt1, pt2);
}

} // namespace internal

template <class K>
inline
typename K::FT
squared_distance(const Point_3<K>& pt1,
                 const Point_3<K>& pt2)
{
  return internal::squared_distance(pt1, pt2, K());
}

} // namespace CGAL

#endif // CGAL_DISTANCE_3_POINT_3_POINT_3_H
