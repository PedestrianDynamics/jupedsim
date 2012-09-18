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
#define	_MACROS_H

// Genauigkeit
#define EPS 0.001
#define EPS_DIST 0.05// [m]
#define EPS_INFO_DIST 2.0 // [m] abstand für Informationsaustausch (GraphRouter)
#define EPS_GOAL 0.05 // [m] Abstand zum Ziel, damit Fußgänger immer zu einem Raum gehört
#define TOLERANZ 0.03  // [m] Toleranz beim erstellen der Linien
#define EPS_V 0.1 // [m/s] wenn  v<EPS_V wird mit 0 gerechnet
// zur Versionskontrolle beim Geometrieformat
#define VERSION 0.30

#define OPS_VERSION "1.0.13"
// Länge von char vectoren zur Ausgabe
#define CLENGTH 1000


// Faktor für TraVisTo (cm <-> m)
#define FAKTOR 100


// final destinations for the pedestrians
#define FINAL_DEST_OUT -1 //default
#define FINAL_DEST_PARKING_TOP 1
#define FINAL_DEST_PARKING_BOTTOM 2
#define FINAL_DEST_ROOM_010 3
#define FINAL_DEST_ROOM_020 4
#define FINAL_DEST_ROOM_030 5
#define FINAL_DEST_ROOM_040 6
#define FINAL_DEST_ROOM_050 7
#define FINAL_DEST_TRAIN 8

//routing
//#define EPS_HL_DIST 0.0205
#define EPS_HL_DIST 0.012
#define EPS_AP_DIST EPS_GOAL+EPS_HL_DIST

// Linked cells
#define LIST_EMPTY 	-1

//GMS
#define ROOM_CLEAN 0
#define ROOM_SMOKED 1

//Square of the constants
// for saving some square roots computations

#define EPS_2  			EPS*EPS
#define EPS_DIST_2  	EPS_DIST*EPS_DIST
#define EPS_GOAL_2  	EPS_GOAL*EPS_GOAL
#define TOLERANZ_2  	TOLERANZ*TOLERANZ
#define EPS_V_2 		EPS_V*EPS_V
#define EPS_HL_DIST_2 	EPS_HL_DIST*EPS_HL_DIST
#define EPS_AP_DIST_2 	EPS_AP_DIST*EPS_AP_DIST

#endif	/* _MACROS_H */

