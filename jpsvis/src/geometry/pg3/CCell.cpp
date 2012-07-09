#include "CCell.h"

CCell::CCell(void)
// Constructor
{
// Werte setzen
  pIsOpen = true; // default: Zellen zugaenglich
  pType = tFloor; // default: Floor
  pAgentID = -1; // leer
//  pPotValue = NULL; // Anzahl der Potenziale, wird spaeter angelegt
  pTransitionID = -1; // default: -1 = kein uebergang
}
//---------------------------------------------------------------------------

CCell::~CCell(void)
// Destructor
{
//  if (pPotValue != NULL)
//		delete [] pPotValue;
	if (pPotValue.size()>0)
		pPotValue.clear();
}
//---------------------------------------------------------------------------

void CCell::Initialize(CTextLog *errorlog)
// Initialisieren der Klasse
{
  pErrorLog = errorlog;                
}
//---------------------------------------------------------------------------

int CCell::GetPotValue(int index)
// Gibt Potenzialwert des Potenzials "index" zurueck
// bei Fehlern wird -1 zurueckgegeben
{
	if (index>=0) { // Plausibilitaetspruefung
		if (pType==tWall)
      return -1;
    else
      return pPotValue[index];
  }
  else {
    pErrorLog->AddEntry("ERROR: Index error in CCell::GetPotValue().");
    return -1;
  }
}
//---------------------------------------------------------------------------

void CCell::SetPotValue(int index, int value)
// Setzt den Potenzialwert "value" fuer Potenzial "index"
{
	if (index>=0) { // Plausibilitaetspruefung
    pPotValue[index] = value; // uebergebenen Wert uebernehmen
  }
  else
    pErrorLog->AddEntry("ERROR: Index error in CCell::SetPotValue().");
}
//---------------------------------------------------------------------------

void CCell::CopyData(CCell cell, int potvectorsize)
// kopiert Daten aus einem Zell-Element in sich selbst
{
  int i;
// unused???  int vectorsize;

  pIsOpen = cell.GetIsOpen();
  pType = cell.GetType();
  pAgentID = cell.GetAgentID();
  pTransitionID = cell.GetTransitionID();
// Potenzial-Zeiger anlegen und Daten uebernehmen
	if (pPotValue.size() > 0) pPotValue.clear();
//	pPotValue = new int [potvectorsize];
	pPotValue.resize(potvectorsize);
	for (i=0; i<potvectorsize; ++i) // Daten uebergeben
    pPotValue[i] = cell.GetPotValue(i);
}
//---------------------------------------------------------------------------

void CCell::IniPotSize(int size)
// legt *pPotValue entsprechend "size" an und faellt sie mit dem Defaultwert "-1"
{
// auch wieder zum Debuggen aufgeblaeht
	if (size>=0) {
		pPotValue.resize(size);
//		pPotValue = new int [size];
		for (int count=0; count<size; ++count)
			pPotValue[count] = -2;
	}
	else
		pErrorLog->AddEntry("ERROR: wrong 'size' in CCell::IniPotSize()");
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
