// Copyright (c) 2011  INRIA Saclay Ile-de-France (France).
// All rights reserved.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/v6.0.1/STL_Extension/include/CGAL/is_convertible.h $
// $Id: include/CGAL/is_convertible.h 50cfbde3b84 $
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Marc Glisse


#ifndef CGAL_IS_CONVERTIBLE_H
#define CGAL_IS_CONVERTIBLE_H

#include <type_traits>
#ifdef CGAL_USE_GMPXX
#include <gmpxx.h>
#endif

namespace CGAL {

template<class From,class To>struct is_implicit_convertible
        : std::is_convertible<From,To> {};

#ifdef CGAL_USE_GMPXX
// Work around a gmpxx misfeature
template<class T>struct is_implicit_convertible<__gmp_expr<mpq_t,T>,mpz_class>
        : std::false_type {};
#endif

// TODO: add is_explicit_convertible (using boost::is_constructible?)
}

#endif // CGAL_IS_CONVERTIBLE_H
