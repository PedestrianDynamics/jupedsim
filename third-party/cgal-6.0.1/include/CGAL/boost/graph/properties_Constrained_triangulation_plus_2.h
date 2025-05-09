// Copyright (c) 2019  GeometryFactory (France).  All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/v6.0.1/Triangulation_2/include/CGAL/boost/graph/properties_Constrained_triangulation_plus_2.h $
// $Id: include/CGAL/boost/graph/properties_Constrained_triangulation_plus_2.h 50cfbde3b84 $
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Mael Rouxel-Labbé

#ifndef CGAL_PROPERTIES_CONSTRAINED_TRIANGULATION_PLUS_2_H
#define CGAL_PROPERTIES_CONSTRAINED_TRIANGULATION_PLUS_2_H

#include <CGAL/license/Triangulation_2.h>


#include <CGAL/Constrained_triangulation_plus_2.h>

#define CGAL_2D_TRIANGULATION_TEMPLATE_PARAMETERS typename Tr
#define CGAL_2D_TRIANGULATION CGAL::Constrained_triangulation_plus_2<Tr>

#include <CGAL/boost/graph/internal/properties_2D_triangulation.h>

#endif /* CGAL_PROPERTIES_CONSTRAINED_TRIANGULATION_PLUS_2_H */
