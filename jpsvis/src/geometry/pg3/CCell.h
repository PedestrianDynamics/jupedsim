/*---------------------------------------------------------------------------
CCell:
======
Autor: Tim Meyer-K�nig (TraffGo HT)
Beschreibung: Verwaltet die Daten einer Zelle

---------------------------------------------------------------------------*/

//---------------------------------------------------------------------------
#ifndef CCellH
#define CCellH

// Systemheader:
#include <vector>
using namespace std;

// Eigene Header:
#include "Common.h"
#include "CTextLog.h"
//---------------------------------------------------------------------------

class CCell {
private:
	bool pIsOpen; // ist Zelle zug�nglich oder nicht? Genutzt bei Up, Down und Door-Zellen
	char pType; // Eigenschaft
	int pAgentID; // Nummer des Agenten, wenn leer = -1
	vector<int> pPotValue; // Potenzialwert der Zelle, Zeigerlaenge = Anzahl der Potenziale
	int pTransitionID; // Nummer des �bergangs, ansonsten = -1
	CTextLog *pErrorLog;

public:
  CCell(void); // Constructor
  ~CCell(void); // Destructor
// Get-Funktionen:
  bool GetIsOpen(void) {return pIsOpen;};
  char GetType(void) {return pType;};
  int GetAgentID(void) {return pAgentID;};
  int GetPotValue(int index); // Gibt Potenzialwert des Potenzials "index" zur�ck
  int GetTransitionID(void) {return pTransitionID;};
  void Initialize(CTextLog *errorlog); // Initialisieren der Klasse
  void IniPotSize(int size); // legt *pPotValue entsprechend "size" an

// Set-Funktionen:
  void CopyData(CCell cell, int potvectorsize); // kopiert Daten aus einem Zell-Element in sich selbst
  void SetIsOpen(bool isopen) {pIsOpen=isopen;};
  void SetType(char type) {pType=type;};
  void SetAgentID(int agentid) {pAgentID=agentid;};
  void SetPotValue(int index, int value); // Setzt den Potenzialwert "value" f�r Potenzial "index"
  void SetTransitionID(int transitionid) {pTransitionID=transitionid;};
};

#endif
