/**
 * \file        Macros.h
 * \date        Jun 16, 2010
 * \version     v0.7
 * \copyright   <2009-2015> Forschungszentrum Jülich GmbH. All rights reserved.
 *
 * \section License
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * \section Description
 * useful constants
 *
 *
 **/


#ifndef _MACROS_H
#define _MACROS_H

#include <cstdlib>
#include <vector>
#include <map>
#include <string.h>
#include <algorithm>
#include <sstream>
#include <iostream>

#include <boost/polygon/voronoi.hpp>
using boost::polygon::voronoi_builder;
using boost::polygon::voronoi_diagram;

#include <boost/geometry.hpp>
#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/adapted/c_array.hpp>
#include <boost/geometry/geometries/ring.hpp>
#include <boost/geometry/algorithms/intersection.hpp>
#include <boost/geometry/algorithms/within.hpp>
#include <boost/foreach.hpp>
//#include "../general/Macros.h"

typedef boost::geometry::model::d2::point_xy<double, boost::geometry::cs::cartesian> point_2d;
typedef boost::geometry::model::polygon<point_2d> polygon_2d;
typedef std::vector<polygon_2d > polygon_list;
typedef boost::geometry::model::segment<boost::geometry::model::d2::point_xy<double> > segment;

typedef boost::polygon::point_data<double> point_type2;
typedef double coordinate_type;
typedef boost::polygon::voronoi_diagram<double> VD;
typedef VD::edge_type edge_type;
typedef VD::cell_type cell_type;
typedef VD::cell_type::source_index_type source_index_type;





#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef VORO_LOCATION
#define VORO_LOCATION "Fundamental_Diagram/Classical_Voronoi/"
#endif

// should be true only when using this file in the simulation core
//#define _SIMULATOR 1
//#define _USE_PROTOCOL_BUFFER 1

#define JPS_OLD_VERSION "0.7" // this version is still supported
#define JPS_VERSION_MINOR "8"
#define JPS_VERSION_MAJOR "0"

#define JPS_VERSION JPS_VERSION_MAJOR "." JPS_VERSION_MINOR

// disable openmp in debug mode
#ifdef _NDEBUG
//#undef _OPENMP
#endif

// precision error
#define J_EPS 0.001
#define J_EPS_EVENT 0.00001 //zum pruefen des aktuellen Zeitschrittes auf events
#define J_EPS_DIST 0.05// [m]

#define J_EPS_GOAL 0.005 /// [m] Abstand zum Ziel, damit Fußgänger immer zu einem Raum gehört
#define J_TOLERANZ 0.03  /// [m] Toleranz beim erstellen der Linien
#define J_EPS_V 0.1 /// [m/s] wenn  v<EPS_V wird mit 0 gerechnet

// routing macros
#define J_QUEUE_VEL_THRESHOLD_NEW_ROOM 0.7 // [m/s] maximum speed to be considered in a queue while looking for a reference in a new room
#define J_QUEUE_VEL_THRESHOLD_JAM 0.2 // [m/s] maximum speed to be considered in a queue while looking for a reference in a jam situation
#define CBA_THRESHOLD 0.15
#define OBSTRUCTION 4

// Length of array
#define CLENGTH 1000

// conversion (cm <-> m)
#define FAKTOR 1

// default final destination for the pedestrians
#define FINAL_DEST_OUT -1

// Linked cells
#define LIST_EMPTY  -1


enum RoomState {
     ROOM_CLEAN=0,
     ROOM_SMOKED=1
};

enum AgentType {
     MALE=0,
     FEMALE,
     CHILD,
     ELDERLY
};

enum FileFormat {
     FORMAT_XML_PLAIN,
     FORMAT_XML_BIN,
     FORMAT_PLAIN,
     FORMAT_VTK,
     FORMAT_XML_PLAIN_WITH_MESH
};

enum RoutingStrategy {
     ROUTING_LOCAL_SHORTEST=1,
     ROUTING_GLOBAL_SHORTEST,
     ROUTING_QUICKEST,
     ROUTING_FROM_FILE,
     ROUTING_NAV_MESH,
     ROUTING_DUMMY,
     ROUTING_SAFEST,
     ROUTING_COGNITIVEMAP,
     ROUTING_UNDEFINED =-1
};

enum OperativModels {
    MODEL_GFCM=1,
    MODEL_GOMPERTZ,
//    MODEL_ORCA,
//    MODEL_CFM,
//    MODEL_VELO
//    MODEL_GNM
};

enum AgentColorMode {
     BY_VELOCITY=1,
     BY_KNOWLEDGE,
     BY_ROUTE,
     BY_ROUTER,
     BY_SPOTLIGHT,
     BY_GROUP,
     BY_FINAL_GOAL,
     BY_INTERMEDIATE_GOAL
};
//global functions for convenience

inline char xmltob(const char * t, char v = 0)
{
     if (t && (*t)) return (char) atoi(t);
     return v;
}
inline int xmltoi(const char * t, int v = 0)
{
     if (t && (*t)) return atoi(t);
     return v;
}
inline long xmltol(const char * t, long v = 0)
{
     if (t && (*t)) return atol(t);
     return v;
}
inline double xmltof(const char * t, double v = 0.0)
{
     if (t && (*t)) return atof(t);
     return v;
}
inline const char * xmltoa(const char * t, const char * v = "")
{
     if (t) return t;
     return v;
}
inline char xmltoc(const char * t, const char v = '\0')
{
     if (t && (*t)) return *t;
     return v;
}

inline std::string polygon_to_string(const polygon_2d & polygon)
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

/**
 * @return true if the element is present in the vector
 */
template<typename A>
inline bool IsElementInVector(const std::vector<A> &vec, const A& el) {
     typename std::vector<A>::const_iterator it;
     it = std::find (vec.begin(), vec.end(), el);
     if(it==vec.end()) {
          return false;
     } else {
          return true;
     }
}

/**
 * Implementation of a map with a default value.
 * @return the default value if the element was not found in the map
 */
template <typename K, typename V>
inline V GetWithDef(const  std::map <K,V> & m, const K & key, const V & defval ) {
     typename std::map<K,V>::const_iterator it = m.find( key );
     if ( it == m.end() ) {
          return defval;
     } else {
          return it->second;
     }
}

inline std::string concatenate(std::string const& name, int i) {
     std::stringstream s;
     s << name << i;
     return s.str();
}

//**************************************************************
//useful colors attributes for debugging
//**************************************************************

//Text attributes
#define OFF       0     //All attributes off
#define BRIGHT      1    //Bold on
//       4    Underscore (on monochrome display adapter only)
#define BLINK       5    //Blink on
//       7    Reverse video on
//      8    Concealed on

//   Foreground colors
#define BLACK     30
#define CYAN      36
#define WHITE     37
#define RED       31
#define GREEN     32
#define YELLOW    33
#define BLUE      34
#define MAGENTA   35

//    Background colors
#define BG_BLACK  40
#define BG_RED    41
#define BG_GREEN  42
#define BG_YELLOW 43
#define BG_BLUE   44
#define BG_CYAN   47
#define BG_WHITE  47

// Special caracters
#define HOME  printf("\033[1;1H");  // cursor up left
#define CLEAR   printf(" \033[2J"); //clear screen
#define RED_LINE  printf("%c[%d;%d;%dm\n",0x1B, BRIGHT,RED,BG_BLACK);
#define GREEN_LINE  printf("\t%c[%d;%d;%dm",0x1B, BRIGHT,GREEN,BG_BLACK);
#define BLUE_LINE  printf("\t%c[%d;%d;%dm",0x1B, BRIGHT,BLUE,BG_BLACK);
#define MAGENTA_LINE  printf("\t%c[%d;%d;%dm",0x1B, BRIGHT,MAGENTA,BG_BLACK);
#define YELLOW_LINE  printf("\t%c[%d;%d;%dm",0x1B, BRIGHT,YELLOW,BG_BLACK);
#define OFF_LINE printf("%c[%dm\n", 0x1B, OFF);


//**************************************************************
//useful macros  for debugging
//**************************************************************
#ifdef TRACE_LOGGING

inline void _printDebugLine(const std::string& fileName, int lineNumber)
{
unsigned found = fileName.find_last_of("/\\");
std::cerr  << "["<< lineNumber  << "]: ---"<< fileName.substr(found+1)<< " ---"<<std::endl;
}

#define dtrace(...)                         \
    (_printDebugLine(__FILE__, __LINE__),   \
    fprintf(stderr, __VA_ARGS__),           \
    (void) fprintf(stderr, "\n"))

#define derror(...)                         \
    (_printDebugLine(__FILE__, __LINE__),   \
    fprintf(stderr, "ERROR: "),             \
    fprintf(stderr, __VA_ARGS__)            \
    )
#else

#define dtrace(...)    ((void) 0)
#define derror(...)                         \
    (fprintf(stderr, __VA_ARGS__)           \
    )
#endif /* TRACE_LOGGING */

#endif  /* _MACROS_H */
