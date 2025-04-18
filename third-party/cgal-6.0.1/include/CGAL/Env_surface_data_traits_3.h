// Copyright (c) 2006  Tel-Aviv University (Israel).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/v6.0.1/Envelope_3/include/CGAL/Env_surface_data_traits_3.h $
// $Id: include/CGAL/Env_surface_data_traits_3.h 50cfbde3b84 $
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s) : Ron Wein          <wein@post.tau.ac.il>
//             Efi Fogel         <efifogel@gmail.com>

#ifndef CGAL_ENV_SURFACE_DATA_TRAITS_3_H
#define CGAL_ENV_SURFACE_DATA_TRAITS_3_H

#include <CGAL/license/Envelope_3.h>


/*! \file
 * Definition of the env_surface_data_traits_3<> class template.
 */

#include <list>

#include <CGAL/Arr_geometry_traits/Curve_data_aux.h>

namespace CGAL {

/*! \class
 * A generic traits class for maintaining the envelope of surfaces that have
 * an extra data field. This traits class is templated with an ordinary traits
 * class, which is also used as a based traits class to inherit from.
 * It can attach data objects to Surface_3 and to Xy_monotone_surface_3 objects
 * (possibly of two different types).
 */
template <typename Traits_, typename XyMonotoneSurfaceData,
          typename SurfaceData = XyMonotoneSurfaceData,
          typename Convert_ = _Default_convert_func<SurfaceData,
                                                    XyMonotoneSurfaceData>>
class Env_surface_data_traits_3 : public Traits_ {
public:
  using Base_traits_3 = Traits_;
  using Xy_monotone_surface_data = XyMonotoneSurfaceData;
  using Surface_data = SurfaceData;
  using Convert = Convert_;

  using Base_surface_3 = typename Base_traits_3::Surface_3;
  using Base_xy_monotone_surface_3 =
    typename Base_traits_3::Xy_monotone_surface_3;

  // Representation of a surface with an additional data field:
  using Surface_3 = _Curve_data_ex<Base_surface_3, Surface_data>;

  // Representation of an xy-monotone surface with an additional data field:
  using Xy_monotone_surface_3 =
    _Curve_data_ex<Base_xy_monotone_surface_3, Xy_monotone_surface_data>;

public:
  /// \name Construction.
  //@{

  /*! Default constructor. */
  Env_surface_data_traits_3() {}

  /*! Constructor from a base-traits class. */
  Env_surface_data_traits_3(const Base_traits_3& traits) :
    Base_traits_3(traits)
  {}
  //@}

  /// \name Overridden functors.
  //@{

  class Make_xy_monotone_3 {
  private:
    const Base_traits_3* base;

  public:
    /*! Constructor. */
    Make_xy_monotone_3(const Base_traits_3* _base) : base (_base) {}

    /*! Subdivide the given surface into xy-monotone surfaces and insert them
     * to the given output iterator.
     * \param S The surface.
     * \param oi The output iterator,
     *           whose value-type is Xy_monotone_surface_2.
     * \return The past-the-end iterator.
     */
    template <typename OutputIterator>
    OutputIterator operator()(const Surface_3& S, bool is_lower,
                              OutputIterator oi) const {
      // Make the original surface xy-monotone.
      std::list<Base_xy_monotone_surface_3> xy_surfaces;
      typename std::list<Base_xy_monotone_surface_3>::iterator xys_it;

      base->make_xy_monotone_3_object()(S, is_lower,
                                        std::back_inserter(xy_surfaces));

      // Attach the data to each of the resulting xy-monotone surfaces.
      for (xys_it = xy_surfaces.begin(); xys_it != xy_surfaces.end(); ++xys_it)
        *oi++ = Xy_monotone_surface_3(*xys_it, Convert() (S.data()));

      return oi;
    }
  };

  /*! Get a Make_xy_monotone_3 functor object. */
  Make_xy_monotone_3 make_xy_monotone_3_object() const
  { return Make_xy_monotone_3(this); }

  //@}

};

} //namespace CGAL

#endif
