// Copyright (c) 2006 Fernando Luis Cacciola Carballal. All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/v6.0.1/Straight_skeleton_2/include/CGAL/predicates/Polygon_offset_pred_ftC2.h $
// $Id: include/CGAL/predicates/Polygon_offset_pred_ftC2.h 50cfbde3b84 $
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Fernando Cacciola <fernando_cacciola@ciudad.com.ar>
//
#ifndef CGAL_POLYGON_OFFSET_PRED_FTC2_H
#define CGAL_POLYGON_OFFSET_PRED_FTC2_H 1

#include <CGAL/license/Straight_skeleton_2.h>

#include <CGAL/constructions/Straight_skeleton_cons_ftC2.h>
#include <CGAL/Straight_skeleton_2/Straight_skeleton_builder_traits_2_aux.h>
#include <CGAL/Uncertain.h>

#include <optional>

namespace CGAL {

namespace CGAL_SS_i {

// Given a triple of oriented straight line segments: (e0,e1,e2) such that
// there exists a distance 'et' for which the offsets lines at 'et' (e0',e1',e2') intersect in a single point;
// returns the relative order of 't' w.r.t. 'et'.
// PRECONDITION: There exists a positive distance et for which the offset triple intersect at a single point.
template<class K>
Uncertain<Comparison_result>
compare_offset_against_isec_timeC2 ( typename K::FT const& t,
                                     Trisegment_2_ptr< Trisegment_2<K, Segment_2_with_ID<K> > > const& tri )
{
  typedef typename K::FT FT ;

  typedef Rational<FT> Rational ;
  typedef Quotient<FT> Quotient ;

  typedef std::optional<Rational> Optional_rational ;

  Uncertain<Comparison_result> rResult = Uncertain<Comparison_result>::indeterminate();

  No_caches<K> no_caches;

  Optional_rational et_ = compute_offset_lines_isec_timeC2(tri, no_caches);
  if ( et_ )
  {
    Quotient et = et_->to_quotient();
    CGAL_assertion ( CGAL_NTS certified_is_positive(et) ) ;

    rResult = CGAL_NTS certified_compare( Quotient(t), et);
  }

  return rResult ;
}

} // namespace CGAL_SS_i

} // end namespace CGAL

#endif // CGAL_POLYGON_OFFSET_PRED_FTC2_H //
// EOF //

