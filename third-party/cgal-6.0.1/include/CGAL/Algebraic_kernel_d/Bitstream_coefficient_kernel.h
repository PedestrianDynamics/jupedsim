// Copyright (c) 2006-2009 Max-Planck-Institute Saarbruecken (Germany).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/v6.0.1/Algebraic_kernel_d/include/CGAL/Algebraic_kernel_d/Bitstream_coefficient_kernel.h $
// $Id: include/CGAL/Algebraic_kernel_d/Bitstream_coefficient_kernel.h 50cfbde3b84 $
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Michael Kerber <mkerber@mpi-inf.mpg.de>
//
// ==========================================================================
#ifndef CGAL_BITSTREAM_COEFFICIENT_KERNEL_H
#define CGAL_BITSTREAM_COEFFICIENT_KERNEL_H 1

#include <CGAL/Algebraic_structure_traits.h>
#include <CGAL/convert_to_bfi.h>

namespace CGAL {

namespace internal {

template <typename Coefficient_> struct Bitstream_coefficient_kernel {

    typedef Coefficient_ Coefficient;

    typedef typename
        CGAL::Get_arithmetic_kernel<Coefficient_>::Arithmetic_kernel
    Arithmetic_kernel;

    typedef typename Arithmetic_kernel::Bigfloat_interval Bigfloat_interval;
    typedef typename Arithmetic_kernel::Integer Integer;
    typedef typename Arithmetic_kernel::Rational Bound;


    typedef typename CGAL::Algebraic_structure_traits<Coefficient>
        ::Is_zero  Is_zero;

    Is_zero is_zero_object() const {
        return Is_zero();
    }

    struct Convert_to_bfi : public CGAL::cpp98::unary_function
        <Coefficient,Bigfloat_interval> {

        Bigfloat_interval operator() (Coefficient c) const {
            return CGAL::convert_to_bfi(c);
        }
    };

    Convert_to_bfi convert_to_bfi_object() const {
        return Convert_to_bfi();
    }


}; // of class Bitstream_coefficient_kernel

} // namespace internal

} //namespace CGAL

#endif // CGAL_BITSTREAM_COEFFICIENT_KERNEL_H
