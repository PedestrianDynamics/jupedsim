// Copyright (c) 2003,2004,2005,2006  INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/v6.0.1/Segment_Delaunay_graph_2/include/CGAL/Segment_Delaunay_graph_2/Are_same_segments_C2.h $
// $Id: include/CGAL/Segment_Delaunay_graph_2/Are_same_segments_C2.h 50cfbde3b84 $
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Menelaos Karavelas <mkaravel@iacm.forth.gr>


#ifndef CGAL_SEGMENT_DELAUNAY_GRAPH_2_ARE_SAME_SEGMENTS_C2_H
#define CGAL_SEGMENT_DELAUNAY_GRAPH_2_ARE_SAME_SEGMENTS_C2_H

#include <CGAL/license/Segment_Delaunay_graph_2.h>


#include <CGAL/Segment_Delaunay_graph_2/Are_same_points_C2.h>

namespace CGAL {

namespace SegmentDelaunayGraph_2 {

template<class K>
class Are_same_segments_C2
{
private:
  typedef Are_same_points_C2<K>       Are_same_points_2;

private:
  Are_same_points_2  same_points;

public:
  typedef typename K::Site_2    Site_2;
  typedef bool                  result_type;
  typedef Site_2                argument_type;

  bool operator()(const Site_2& p, const Site_2& q) const
  {
    CGAL_precondition( p.is_segment() && q.is_segment() );

    return
      ( same_points(p.source_site(), q.source_site()) &&
        same_points(p.target_site(), q.target_site()) ) ||
      ( same_points(p.source_site(), q.target_site()) &&
        same_points(p.target_site(), q.source_site()) );
  }
};

} //namespace SegmentDelaunayGraph_2

} //namespace CGAL

#endif // CGAL_SEGMENT_DELAUNAY_GRAPH_2_ARE_SAME_SEGMENTS_C2_H
