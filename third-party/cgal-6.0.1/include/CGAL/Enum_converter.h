// Copyright (c) 2003
// Utrecht University (The Netherlands),
// ETH Zurich (Switzerland),
// INRIA Sophia-Antipolis (France),
// Max-Planck-Institute Saarbruecken (Germany),
// and Tel-Aviv University (Israel).  All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/v6.0.1/Kernel_23/include/CGAL/Enum_converter.h $
// $Id: include/CGAL/Enum_converter.h 50cfbde3b84 $
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Menelaos Karavelas <mkaravel@cse.nd.edu>

#ifndef CGAL_ENUM_CONVERTER_H
#define CGAL_ENUM_CONVERTER_H

#include <CGAL/basic.h>
#include <CGAL/enum.h>

namespace CGAL {

struct Enum_converter
{
  bool              operator()(bool b) const { return b; }

  Sign              operator()(Sign s) const { return s; }

  Bounded_side      operator()(Bounded_side bs) const { return bs; }

  Angle operator()(Angle a) const { return a; }
};


} //namespace CGAL


#endif // CGAL_ENUM_CONVERTER_H
