// Copyright (c) 2005  Tel-Aviv University (Israel).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/v6.0.1/Boolean_set_operations_2/include/CGAL/Gps_circle_segment_traits_2.h $
// $Id: include/CGAL/Gps_circle_segment_traits_2.h 50cfbde3b84 $
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Baruch Zukerman <baruchzu@post.tau.ac.il>

#ifndef CGAL_GPS_CIRCLE_SEGMENT_TRAITS_2_H
#define CGAL_GPS_CIRCLE_SEGMENT_TRAITS_2_H

#include <CGAL/license/Boolean_set_operations_2.h>

#include <CGAL/disable_warnings.h>

#include <CGAL/Gps_traits_2.h>
#include <CGAL/Arr_circle_segment_traits_2.h>

namespace CGAL {

template <class Kernel_, bool Filer_ = true>
class Gps_circle_segment_traits_2 :
  public Gps_traits_2<Arr_circle_segment_traits_2<Kernel_, Filer_> >
{
public:
  Gps_circle_segment_traits_2(bool use_cache = false) :
    Gps_traits_2<Arr_circle_segment_traits_2<Kernel_, Filer_> >()
  {
    this->m_use_cache = use_cache;
  }

};

} //namespace CGAL

#include <CGAL/enable_warnings.h>

#endif
