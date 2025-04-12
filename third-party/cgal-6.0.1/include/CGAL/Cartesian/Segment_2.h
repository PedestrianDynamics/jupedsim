// Copyright (c) 2000
// Utrecht University (The Netherlands),
// ETH Zurich (Switzerland),
// INRIA Sophia-Antipolis (France),
// Max-Planck-Institute Saarbruecken (Germany),
// and Tel-Aviv University (Israel).  All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/v6.0.1/Cartesian_kernel/include/CGAL/Cartesian/Segment_2.h $
// $Id: include/CGAL/Cartesian/Segment_2.h 50cfbde3b84 $
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Andreas Fabri, Herve Bronnimann

#ifndef CGAL_CARTESIAN_SEGMENT_2_H
#define CGAL_CARTESIAN_SEGMENT_2_H

#include <CGAL/array.h>
#include <CGAL/Cartesian/predicates_on_points_2.h>

namespace CGAL {
template < class R_ >
class SegmentC2
{
  typedef typename R_::Point_2              Point_2;
  typedef typename R_::Segment_2            Segment_2;

  typedef std::array<Point_2, 2>          Rep;
  typedef typename R_::template Handle<Rep>::type  Base;

  Base base;

public:
  typedef R_                                     R;

  SegmentC2()
  {}

  SegmentC2(const Point_2 &sp, const Point_2 &ep)
    : base{sp, ep}
  {}

  const Point_2 &
  source() const
  {
      return get_pointee_or_identity(base)[0];
  }

  const Point_2 &
  target() const
  {
      return get_pointee_or_identity(base)[1];
  }

};

} //namespace CGAL

#endif // CGAL_CARTESIAN_SEGMENT_2_H
