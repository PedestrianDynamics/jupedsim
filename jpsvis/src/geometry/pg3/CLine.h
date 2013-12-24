/*---------------------------------------------------------------------------
CLine:
======
Autor: Tim Meyer-K�nig (TraffGo HT)
Beschreibung: Verwaltet Linienelemente, die durch ihre Eigenschaft und die
Koordinaten des Start- und Endpunkts definiert werden

---------------------------------------------------------------------------*/

#ifndef CLineH
#define CLineH
// Systemheader:

// Eigene Header:
#include "Common.h"
#include "CTextLog.h"
#include "../jul/CPoint.h"

//---------------------------------------------------------------------------

class CLine {
private:
  bool pIsOpen; // Ist Zelle zugänglich oder nicht
  char pType; // Eigenschaft
  CPoint pPoint1; // (Koordinaten des ersten Punkts)
  CPoint pPoint2; // (Koordinaten des zweiten Punkts)
  int pTransitionID; // Nummer des Übergangs, ansonsten "-1"
  CTextLog *pErrorLog;

public:
  CLine(void); // Constructor
  ~CLine(void); // Destructor
  void Initialize(CTextLog *errorlog); // Initialisieren

// Get-Funktionen:
  bool GetIsOpen(void) {return pIsOpen;}; // ist Element zugänglich?
  char GetType(void) {return pType;}; // gibt den Linientyp zurück
  int GetTransitionID(void) {return pTransitionID;}; // gibt den Transition-Index zurück
  const CPoint& GetPoint1(void) const {return pPoint1;}; // gibt die Koordinaten des Anfangspunkts zurück
  const CPoint& GetPoint2(void) const {return pPoint2;}; // gibt die Koordinaten des Endpunkts zurück
  
// Set-Funktionen:
  void CopyData(CLine line); // kopiert Daten aus einem Linien-Element in sich selbst
  void SetIsOpen(bool isopen) {pIsOpen=isopen;}; // ist Element zugänglich?
  void SetType(char type) {pType=type;}; // setzt den Typ
  void SetPoint1(const CPoint& p) {pPoint1=p;}; // setzt die Anfangskoordinaten
  void SetPoint2(const CPoint& p) {pPoint2=p;}; // setzt die Endkoordinaten
  void SetTransitionID(int &transitionid) {pTransitionID=transitionid;}; // setzt den Transition-Index
};


#endif
