// Copyright (c) 2000  Max-Planck-Institute Saarbruecken (Germany).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/v6.0.1/Partition_2/include/CGAL/Partition_2/is_degenerate_polygon_2.h $
// $Id: include/CGAL/Partition_2/is_degenerate_polygon_2.h 50cfbde3b84 $
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Susan Hert <hert@mpi-sb.mpg.de>

#ifndef CGAL_IS_DEGENERATE_POLYGON_2_H
#define CGAL_IS_DEGENERATE_POLYGON_2_H

#include <CGAL/license/Partition_2.h>


namespace CGAL {

// tests if a sequence of points represents a degenerate polygon (i.e.
// one of zero area)
template<class BidirectionalIterator, class Traits>
bool
is_degenerate_polygon_2(BidirectionalIterator first,
                        BidirectionalIterator last,
                        const Traits& traits)
{
   if (first == last) return true;

   BidirectionalIterator prev = last;
   prev--;
   BidirectionalIterator curr = first;
   BidirectionalIterator next = first;
   next++;

   // fewer than three vertices
   if (prev == first) return true;
   if (next == last) return true;

   typedef typename Traits::Orientation_2                Orientation_2;

   Orientation_2 orientation = traits.orientation_2_object();

   while (curr != last)
   {
     if (orientation(*prev, *curr, *next) != COLLINEAR)
        return false;

     prev++;
     if (prev == last)
        prev = first;
     next++;
     if (next == last)
       next = first;
     curr++;
   }
   return true;

}

template<class InputIterator>
bool
is_degenerate_polygon_2(InputIterator first, InputIterator last)
{
   if (first == last) return true;

   typedef typename std::iterator_traits<InputIterator>::value_type Point_2;
   typedef typename Kernel_traits<Point_2>::Kernel  K;
   return is_degenerate_polygon_2(first, last, K());
}

}

#endif // CGAL_IS_DEGENERATE_POLYGON_2_H
