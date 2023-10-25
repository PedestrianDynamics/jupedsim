// Copyright (c) 2023  GeometryFactory Sarl (France).
// All rights reserved.
//
// $URL: https://github.com/CGAL/cgal/blob/v5.6/STL_Extension/include/CGAL/Base_with_time_stamp.h $
// $Id: Base_with_time_stamp.h 166ff0f 2023-02-14T13:28:07+01:00 Laurent Rineau
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Laurent Rineau

#ifndef CGAL_BASE_WITH_TIME_STAMP_H
#define CGAL_BASE_WITH_TIME_STAMP_H

#include <CGAL/tags.h> // for Tag_true
#include <cstdint>     // for std::size_t
#include <utility>     // for std::forward

namespace CGAL {

template <typename Base>
class Base_with_time_stamp : public Base {
  std::size_t time_stamp_ = -1;
public:
  typedef CGAL::Tag_true Has_timestamp;

  std::size_t time_stamp() const {
    return time_stamp_;
  }
  void set_time_stamp(const std::size_t& ts) {
    time_stamp_ = ts;
  }

  template < class TDS >
  struct Rebind_TDS {
    typedef typename Base::template Rebind_TDS<TDS>::Other Base2;
    typedef Base_with_time_stamp<Base2> Other;
  };
};

} // namespace CGAL

#endif // CGAL_BASE_WITH_TIME_STAMP_H
