// Copyright (c) 2000
// Utrecht University (The Netherlands),
// ETH Zurich (Switzerland),
// INRIA Sophia-Antipolis (France),
// Max-Planck-Institute Saarbruecken (Germany),
// and Tel-Aviv University (Israel).  All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/v6.0.1/Cartesian_kernel/include/CGAL/Cartesian/predicates_on_directions_2.h $
// $Id: include/CGAL/Cartesian/predicates_on_directions_2.h 50cfbde3b84 $
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Herve Bronnimann

#ifndef CGAL_CARTESIAN_PREDICATES_ON_DIRECTIONS_2_H
#define CGAL_CARTESIAN_PREDICATES_ON_DIRECTIONS_2_H

namespace CGAL {

template < class K >
inline
typename K::Boolean
equal_direction(const DirectionC2<K> &d1,
                const DirectionC2<K> &d2)
{
  return equal_directionC2(d1.dx(), d1.dy(), d2.dx(), d2.dy());
}

template < class K >
inline
typename K::Comparison_result
compare_angle_with_x_axis(const DirectionC2<K> &d1,
                          const DirectionC2<K> &d2)
{
  return K().compare_angle_with_x_axis_2_object()(d1, d2);
}

} //namespace CGAL

#endif // CGAL_CARTESIAN_PREDICATES_ON_DIRECTIONS_2_H
