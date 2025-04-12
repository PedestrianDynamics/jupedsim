// Copyright (c) 2007-09  INRIA Sophia-Antipolis (France).
// All rights reserved.
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/v6.0.1/Stream_support/include/CGAL/IO/trace.h $
// $Id: include/CGAL/IO/trace.h 50cfbde3b84 $
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s) : Laurent Saboret

#ifndef CGAL_IO_TRACE_H
#define CGAL_IO_TRACE_H

#include <iostream>

/// \cond SKIP_IN_MANUAL

// Trace utilities
// ---------------


// CGAL_TRACE_STREAM = C++ stream that prints to std::cerr
// if DEBUG_TRACE is defined (ignored otherwise)
#ifdef DEBUG_TRACE
  #define CGAL_TRACE_STREAM  std::cerr
#else
  #define CGAL_TRACE_STREAM  if (false) std::cerr
#endif

/// \endcond

#endif // CGAL_IO_TRACE_H
