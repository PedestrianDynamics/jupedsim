// Copyright (c) 2002
// Utrecht University (The Netherlands),
// ETH Zurich (Switzerland),
// INRIA Sophia-Antipolis (France),
// Max-Planck-Institute Saarbruecken (Germany),
// and Tel-Aviv University (Israel).  All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/v6.0.1/Kernel_23/include/CGAL/Is_a_predicate.h $
// $Id: include/CGAL/Is_a_predicate.h 50cfbde3b84 $
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Sylvain Pion

#ifndef CGAL_IS_A_PREDICATE_H
#define CGAL_IS_A_PREDICATE_H

// How to determine if a kernel functor is a predicate or a construction.

#include <CGAL/basic.h>
#include <CGAL/enum.h>

namespace CGAL {

namespace internal {

// By default it's a construction
template <typename Return_type>
struct Return_type_of_predicate {
    typedef CGAL::Tag_false type;
};

// Specializations for predicates
template <>
struct Return_type_of_predicate<bool> {
    typedef CGAL::Tag_true type;
};

template <>
struct Return_type_of_predicate<CGAL::Sign> {
    typedef CGAL::Tag_true type;
};

template <>
struct Return_type_of_predicate<CGAL::Bounded_side> {
    typedef CGAL::Tag_true type;
};

template <>
struct Return_type_of_predicate<CGAL::Angle> {
    typedef CGAL::Tag_true type;
};

} // namespace internal

template <typename Functor>
struct Is_a_predicate {
  typedef typename internal::Return_type_of_predicate<
                   typename Functor::result_type>::type type;
};

} //namespace CGAL

#endif // CGAL_IS_A_PREDICATE_H
