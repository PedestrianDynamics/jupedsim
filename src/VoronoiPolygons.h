#include <boost/geometry/geometry.hpp>
#include <boost/geometry/algorithms/intersection.hpp>
#include <boost/geometry/algorithms/intersects.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/segment.hpp>
#include <boost/foreach.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/adapted/c_array.hpp>


using namespace boost::geometry;
using namespace boost::lambda;

typedef model::d2::point_xy<double, cs::cartesian> point_2d;
typedef model::polygon<point_2d> polygon_2d;
//typedef model::segment<point_2d> segment;
typedef model::box<point_2d> box_2d;

#define PI 3.14159265
//BOOST_GEOMETRY_REGISTER_C_ARRAY_CS(cs::cartesian)
   

float angleSubtendedBy(point_2d point,point_2d about);
model::segment<point_2d> forceSegmentClockwise(model::segment<point_2d> s1, point_2d about);
bool segIntersects(model::segment<point_2d> *s1, model::segment<point_2d> s2);
std::vector<polygon_2d>  getVoronoiPolygons(float* xs, float* ys, int Npoints, float xmin, float xmax, float ymin, float ymax);
std::vector<polygon_2d>  cutPolygonsWithGgeometry(std::vector<polygon_2d> polygon, polygon_2d Geometry, float* xs, float* ys);
std::vector<polygon_2d>  cutPolygonsWithSquare(std::vector<polygon_2d> polygon, float* xs, float* ys, float length);
std::vector<polygon_2d>  cutPolygonsWithCircle(std::vector<polygon_2d> polygon, float* xs, float* ys, float radius);






