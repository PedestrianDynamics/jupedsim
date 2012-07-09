/*---------------------------------------------------------------------------
CTransition:
============
Autor: Tim Meyer-Koenig (TraffGo HT)
Beschreibung: Verwaltet die Daten eines Uebergangs zwischen zwei Raeumen, also einer
Tuerschwelle, oder der ersten, bzw. letzten Stufe einer Treppe. Geometrisch werden
die Daten in CRoom verwaltet, doch CTransition enthaelt die Informationen darueber,
welche Raeume verbunden werden.

---------------------------------------------------------------------------*/

//---------------------------------------------------------------------------
#ifndef CTransitionH
#define CTransitionH

// Systemheader:
#include <stdio.h>
#include <string>
using namespace std;

// Eigene Header:
#include "CTextLog.h"
#include "Common.h"
//---------------------------------------------------------------------------

class CTransition {
private:
  int pTransitionIndex; // Uebergangsindex
  int pRoom1;
  int pRoom2; // Indizes der Raeume, die verbunden werden
  string pCaption; // Name, z.B. bei Tueren entsprechend des Raumbuchs
  CTextLog *pErrorLog;

public:
  CTransition(void); // Constructor
  ~CTransition(void); // Destructor
  void Initialize(CTextLog *errorlog, int i); // Initialisieren
  void CopyData(CTransition transition); // Kopiert die Daten von "transition" in sich selbst
  void LoadFromFile(FILE *buildingfile); // Laedt Transitiondaten aus Datei "buildingfile"

// Get-Funktionen:
  int GetTransitionIndex(void) {return pTransitionIndex;}; // Uebergangsindex
  int GetRoom1(void) {return pRoom1;};
  int GetRoom2(void) {return pRoom2;}; // Indizes der Raeume, die verbunden werden
  string GetCaption(void) {return pCaption;}; // Name, z.B. bei Tueren entsprechend des Raumbuchs

// Set-Funktionen:
  void SetTransitionIndex(int index) {pTransitionIndex = index;}; // Uebergangsindex
  void SetRoom1(int room1) {pRoom1 = room1;};
  void SetRoom2(int room2) {pRoom2 = room2;}; // Indizes der Raeume, die verbunden werden
  void SetCaption(string caption) {pCaption = caption;}; // Name, z.B. bei Tueren entsprechend des Raumbuchs
};

#endif
