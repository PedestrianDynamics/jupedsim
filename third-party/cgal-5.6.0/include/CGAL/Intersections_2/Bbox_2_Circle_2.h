// Copyright (c) 2018 GeometryFactory (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/v5.6/Intersections_2/include/CGAL/Intersections_2/Bbox_2_Circle_2.h $
// $Id: Bbox_2_Circle_2.h 3a4e230 2022-11-22T12:22:42+01:00 Mael Rouxel-Labbé
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Andreas Fabri
//

#ifndef CGAL_INTERSECTIONS_2_BBOX_2_CIRCLE_2_H
#define CGAL_INTERSECTIONS_2_BBOX_2_CIRCLE_2_H

#include <CGAL/Bbox_2.h>
#include <CGAL/Circle_2.h>

#include <CGAL/Intersections_2/Circle_2_Iso_rectangle_2.h>

namespace CGAL {
namespace Intersections {
namespace internal {

template <class K>
typename K::Boolean
do_intersect(const CGAL::Bbox_2& bbox,
             const typename K::Circle_2& circle,
             const K&)
{
  return do_intersect_circle_iso_rectangle_2(circle, bbox, K());
}

template <class K>
typename K::Boolean
do_intersect(const typename K::Circle_2& circle,
             const CGAL::Bbox_2& bbox,
             const K&)
{
  return do_intersect_circle_iso_rectangle_2(circle, bbox, K());
}

} // namespace internal
} // namespace Intersections

template<typename K>
typename K::Boolean
do_intersect(const CGAL::Bbox_2& a,
             const Circle_2<K>& b)
{
  return K().do_intersect_2_object()(a, b);
}

template<typename K>
typename K::Boolean
do_intersect(const Circle_2<K>& a,
             const CGAL::Bbox_2& b)
{
  return K().do_intersect_2_object()(a, b);
}

} // namespace CGAL

#endif // CGAL_INTERSECTIONS_2_BBOX_2_CIRCLE_2_H
