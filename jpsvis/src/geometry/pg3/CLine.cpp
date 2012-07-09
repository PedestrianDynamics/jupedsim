#include "CLine.h"



CLine::CLine(void)
// Constructor
{
// Werte setzen:
  pIsOpen = false; // Default sind Linienelemente nicht zugänglich
  pType = tWall; // Default: Wand
  pPoint1.SetX(0.0);
  pPoint1.SetY(0.0); // erster Punkt
  pPoint2.SetX(0.0);
  pPoint2.SetY(0.0); // zweiter Punkt
  pTransitionID = -1; // Default: kein �bergang
}

//---------------------------------------------------------------------------

CLine::~CLine(void)
// Destructor
{
}
//---------------------------------------------------------------------------

void CLine::Initialize(CTextLog *errorlog)
// Initialisieren
{
  pErrorLog = errorlog;
}

//---------------------------------------------------------------------------

void CLine::CopyData(CLine line)
// kopiert Daten aus einem Linien-Element in sich selbst
{
// Werte aus "line" kopieren:
  pIsOpen = line.GetIsOpen();
  pType = line.GetType();
  pPoint1 = line.GetPoint1();
  pPoint2 = line.GetPoint2();
  pTransitionID = line.GetTransitionID();
}
//---------------------------------------------------------------------------
