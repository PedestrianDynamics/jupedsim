// Copyright (c) 2004
// Utrecht University (The Netherlands),
// ETH Zurich (Switzerland),
// INRIA Sophia-Antipolis (France),
// Max-Planck-Institute Saarbruecken (Germany),
// and Tel-Aviv University (Israel).  All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/v6.0.1/Kernel_23/include/CGAL/Kernel/mpl.h $
// $Id: include/CGAL/Kernel/mpl.h 50cfbde3b84 $
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Sylvain Pion

#ifndef CGAL_KERNEL_MPL_H
#define CGAL_KERNEL_MPL_H

#include <CGAL/config.h>

// Some tools for basic template metaprogramming.
// These bits should move to CGAL/mpl.h in STL_Extension, or taken from Boost.

namespace CGAL {

// The additional int parameter is to obtain different types.
template < typename A, typename B, int = 0 >
struct First_if_different {
  typedef A Type;
};

template < typename A, int i >
struct First_if_different<A, A, i> {
  struct Type{};
};

} //namespace CGAL

#endif
