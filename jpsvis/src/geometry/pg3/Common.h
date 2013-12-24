/*---------------------------------------------------------------------------
Common:
=======
Autor: Tim Meyer-K�nig (TraffGo HT)
Beschreibung: Enth�lt allgemeine Definitionen und Funktionen

---------------------------------------------------------------------------*/
#ifndef CommonH
#define CommonH
//---------------------------------------------------------------------------

#include <stdio.h>
#include <string>
#include <cstring>
using namespace std;

//---------------------------------------------------------------------------

// Linien- und Zelltypen;
#define tFloor        0
#define tWall         1
#define tStep         2
#define tTransition   3
#define tOutside      4
//---------------------------------------------------------------------------

void ReadLine(FILE *file, char *line); // Liest die gesamte Zeile aus "file" in "line" ein
string DelFreeSpaces(string value); // Entfernt Leerzeichen am Anfang von "value"

#endif
 