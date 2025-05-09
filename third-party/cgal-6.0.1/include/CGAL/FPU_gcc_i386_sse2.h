// Copyright (c) 2010-2011  GeometryFactory Sarl (France)
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/v6.0.1/Number_types/include/CGAL/FPU_gcc_i386_sse2.h $
// $Id: include/CGAL/FPU_gcc_i386_sse2.h 50cfbde3b84 $
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Laurent Rineau

extern "C" {
#include <fenv.h>
}

namespace CGAL {

// replacement for C99

inline int
feclearexcept(int exceptions) {
    int mxcsr;
    asm volatile("stmxcsr %0" : "=m" (mxcsr) );
    mxcsr &= ~exceptions;
    asm volatile("ldmxcsr %0" : : "m" (mxcsr) );
    return 0;
}

inline int
fetestexcept(int exceptions) {
    int status;
    asm volatile("stmxcsr %0" : "=m" (status) );
    return status & exceptions;
}

} // end namespace CGAL
