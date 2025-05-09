// Copyright (c) 2023  INRIA (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/v6.0.1/Orthtree/include/CGAL/Orthtree_traits.h $
// $Id: include/CGAL/Orthtree_traits.h 50cfbde3b84 $
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Sven Oesau

#ifndef ORTHTREE_TESTS_ORTHTREE_TRAITS_H
#define ORTHTREE_TESTS_ORTHTREE_TRAITS_H

#include <CGAL/license/Orthtree.h>

#include <CGAL/Dimension.h>
#include <CGAL/Orthtree/Cartesian_ranges.h>

#include <CGAL/Orthtree_traits_base.h>

namespace CGAL {

/*!
  \ingroup PkgOrthtreeTraits

  Traits class for defining an orthtree using the class `CGAL::Orthtree` without storing data in the nodes.

  \tparam GeomTraits model of `Kernel`.
  \tparam dimension the dimension of the ambient Euclidean space.

  \cgalModels{OrthtreeTraits}
  \sa `CGAL::Octree`
  \sa `CGAL::Quadtree`
  \sa `CGAL::Orthtree_traits_base<GeomTraits, DimensionTag>`
*/
template <typename GeomTraits, int dimension>
struct Orthtree_traits : public Orthtree_traits_base<GeomTraits, dimension> {
public:
  using Base = Orthtree_traits_base<GeomTraits, dimension>;
  using Self = Orthtree_traits<GeomTraits, dimension>;
  using Tree = Orthtree<Self>;

  using Node_index = typename Base::Node_index;

  Orthtree_traits() {}

  auto construct_root_node_bbox_object() const {
    return [&]() -> typename Self::Bbox_d {
      return {std::apply(Self::construct_point_d_object(), std::array<typename Self::FT, Self::dimension>{-1.0, -1.0, -1.0}),
              std::apply(Self::construct_point_d_object(), std::array<typename Self::FT, Self::dimension>{1.0, 1.0, 1.0})};
    };
  }
};

}


#endif //ORTHTREE_TESTS_ORTHTREE_TRAITS_H
