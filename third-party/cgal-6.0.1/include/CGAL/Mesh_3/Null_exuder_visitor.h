// Copyright (c) 2010 INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/v6.0.1/Mesh_3/include/CGAL/Mesh_3/Null_exuder_visitor.h $
// $Id: include/CGAL/Mesh_3/Null_exuder_visitor.h 50cfbde3b84 $
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Stephane Tayeb
//
//******************************************************************************
// File Description :
//******************************************************************************

#ifndef CGAL_MESH_3_NULL_EXUDER_VISITOR_H
#define CGAL_MESH_3_NULL_EXUDER_VISITOR_H

#include <CGAL/license/Mesh_3.h>


#include <cstddef>

namespace CGAL {
namespace Mesh_3 {

template < typename C3T3 >
class Null_exuder_visitor
{
  typedef typename C3T3::Triangulation    Tr;
  typedef typename Tr::Geom_traits::FT    FT;

public:
  void after_cell_pumped(std::size_t) {}
};

} // end namespace Mesh_3
} // end namespace CGAL

#endif // CGAL_MESH_3_NULL_EXUDER_VISITOR_H
