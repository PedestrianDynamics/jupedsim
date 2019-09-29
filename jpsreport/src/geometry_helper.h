#include <string>

#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/adapted/c_array.hpp>
#include <boost/geometry/geometries/ring.hpp>

using namespace boost::geometry;
typedef model::d2::point_xy<double, cs::cartesian> point_2d;
typedef model::polygon<point_2d> polygon_2d;
typedef model::ring<point_2d> ring;
typedef std::vector<polygon_2d > polygon_list;
typedef boost::geometry::model::segment<boost::geometry::model::d2::point_xy<double> > segment;

std::string polygon_to_string(const polygon_2d & polygon);
