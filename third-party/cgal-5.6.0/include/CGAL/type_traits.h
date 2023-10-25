// Copyright (c) 2007  Max-Planck-Institute Saarbruecken (Germany).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/v5.6/STL_Extension/include/CGAL/type_traits.h $
// $Id: type_traits.h 2e90313 2022-11-09T10:50:01+01:00 Laurent Rineau
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Andreas Meyer

#ifndef CGAL_TYPE_TRAITS_H
#define CGAL_TYPE_TRAITS_H

#include <boost/type_traits/is_base_and_derived.hpp>
#include <boost/mpl/or.hpp>

#include <type_traits>

namespace CGAL {

template< class Base, class Derived >
struct is_same_or_derived :
  public ::boost::mpl::or_<
    ::std::is_same< Base, Derived >,
    ::boost::is_base_and_derived< Base, Derived >
  >::type
{};

namespace cpp20 {

  template< class T >
  struct remove_cvref {
      typedef std::remove_cv_t<std::remove_reference_t<T>> type;
  };

  template< class T >
  using remove_cvref_t = typename remove_cvref<T>::type;

} // end namespace cpp20

} // end namespace CGAL

#endif // CGAL_TYPE_TRAITS_H
