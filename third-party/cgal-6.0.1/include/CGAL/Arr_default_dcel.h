// Copyright (c) 2023 Tel-Aviv University (Israel).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/v6.0.1/Arrangement_on_surface_2/include/CGAL/Arr_default_dcel.h $
// $Id: include/CGAL/Arr_default_dcel.h 50cfbde3b84 $
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s): Efi Fogel       <efifogel@gmail.com>

#ifndef CGAL_ARR_DEFAULT_DCEL_H
#define CGAL_ARR_DEFAULT_DCEL_H

#include <CGAL/license/Arrangement_on_surface_2.h>

#include <CGAL/disable_warnings.h>

/*! \file
 * The definition of the Arr_default_dcel<Traits> type.
 */

#include <CGAL/Arr_dcel.h>

namespace CGAL {

/*! The default arrangement DCEL type.
 * The Traits parameters corresponds to a geometric traits class, which
 * defines the Point_2 and X_monotone_curve_2 types.
 */
template <typename Traits> using Arr_default_dcel = Arr_dcel<Traits>;

} //namespace CGAL

#endif
