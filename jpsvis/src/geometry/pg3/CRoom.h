/*---------------------------------------------------------------------------
CRoom:
======
Autor: Tim Meyer-Koenig (TraffGo HT)
Beschreibung: Verwaltet die Daten eines Raums, der sowohl kontinuierlich (*CLine)
und zellular (*CCell) repraesentiert wird.

---------------------------------------------------------------------------*/

//---------------------------------------------------------------------------
#ifndef CRoomH
#define CRoomH

// Systemheader:
#include <string>
#include <cstring>
#include <cstdlib>
#include <stdio.h>
using namespace std;

// Eigene Header:
#include "Common.h"
#include "CTextLog.h"
#include "CCell.h"
#include "CLine.h"
#include "CCoordsList.h"

//---------------------------------------------------------------------------

class CRoom {
private:
	int pRoomIndex; // Raumindex
	string pCaption; // Raumname
	CTextLog *pErrorLog; // Log-Klasse fuer Fehlermeldungen

// Geometriedaten:
	int pZPos; // Hoehenindex, in meinen Augen ist hier die Unterscheidung kontunuierlich/zellular egal
	int pXCellPos;
	int pYCellPos; // Raumposition in Zellen
	int pXCellSize;
	int pYCellSize; // Raumabmessung in Zellen
//	int pLineElementCount; // Anzahl der Linienelemente
	int pTransitionCount; // Anzahl der Transitions (=Anzahl der Potenziale)
	vector<int> pTransitionTable; // uebersetzungstabelle pTransitionsTable[0..pTransitionCount-1]=globale Transition-Nummer
	CPoint pContPos; // Raumposition kontinuierlich in Metern
	float pXContSize;
	float pYContSize; // Raumabmessung kontinuierlich in Metern
	vector<CCell> pCells;  // zellulare Repraesentation des Raums [pXCellSize*pYCellSize]
	vector<CLine> pLines;  // kontinuierliche Repraesentation des Raumes [pContElementCount]

/* !Agentendaten habe ich erstmal generell auskommentiert!
// Agentendaten:
	int pTransAgentCount; // Anzahl der Agenten auf Transition-Zellen
	int *pTransAgents;  // Indizes der Agenten auf Trans-Zellen [pTransAgentCount]
	int pAgentCount; // Anzahl der Agenten in diesem Raum
	CAgents *pAgents; // Agenten des Raumes [pAgentCount]
*/
public:
  CRoom(void); // Constructor
  ~CRoom(void); // Destructor
  void Initialize(CTextLog *errorlog, int roomindex); // weist pErrorLog zu
  void EditDoor(int index, bool isopen); // setzt Oeffnungszustand der Tuer "index" auf "isopen"
  void CopyData(CRoom room); // kopiert die Daten von "room" in sich selbst
  void LoadFromFile(FILE *buildingfile); // laedt Raumdaten aus Datei "buildingfile"
  void SpreadPotential(int transition); // breitet das Potenzial von "transition" im Raum aus
  void SavePotsToFile(FILE *potfile); // Speichert Potenziale int "potfile", zum Debuggen
// Get-Funktionen:
  bool GetCell(int x, int y, CCell &cell); // setzt Zeiger "cell" auf die Zelle an Position (x, y)
  bool GetLine(int index, CLine &line);  // setzt Zeiger "line" auf das Linienelement "index"
  CLine GetLine(int index); // gibt Linienelement "index" zurueck
  int GetRoomIndex(void) {return pRoomIndex;}; // Raumindex
  string GetCaption(void) {return pCaption;}; // Raumname
  int GetXCellPos(void) {return pXCellPos;};
  int GetYCellPos(void) {return pYCellPos;}; // Raumposition in Zellen
  int GetZPos(void) {return pZPos;}; // Hoehenindex
  int GetXCellSize(void) {return pXCellSize;};
  int GetYCellSize(void) {return pYCellSize;}; // Raumabmessung in Zellen
//  int GetLineElementCount(void) {return pLineElementCount;}; // Anzahl der Linienelemente
	int GetLineElementCount(void) {return pLines.size();}; // Anzahl der Linienelemente
  int LookPotValue(int transition, int direction, int &x, int &y); // gibt den Wert des Potenzials 'transition' in Richtung 'direction' von (x,y) aus zurueck
  int GetPotValue(int transition, int x, int y); // gibt den Wert des Potenzials 'transition' in Richtung 'direction' von (x,y) aus zurueck
  int GetTransitionCount(void) {return pTransitionCount;}; // gibt Anzahl der Transitions zurueck
  int GetTransitionTableValue(int index); // gibt Wert von Position "index" zurueck
  const CPoint& GetContPos(void) const {return pContPos;}; // Raumposition kontinuierlich in Metern
  float GetXContSize(void) {return pXContSize;};
  float GetYContSize(void) {return pYContSize;}; // Raumabmessung kontinuierlich in Metern

// Set-Funktionen:
/* diese hier sind vielleicht gar nicht noetig:
  void SetRoomIndex(int index) {pRoomIndex = index;}; // Raumindex
  void SetCaption(string caption) {pCaption = caption;}; // Raumname
  void SetXCellPos(int xpos) {pXCellPos = xpos;};
  void SetYCellPos(int ypos) {pYCellPos = ypos;};
  void SetZCellPos(int zpos) {pZCellPos = zpos;}; // Raumposition in Zellen
  void SetXCellSize(int xsize) {pXCellSize = xsize;};
  void SetYCellSize(int ysize) {pYCellSize = ysize;}; // Raumabmessung in Zellen
  void SetContElementCount(int count) {pContElementCount = count;}; // Anzahl der Linienelemente
  void SetZContPos(float zpos) {pZContPos = zpos;}; // Raumposition kontinuierlich in Metern
  void GetXContSize(float xsize) {pXContSize = xsize;};
  void GetYContSize(float ysize) {pYContSize = ysize;}; // Raumabmessung kontinuierlich in Metern
*/
  void SetContPos(const CPoint& p){pContPos=p;}; // setzt die Punktkoordinaten in pContPos
};

#endif
