// Copyright (c) 1997-2010  INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/v5.6/Kernel_23/include/CGAL/Projection_traits_xy_3.h $
// $Id: Projection_traits_xy_3.h 404cf8f 2023-01-09T11:13:29+00:00 Andreas Fabri
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Mariette Yvinec

#ifndef CGAL_PROJECTION_TRAITS_XY_3_H
#define CGAL_PROJECTION_TRAITS_XY_3_H

#include <CGAL/Kernel_23/internal/Projection_traits_3.h>
#include <CGAL/Triangulation_structural_filtering_traits.h>

namespace CGAL {

template < class R >
class Projection_traits_xy_3
  : public internal::Projection_traits_3<R,2>
{};

template < class R >
struct Triangulation_structural_filtering_traits<Projection_traits_xy_3<R> > {
  typedef typename Triangulation_structural_filtering_traits<R>::Use_structural_filtering_tag  Use_structural_filtering_tag;
};

} //namespace CGAL

#endif // CGAL_PROJECTION_TRAITS_XY_3_H
