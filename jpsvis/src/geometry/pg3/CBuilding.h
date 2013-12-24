/*---------------------------------------------------------------------------
CGeometry:
==========
Autor: Tim Meyer-Koenig (TraffGo HT)
Beschreibung: Verwaltet die gesamte Geometrie

---------------------------------------------------------------------------*/

//---------------------------------------------------------------------------
#ifndef CBuildingH
#define CBuildingH

// Systemheader:
#include <string>
using namespace std;

// Eigene Header:
#include "CRoom.h"
#include "CTransition.h"
#include "CTextLog.h"

//---------------------------------------------------------------------------

class CBuilding {
private:
// Variablen
	float pCellSize; // Kantenlaenge der Zellen
//	int pRoomCount; // Anzahl der Raeume
//	int pTransitionCount; // Anzahl der uebergaenge (Transitions)
	string pCaption; // Name des Projekts
	vector<CRoom> pRooms; // Liste der Räume
	vector<CTransition> pTransitions; // Liste der Transitions
	CTextLog *pErrorLog; // Fehlerlog, in das man Fehlermeldungen schreiben kann
// Funktionen

public:
  CBuilding(CTextLog *errorlog); // Constructor
  ~CBuilding(void); // Destructor
  void LoadFromFile(string filename); // Laedt Geometrie-Datei
  void SavePotData(string filename); // zum Debuggen die Potenzialdaten der Raeume in "filename" speichern

// Get-Funktionen:
	CRoom GetRoom(int index); // Gibt Raum der Nummer "index" zurueck
//  int GetRoomCount(void) {return pRoomCount;};
	int GetRoomCount(void) {return pRooms.size();};
//  int GetTransitionCount(void) {return pTransitionCount;};
	int GetTransitionCount(void) {return pTransitions.size();};
	string GetCaption(void) {return pCaption;};
// Set-Funktionen:
};

#endif
