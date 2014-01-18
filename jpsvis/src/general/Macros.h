/**
 * File:   Macros.h
 *
 * Created on 16. Juni 2010, 16:59
 *
 * @section LICENSE
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * @section DESCRIPTION
 *
 *
 */

#ifndef _MACROS_H
#define _MACROS_H

#include <cstdlib>
#include <vector>
#include <string.h>

//#define _SIMULATOR 1

#ifdef _NDEBUG
//#undef _OPENMP
#endif

// precision error
#define J_EPS 0.001
#define J_EPS_DIST 0.05// [m]
#define J_EPS_INFO_DIST 2.0 // [m] abstand für Informationsaustausch (GraphRouter)
#define J_EPS_GOAL 0.005 // [m] Abstand zum Ziel, damit Fußgänger immer zu einem Raum gehört
#define J_TOLERANZ 0.03  // [m] Toleranz beim erstellen der Linien
#define J_EPS_V 0.1 // [m/s] wenn  v<EPS_V wird mit 0 gerechnet


#define JPS_VERSION "0.5"
#define JPS_VERSION_MINOR 5
#define JPS_VERSION_MAJOR 0

// Länge von char vectoren zur Ausgabe
#define CLENGTH 1000


// Faktor für TraVisTo (cm <-> m)
#define FAKTOR 100


// final destinations for the pedestrians
#define FINAL_DEST_OUT -1 //default


//routing
#define J_EPS_HL_DIST 0.012
#define J_EPS_AP_DIST J_EPS_GOAL+J_EPS_HL_DIST

// Linked cells
#define LIST_EMPTY 	-1


enum RoomState {
	ROOM_CLEAN=0,
	ROOM_SMOKED=1,
};

enum FileFormat {
	FORMAT_XML_PLAIN,
	FORMAT_XML_BIN,
	FORMAT_PLAIN,
	FORMAT_VTK,
	FORMAT_XML_PLAIN_WITH_MESH
};

enum RoutingStrategy {
	ROUTING_LOCAL_SHORTEST,
	ROUTING_GLOBAL_SHORTEST,
	ROUTING_QUICKEST,
	ROUTING_DYNAMIC,
	ROUTING_FROM_FILE,
	ROUTING_NAV_MESH,
	ROUTING_DUMMY,
	ROUTING_SAFEST,
	ROUTING_UNDEFINED =-1
};


//global functions for convenience

inline char    xmltob(const char * t,char    v=0){ if (t&&(*t)) return (char)atoi(t); return v; }
inline int     xmltoi(const char * t,int     v=0){ if (t&&(*t)) return atoi(t); return v; }
inline long    xmltol(const char * t,long    v=0){ if (t&&(*t)) return atol(t); return v; }
inline double  xmltof(const char * t,double  v=0.0){ if (t&&(*t)) return atof(t); return v; }
inline const char * xmltoa(const char * t,      const char * v=""){ if (t)       return  t; return v; }
inline char xmltoc(const char * t,const char v='\0'){ if (t&&(*t)) return *t; return v; }

#endif	/* _MACROS_H */

