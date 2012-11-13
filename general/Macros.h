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
#define VERSION 0.40
#define JPS_VERSION "1.1"

// Länge von char vectoren zur Ausgabe
#define CLENGTH 1000


// Faktor für TraVisTo (cm <-> m)
//#define FAKTOR 100
#define FAKTOR 100


// final destinations for the pedestrians
#define FINAL_DEST_OUT -1 //default
//#define FINAL_DEST_PARKING_TOP 1
//#define FINAL_DEST_PARKING_BOTTOM 2
//#define FINAL_DEST_ROOM_010 3
//#define FINAL_DEST_ROOM_020 4
//#define FINAL_DEST_ROOM_030 5
//#define FINAL_DEST_ROOM_040 6
//#define FINAL_DEST_ROOM_050 7
//#define FINAL_DEST_TRAIN 8

//routing
#define EPS_HL_DIST 0.012
#define EPS_AP_DIST EPS_GOAL+EPS_HL_DIST

// Linked cells
#define LIST_EMPTY 	-1

//rooms states
#define ROOM_CLEAN 0
#define ROOM_SMOKED 1


#endif	/* _MACROS_H */

