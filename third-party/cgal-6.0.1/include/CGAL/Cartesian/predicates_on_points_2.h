// Copyright (c) 2000
// Utrecht University (The Netherlands),
// ETH Zurich (Switzerland),
// INRIA Sophia-Antipolis (France),
// Max-Planck-Institute Saarbruecken (Germany),
// and Tel-Aviv University (Israel).  All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/v6.0.1/Cartesian_kernel/include/CGAL/Cartesian/predicates_on_points_2.h $
// $Id: include/CGAL/Cartesian/predicates_on_points_2.h 50cfbde3b84 $
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Herve Bronnimann

#ifndef CGAL_CARTESIAN_PREDICATES_ON_POINTS_2_H
#define CGAL_CARTESIAN_PREDICATES_ON_POINTS_2_H

#include <CGAL/Cartesian/Point_2.h>
#include <CGAL/predicates/kernel_ftC2.h>

namespace CGAL {

template < class K >
inline
bool
equal_xy(const PointC2<K> &p, const PointC2<K> &q)
{
  return CGAL_AND( p.x() == q.x() , p.y() == q.y() );
}

#if 0
// Unused, undocumented, un-functorized.
template < class K >
inline
Comparison_result
compare_deltax_deltay(const PointC2<K>& p,
                      const PointC2<K>& q,
                      const PointC2<K>& r,
                      const PointC2<K>& s)
{
  return compare_deltax_deltayC2(p.x(), q.x(), r.y(), s.y());
}
#endif

template < class K >
inline
Comparison_result
compare_lexicographically_yx(const PointC2<K> &p,
                             const PointC2<K> &q)
{
  return compare_lexicographically_xyC2(p.y(), p.x(), q.y(), q.x());
}

} //namespace CGAL

#endif // CGAL_CARTESIAN_PREDICATES_ON_POINTS_2_H
