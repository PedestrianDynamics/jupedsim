// Copyright (c) 2007  GeometryFactory (France).  All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/v6.0.1/HalfedgeDS/include/CGAL/HalfedgeDS_halfedge_max_base_with_id.h $
// $Id: include/CGAL/HalfedgeDS_halfedge_max_base_with_id.h 50cfbde3b84 $
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Andreas Fabri, Fernando Cacciola

#ifndef CGAL_HALFEDGEDS_HALFEDGE_MAX_BASE_WITH_ID_H
#define CGAL_HALFEDGEDS_HALFEDGE_MAX_BASE_WITH_ID_H 1

#include <CGAL/HalfedgeDS_halfedge_base.h>

namespace CGAL {

template < class Refs, class ID>
class HalfedgeDS_halfedge_max_base_with_id : public HalfedgeDS_halfedge_base< Refs, Tag_true, Tag_true, Tag_true >
{
public:
    typedef HalfedgeDS_halfedge_base< Refs, Tag_true, Tag_true, Tag_true> Base ;

    typedef typename Base::Base_base Base_base ;

    typedef ID size_type ;

private:

    size_type mID ;

public:

    HalfedgeDS_halfedge_max_base_with_id( size_type i = size_type(-1) ) : mID(i) {}

    size_type&       id()       { return mID; }
    size_type const& id() const { return mID; }
};

} //namespace CGAL

#endif // CGAL_HALFEDGEDS_HALFEDGE_MAX_BASE_WITH_ID_H //
// EOF //
