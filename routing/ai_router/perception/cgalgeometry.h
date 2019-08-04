#pragma once

#include "geometry/Room.h"

#include <CGAL/Simple_cartesian.h>
#include <CGAL/Boolean_set_operations_2.h>
#include <CGAL/Triangular_expansion_visibility_2.h>
#include <CGAL/Arr_segment_traits_2.h>
#include <CGAL/Arr_naive_point_location.h>
#include <CGAL/Arrangement_2.h>

#include <unordered_map>


class Building;
class Line;

//typedef CGAL::Lazy_exact_nt<CGAL::Quotient<CGAL::MP_Float> > NT;
//CGAL::Simple_cartesian::Point_2  CGAL::Lazy_exact_nt<
//typedef CGAL::Exact_predicates_inexact_constructions_kernel     Kernel;
typedef CGAL::Simple_cartesian<CGAL::Lazy_exact_nt<CGAL::Gmpq>> Kernel;
typedef Kernel::Point_2                                         Point_2;
//Exact_predicates_inexact_constructions_kernel::Point_3
typedef Kernel::Segment_2                                       Segment_2;
typedef CGAL::Polygon_2<Kernel>                                 Polygon_2;
typedef CGAL::Polygon_with_holes_2<Kernel>                      Polygon_with_holes_2;
typedef CGAL::Arr_segment_traits_2<Kernel>                      Traits_2;
typedef CGAL::Arrangement_2<Traits_2>                           Arrangement_2;
typedef CGAL::Triangular_expansion_visibility_2<Arrangement_2>  TEV;

using ptrFloor = const Room*;

class CGALGeometry
{
public:
    explicit CGALGeometry();
    explicit CGALGeometry(const std::unordered_map<ptrFloor, std::vector<const Line *> > &allWalls);

    const Arrangement_2 &GetVisEnvCGAL(ptrFloor floor) const;
    const TEV* GetTEV(ptrFloor floor) const;

private:

    std::unordered_map<ptrFloor,const Arrangement_2> _CGAL_env;
    std::unordered_map<ptrFloor,std::unique_ptr<TEV>> _tev;
};

float roundfloat(float num, int precision);
