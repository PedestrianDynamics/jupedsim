// Copyright (c) 2000
// Utrecht University (The Netherlands),
// ETH Zurich (Switzerland),
// INRIA Sophia-Antipolis (France),
// Max-Planck-Institute Saarbruecken (Germany),
// and Tel-Aviv University (Israel).  All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/v6.0.1/Cartesian_kernel/include/CGAL/Cartesian/line_constructions_2.h $
// $Id: include/CGAL/Cartesian/line_constructions_2.h 50cfbde3b84 $
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Herve Bronnimann

#ifndef CGAL_CARTESIAN_LINE_CONSTRUCTIONS_2_H
#define CGAL_CARTESIAN_LINE_CONSTRUCTIONS_2_H

#include <CGAL/Cartesian/Point_2.h>
#include <CGAL/Cartesian/Line_2.h>

#include <CGAL/Cartesian/Direction_2.h>

namespace CGAL {

template < class K >
inline
LineC2<K>
line_from_point_direction(const PointC2<K> &p,
                          const DirectionC2<K> &d)
{
  return K().construct_line_2_object()(p, d);
}

template < class K >
inline
LineC2<K>
perpendicular_through_point(const LineC2<K> &l,
                            const PointC2<K> &p)
{
  typename K::FT a, b, c;
  perpendicular_through_pointC2(l.a(), l.b(), p.x(), p.y(), a, b, c);
  return LineC2<K>(a, b, c);
}

} //namespace CGAL

#endif // CGAL_CARTESIAN_LINE_CONSTRUCTIONS_2_H
