#include "geometry_helper.h"

std::string polygon_to_string(const polygon_2d & polygon)
{
     std::string polygon_str = "((";
     for(auto point: boost::geometry::exterior_ring(polygon) )
     {
          double x = boost::geometry::get<0>(point);
          double y = boost::geometry::get<1>(point);
          polygon_str.append("(");
          polygon_str.append(std::to_string(x));
          polygon_str.append(", ");
          polygon_str.append(std::to_string(y));
          polygon_str.append("), ");
     }
     for(auto pRing: boost::geometry::interior_rings(polygon) )
     {
          for(auto point: pRing )
          {
               double x = boost::geometry::get<0>(point);
               double y = boost::geometry::get<1>(point);
               polygon_str.append("(");
               polygon_str.append(std::to_string(x));
               polygon_str.append(", ");
               polygon_str.append(std::to_string(y));
               polygon_str.append("), ");
          }
     }
     polygon_str.pop_back(); polygon_str.pop_back();  //remove last komma
     polygon_str.append("))");
     return polygon_str;
}