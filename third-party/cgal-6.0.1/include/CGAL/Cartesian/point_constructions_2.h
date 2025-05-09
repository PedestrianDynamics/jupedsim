// Copyright (c) 2000
// Utrecht University (The Netherlands),
// ETH Zurich (Switzerland),
// INRIA Sophia-Antipolis (France),
// Max-Planck-Institute Saarbruecken (Germany),
// and Tel-Aviv University (Israel).  All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/v6.0.1/Cartesian_kernel/include/CGAL/Cartesian/point_constructions_2.h $
// $Id: include/CGAL/Cartesian/point_constructions_2.h 50cfbde3b84 $
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Herve Bronnimann

#ifndef CGAL_CARTESIAN_POINT_CONSTRUCTIONS_2_H
#define CGAL_CARTESIAN_POINT_CONSTRUCTIONS_2_H

#include <CGAL/Cartesian/Point_2.h>
#include <CGAL/Cartesian/Line_2.h>

namespace CGAL {

template < class K >
inline
typename K::Point_2
line_get_point(const LineC2<K> &l, const typename K::FT &i)
{
  typename K::FT x, y;
  line_get_pointC2(l.a(), l.b(), l.c(), i, x, y);
  return PointC2<K>(x, y);
}

template < class K >
inline
typename K::Point_2
line_project_point(const LineC2<K> &l,
                   const PointC2<K> &p)
{
  typename K::FT x, y;
  line_project_pointC2(l.a(), l.b(), l.c(), p.x(), p.y(), x, y);
  return PointC2<K>(x, y);
}

} //namespace CGAL

#endif // CGAL_CARTESIAN_POINT_CONSTRUCTIONS_2_H
