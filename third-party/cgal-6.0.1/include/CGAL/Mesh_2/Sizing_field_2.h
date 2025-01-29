// Copyright (c) 2013 INRIA Sophia-Antipolis (France),
//               2014-2015 GeometryFactory (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/v6.0.1/Mesh_2/include/CGAL/Mesh_2/Sizing_field_2.h $
// $Id: include/CGAL/Mesh_2/Sizing_field_2.h 50cfbde3b84 $
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s) : Jane Tournois, Pierre Alliez
//

#ifndef CGAL_SIZING_FIELD_2_H
#define CGAL_SIZING_FIELD_2_H

#include <CGAL/license/Mesh_2.h>


#include <list>

#include <CGAL/basic.h>

namespace CGAL
{

template <typename Tr>
class Sizing_field_2 // pure virtual class
{
public:
  typedef typename Tr::Geom_traits::Point_2 Point_2;
  typedef typename Tr::Geom_traits::FT      FT;

public:
  Sizing_field_2()
  {
  }
  Sizing_field_2(Tr& )
  {
  }

  virtual ~Sizing_field_2()
  {
  }

  virtual FT operator()(const Point_2& p) const = 0;
};

}//namespace CGAL

#endif
