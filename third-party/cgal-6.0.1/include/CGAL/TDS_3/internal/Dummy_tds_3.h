// Copyright (c) 2003  INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/v6.0.1/TDS_3/include/CGAL/TDS_3/internal/Dummy_tds_3.h $
// $Id: include/CGAL/TDS_3/internal/Dummy_tds_3.h 50cfbde3b84 $
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Sylvain Pion

#ifndef CGAL_INTERNAL_TRIANGULATION_DUMMY_TDS_3_H
#define CGAL_INTERNAL_TRIANGULATION_DUMMY_TDS_3_H

#include <CGAL/license/TDS_3.h>


namespace CGAL { namespace internal {

// Dummy TDS which provides all types that a vertex_base or cell_base can use.
struct Dummy_tds_3 {
  struct Concurrency_tag {};

  struct Vertex {};
  struct Cell {};
  struct Facet {};
  struct Edge {};

  struct Vertex_handle {};
  struct Cell_handle {};

  struct Vertex_iterator {};
  struct Cell_iterator {};
  struct Facet_iterator {};
  struct Edge_iterator {};

  struct Cell_circulator {};
  struct Facet_circulator {};
};

}} // namespace CGAL::internal

#endif // CGAL_INTERNAL_TRIANGULATION_DUMMY_TDS_3_H
