// Copyright (c) 1997-2001
// ETH Zurich (Switzerland).  All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/v6.0.1/Bounding_volumes/include/CGAL/Min_circle_2_traits_2.h $
// $Id: include/CGAL/Min_circle_2_traits_2.h 50cfbde3b84 $
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Sven Schoenherr <sven@inf.ethz.ch>, Bernd Gaertner

#ifndef CGAL_MIN_CIRCLE_2_TRAITS_2_H
#define CGAL_MIN_CIRCLE_2_TRAITS_2_H

#include <CGAL/license/Bounding_volumes.h>


#include <CGAL/Min_circle_2/Optimisation_circle_2.h>
#include <CGAL/predicates_on_points_2.h>

namespace CGAL {

// Class declarations
// ==================
template < class Traits_ >
class Min_circle_2;

template < class K_ >
class Min_circle_2_traits_2;

// Class interface and implementation
// ==================================
template < class K_ >
class Min_circle_2_traits_2 {
  public:
    // types
    typedef  K_                              K;
    typedef  typename K::Point_2             Point;
    typedef  CGAL::Optimisation_circle_2<K>  Circle;
    typedef  typename K::Orientation_2       Orientation_2;

private:
    // data members
    Circle  circle;                                 // current circle

    // friends
    friend  class CGAL::Min_circle_2< CGAL::Min_circle_2_traits_2<K> >;

  public:
    // creation (use default implementations)
    // CGAL::Min_circle_2_traits_2( );
    // CGAL::Min_circle_2_traits_2( CGAL::Min_circle_2_traits_2<K> const&);

    // operations
    inline
    CGAL::Orientation
    orientation( const Point& p, const Point& q, const Point& r) const
    {
      Orientation_2 orientation;
        return(orientation( p, q, r));
    }
};

} //namespace CGAL

#endif // CGAL_MIN_CIRCLE_2_TRAITS_2_H

// ===== EOF ==================================================================
