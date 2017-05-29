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
#include <limits>


#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// define some colors
#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"
#define RESET "\033[0m"

// should be true only when using this file in the simulation core
#define _SIMULATOR 1
//#define _USE_PROTOCOL_BUFFER 1

#define JPS_OLD_VERSION "0.5" // this version is still supported
#define JPS_VERSION_MINOR "8"
#define JPS_VERSION_MAJOR "0"

#define JPS_VERSION JPS_VERSION_MAJOR "." JPS_VERSION_MINOR

// disable openmp in debug mode
#ifdef _NDEBUG
//#undef _OPENMP
#endif

// precision error
#define J_EPS  0.001//0.001
#define J_EPS_EVENT 0.00001 //zum pruefen des aktuellen Zeitschrittes auf events
#define J_EPS_DIST 0.05// [m]

#define J_EPS_GOAL 0.005 /// [m] Abstand zum Ziel, damit Fußgänger immer zu einem Raum gehört
#define J_TOLERANZ 0.03  /// [m] Toleranz beim erstellen der Linien
#define J_EPS_V 0.1 /// [m/s] wenn  v<EPS_V wird mit 0 gerechnet

// Length of array
#define CLENGTH 1000

// conversion (cm <-> m)
#define FAKTOR 1

// default final destination for the pedestrians
#define FINAL_DEST_OUT -1

// Linked cells
#define LIST_EMPTY  -1

// Not-a-Number (NaN)
#define J_NAN std::numeric_limits<double>::quiet_NaN()

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
     ROUTING_AI,
     ROUTING_FLOORFIELD,
     ROUTING_FF_GLOBAL_SHORTEST,
     ROUTING_FF_LOCAL_SHORTEST,
     ROUTING_FF_QUICKEST,
     ROUTING_UNDEFINED =-1
};

enum OperativModels {
    MODEL_GCFM=1,
    MODEL_GOMPERTZ,
    MODEL_VELOCITY,
    MODEL_GRADIENT,
    MODEL_KRAUSZ
//    MODEL_ORCA,
//    MODEL_CFM,
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

enum LineIntersectType {
	NO_INTERSECTION = 0,
	INTERSECTION,
	OVERLAP // overlap, parallel, no intersection
};

enum FFRouterMode {
     global_shortest = 0,
     local_shortest,
     quickest
};

enum GridCode { //used in floor fields
    WALL = -10,
    INSIDE = -11,
    OUTSIDE = -12,
    //instead of constants, GridCode stores UID of doors
    OPEN_CROSSING = -13,
    OPEN_TRANSITION = -14,
    CLOSED_CROSSING = -15,     //closed crossings and transitions are marked as walls in "parseBuilding"
    CLOSED_TRANSITION = -16
};

enum FastMarchingFlags {
     FM_UNKNOWN = 0,
     FM_SINGLE = 1, // single and double indicate whether a grid point was reached from one or two
     FM_DOUBLE = 2, // directions (rows and columns) when performing the fast marching algorithm
     FM_FINAL = 3,
     FM_ADDED = 4, // added to trial but not calculated
     //FM_BLOCKER = -5, // currently not used
     FM_OUTSIDE = -7
};

enum FloorfieldMode {
     FF_CENTRALPOINT,
     FF_FULL_LINE,
     FF_WALL_AVOID,
     FF_HOMO_SPEED
};

enum MagicNumbers{
     UNKNOWN_DISTANCE,
     UNKNOWN_COST,
     WALL_ON_COSTARRAY,
     TARGET_REGION
};

constexpr double magicnum(int i) {
     return (i == UNKNOWN_DISTANCE) ? -3.0 : (i == UNKNOWN_COST) ? -2.0 : (i == WALL_ON_COSTARRAY) ? -7.0 : (i == TARGET_REGION) ? 0.0 : NAN;
//     switch (i) {
//          case UNKNOWN_DISTANCE:
//               return -3.0;
//          case UNKNOWN_COST:
//               return -2.0;
//          case WALL_ON_COSTARRAY:
//               return -7.0;
//          case TARGET_REGION:
//               return 0.0;
//          default:
//               std::cerr << "ERROR: \tunknown magic number " << i << std::endl;
//               return NAN;
//     }
}

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

inline void ReplaceStringInPlace(std::string& subject, const std::string& search,
                          const std::string& replace) {
    size_t pos = 0;
    while((pos = subject.find(search, pos)) != std::string::npos) {
         subject.replace(pos, search.length(), replace);
         pos += replace.length();
    }
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
