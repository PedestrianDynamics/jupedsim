#include "CRoom.h"


CRoom::CRoom(void)
// Constructor
{
// Allgemeine Daten:
  pErrorLog = 0;
  pRoomIndex = -1;          // Raumindex
  pCaption = "no caption";  // Raumname
// Geometriedaten:
  pXCellPos = 0;
  pYCellPos = 0;            // Raumposition in Zellen
  pZPos = 0;                // Hoehenindex
  pXCellSize = 0;
  pYCellSize = 0;           // Raumabmessung in Zellen
//  pLineElementCount = 0;    // Anzahl der Linienelemente
  pTransitionCount = 0;     // Anzahl der Transitions
//  pTransitionTable = NULL;
	pContPos = CPoint();  /* Default Konstruktor setzt automatisch (0,0) */
  pXContSize = 0.0;
  pYContSize = 0.0;         // Raumabmessung kontinuierlich in Metern
//  pCells = 0;               // zellulare Repraesentation des Raums [pXCellSize*pYCellSize]
//  pLines = 0;               // kontinuierliche Repraesentation des Raumes [pContElementCount]

}
//---------------------------------------------------------------------------

CRoom::~CRoom(void)
// Destructor
{
//	if (pCells != NULL)
//		delete [] pCells;
	if (pCells.size()>0) pCells.clear();
//	if (pLines != NULL)
//		delete [] pLines;
	if (pLines.size()>0) pLines.clear();
//	if (pTransitionTable != NULL)
//		free(pTransitionTable);
	if (pTransitionTable.size()>0) pTransitionTable.clear();
}
//---------------------------------------------------------------------------

void CRoom::Initialize(CTextLog *errorlog, int roomindex)
// weist pErrorLog zu
{
  pErrorLog = errorlog;
  pRoomIndex = roomindex;
}
//---------------------------------------------------------------------------

bool CRoom::GetCell(int x, int y, CCell &cell)
// setzt Zeiger "cell" auf die Zelle an Position (x, y, z)
{
  bool returnvalue = false;

  if ((x>-1)&&(y>-1)) { // Pruefen ob Koordinaten innerhalb der Haupabmessung liegen
    if ((x<pXCellSize)&&(y<pYCellSize)) {
//      if (pCells != NULL) {
			if (pCells.size()>0) {
				cell = pCells[y*pXCellSize+x];
				returnvalue = true;
      }
      else {
				pErrorLog->AddEntry("ERROR: in CRoom:GetCell(), pCells nicht angelegt");
      }
    }
    else {
      pErrorLog->AddEntry("ERROR: in CRoom:GetCell(), x oder y > Maximalabmessungen");
    }
  }
  else {
    pErrorLog->AddEntry("ERROR: in CRoom:GetCell(), x oder y < 0");
  }
  return returnvalue;
}
//---------------------------------------------------------------------------

bool CRoom::GetLine(int index, CLine &line)
// setzt Zeiger "line" auf das Linienelement "index"
{
  bool returnvalue = false;

  if (index>-1) { // Pruefen ob Index innerhalb der Zeigerlaenge liegt
//    if (index<pLineElementCount) {
		if (index<(int)pLines.size()) {
      line = pLines[index];
      returnvalue = true;
    }
    else {
      pErrorLog->AddEntry("ERROR: in CRoom:GetLine(), index > Zeigerlaenge");
    }
  }
  else {
    pErrorLog->AddEntry("ERROR: in CRoom:GetLine(), index < 0");
  }
  return returnvalue;
}
//---------------------------------------------------------------------------

void CRoom::EditDoor(int index, bool isopen)
// oeffnet Tuer "index"
{
  int i;

// Zellen durchgehen:
//  if (pCells != 0) {
	if (pCells.size() > 0) {
		for (i=0; i<(pXCellSize*pYCellSize); ++i) { // alle Zellen durchgehen
			if (pCells[i].GetTransitionID() == index)
				pCells[i].SetIsOpen(isopen);
		}
  }
  else
    pErrorLog->AddEntry("ERROR: CRoom:OpenDoor(): pCells == 0");

// Linienelemente durchgehen:
//  if (pLines != 0) {
	if (pLines.size() > 0) {
//    for (i=0; i<pLineElementCount; ++i) { // alle Zellen durchgehen
		for (i=0; i<(int)pLines.size(); ++i) { // alle Zellen durchgehen
      if (pLines[i].GetTransitionID() == index)
        pLines[i].SetIsOpen(isopen);
    }
  }
  else
    pErrorLog->AddEntry("ERROR: CRoom:OpenDoor(): pLines == 0");
}
//---------------------------------------------------------------------------

void CRoom::CopyData(CRoom room)
// Kopiert die Daten von "room" in sich selbst
{
	int i, x, y;
	int lineelementcount = room.GetLineElementCount();

	pRoomIndex = room.GetRoomIndex();
	pCaption = room.GetCaption();
	pXCellPos = room.GetXCellPos();
	pYCellPos = room.GetYCellPos();
	pZPos = room.GetZPos();
	pXCellSize = room.GetXCellSize();
	pYCellSize = room.GetYCellSize();

  pTransitionCount = room.GetTransitionCount();
//	if (pTransitionTable!=NULL) free(pTransitionTable);
	if (pTransitionTable.size()>0) pTransitionTable.clear();
//	pTransitionTable = (int*) calloc(pTransitionCount, sizeof(int));
	pTransitionTable.resize(pTransitionCount);
  for (i=0; i<pTransitionCount; ++i)
    pTransitionTable[i] = room.GetTransitionTableValue(i);
  pContPos = room.GetContPos();
  pXContSize = room.GetXContSize();
  pYContSize = room.GetYContSize();
//  if (pCells!=NULL) delete [] pCells;
	if (pCells.size()>0) pCells.clear();
//	pCells = new CCell [pXCellSize*pYCellSize];
	pCells.resize(pXCellSize*pYCellSize);
  for (y=0; y<pYCellSize; ++y)
    for (x=0; x<pXCellSize; ++x) {
      CCell cell;
      pCells[y*pXCellSize+x].Initialize(pErrorLog);
      if (room.GetCell(x, y, cell))
        pCells[y*pXCellSize+x].CopyData(cell, pTransitionCount);
    }
//  if (pLines!=NULL) delete [] pLines;
	if (pLines.size()>0) pLines.clear();
//  pLines = new CLine [pLineElementCount];
	pLines.resize(lineelementcount);
  for (i=0; i<(int)pLines.size(); ++i) {
    CLine line;
    pLines[i].Initialize(pErrorLog);
    if (room.GetLine(i, line))
      pLines[i].CopyData(line);
  }
}
//---------------------------------------------------------------------------

void CRoom::LoadFromFile(FILE *buildingfile)
// Laedt Raumdaten aus Datei "buildingfile"
{
  char line[255];

  fscanf(buildingfile,"%s",&line[0]);
  while (strcmp(line,"</room>")) {
    if (!strcmp(line,"caption")) {  // Raumname
      char value[255];
      ReadLine(buildingfile, value);
      string *tempstring = new string(value);
      pCaption = *tempstring;
      delete tempstring;
      pCaption = DelFreeSpaces(pCaption);
    }
    else if (!strcmp(line,"zpos")) // z-Position
      fscanf(buildingfile,"%i", &pZPos);
// Zelldaten einlesen
    else if (!strcmp(line,"<celldata>")) {
      while (strcmp(line,"</celldata>")) {
        fscanf(buildingfile,"%s",&line[0]);
        if (!strcmp(line,"xpos")) // x-Position
          fscanf(buildingfile,"%i", &pXCellPos);
        else if (!strcmp(line,"ypos")) // y-Position
          fscanf(buildingfile,"%i", &pYCellPos);
        else if (!strcmp(line,"xmax")) // Abmessung in x-Richtung
          fscanf(buildingfile,"%i", &pXCellSize);
        else if (!strcmp(line,"ymax")) // Abmessung in y-Richtung
          fscanf(buildingfile,"%i", &pYCellSize);
        else if (!strcmp(line,"<cells>")) { // Zellen-Zeiger anlegen und Zellinfos einlesen
          int x, y;
          char cellvalue[2];
//          if (pCells != NULL) delete [] pCells;
					if (pCells.size()>0) pCells.clear();
//					pCells = new CCell [pXCellSize*pYCellSize];
					pCells.resize(pXCellSize*pYCellSize);
// Zellen lesen
          for (y=0; y<pYCellSize; ++y) {
            for (x=0; x<pXCellSize; ++x) {
              fscanf(buildingfile,"%1s", cellvalue);
              if (!strcmp(cellvalue, "0")) // begehbarer Boden
                pCells[y*pXCellSize+x].SetType(tFloor);
              else if (!strcmp(cellvalue, "1")) // Wand
                pCells[y*pXCellSize+x].SetType(tWall);
              else if (!strcmp(cellvalue, "2")) // Stufe
                pCells[y*pXCellSize+x].SetType(tStep);
              else if (!strcmp(cellvalue, "x")) // Stufe
                pCells[y*pXCellSize+x].SetType(tOutside);
// Bei Transition die Zahl zwischen den "T" einlesen und speichern
              else if (!strcmp(cellvalue, "T")) {
                string transition;
                int count = 0;
                transition.clear();
                fscanf(buildingfile,"%1s", cellvalue);
                while (strcmp(cellvalue, "T")) {
                  transition += cellvalue[0];
                  ++count;
                  fscanf(buildingfile,"%1s", cellvalue);
                }
                count = atoi(transition.c_str());
                pCells[y*pXCellSize+x].SetType(tTransition);
                pCells[y*pXCellSize+x].SetTransitionID(atoi(transition.c_str()));
              }
              else
                pErrorLog->AddEntry("ERROR: Unknown entry in <celldata> of project file.");
            }
          }
        }
      }
    } // Ende: Zelldaten einlesen
// Kontinuierliche Daten einlesen:
    else if (!strcmp(line,"<contdata>")) {
      float xcontpos = 0.0;
      float ycontpos = 0.0;
      while (strcmp(line,"</contdata>")) {
        fscanf(buildingfile,"%s",&line[0]);
        if (!strcmp(line,"xpos")) // x-Position
          fscanf(buildingfile,"%f", &xcontpos);
        else if (!strcmp(line,"ypos")) // y-Position
          fscanf(buildingfile,"%f", &ycontpos);
				else if (!strcmp(line,"elements")) {// Elementanzahl
					int lineelementcount;
					fscanf(buildingfile,"%i", &lineelementcount);
//          pLines = new CLine [pLineElementCount];
					pLines.resize(lineelementcount);
        }
        else if (!strcmp(line,"<elements>")) { // Zellen-Zeiger anlegen und Zellinfos einlesen
          char type;
          float x1, x2, y1, y2;
          int transition = -1;
//unused???          int count;
          int elementcount = 0;
          while (strcmp(line,"</elements>")) {
            fscanf(buildingfile,"%s",&line[0]);
            if (!strcmp(line,"wall")) { // Wand-Element
              fscanf(buildingfile, "%f %f %f %f", &x1, &y1, &x2, &y2);
              type = tWall;
            }
            else if (!strcmp(line,"step")) { // Stufen-Element
              fscanf(buildingfile, "%f %f %f %f", &x1, &y1, &x2, &y2);
              type = tStep;
            }
            else if (!strcmp(line,"trans")) { // Transition-Element
              fscanf(buildingfile, "%i %f %f %f %f", &transition, &x1, &y1, &x2, &y2);
              type = tTransition;
            }
// Linienelement der Liste hinzufuegen
// if-Abfrage um bei letztem durchlauf Speicherzugriffsfehler zu vermeiden
//            if (elementcount<pLineElementCount) {
						if (elementcount<(int)pLines.size()) {
              pLines[elementcount].Initialize(pErrorLog);
              pLines[elementcount].SetType(type);
              CPoint point1(x1,y1);
              pLines[elementcount].SetPoint1(point1);
              CPoint point2(x2,y2);
              pLines[elementcount].SetPoint2(point2);
              if (transition>=0)
                pLines[elementcount].SetTransitionID(transition);
            }
            ++elementcount;
          }
        } // Ende: <elements>
      }
      SetContPos(CPoint(xcontpos, ycontpos));
    } // Ende: Kontinuierliche Daten einlesen
    fscanf(buildingfile,"%s",&line[0]);
  } // Ende: <room>

// Raum initialisieren
// Alle Zellen durchgehen und Anzahl der Transitions zaehlen, um danach alle Zellen
// zu initialisieren (Zeiger fuer Potenzial anlegen)
  pTransitionCount = 0;
//  if (pTransitionTable != NULL) free(pTransitionTable);
	if (pTransitionTable.size()>0) pTransitionTable.clear();
//	pTransitionTable = (int*) calloc(1, sizeof(int));
	for (int count=0; count<pXCellSize*pYCellSize; ++count) { // alle Zellen durchsuchen
		if (pCells[count].GetType() == tTransition) { // wenn Transition-Zelle
			bool isfound = false;
			for (int trans=0; trans<pTransitionCount; ++trans) { // gucken, ob diese Transition schon detektiert wurde
				if (pTransitionTable[trans] == pCells[count].GetTransitionID())
					isfound = true;
			}
			if (!isfound) { // wenn Transition-Index noch nicht gefunden wurde, merken
				++pTransitionCount;
				pTransitionTable.push_back(pCells[count].GetTransitionID());
//        pTransitionTable = (int*) realloc(pTransitionTable, pTransitionCount*sizeof(int));
//        pTransitionTable[pTransitionCount-1] = pCells[count].GetTransitionID();
      }
    }
  }
  for (int count=0; count<pXCellSize*pYCellSize; ++count) { // alle Zellen initialisieren
    pCells[count].IniPotSize(pTransitionCount);
  }
} // Ende: LoadFromFile()
//---------------------------------------------------------------------------

CLine CRoom::GetLine(int index)
// gibt Linienelement "index" zurueck
{
//  if ((index>=0) && (index<pLineElementCount))
	if ((index>=0) && (index<(int)pLines.size()))
		return pLines[index];
  else {
    pErrorLog->AddEntry("ERROR: Wrong 'index' in CRoom::GetLine()");
    printf("ERROR: Wrong 'index' in CRoom::GetLine()\n");
    exit(0);
	}
}
//---------------------------------------------------------------------------

void CRoom::SpreadPotential(int transition)
// breitet das Potenzial von "transition" im Raum aus
{
	int i, j;
	int x, y;
	int potvalue = 0;
	int transitionindex = -1;
//  CCoordsList *oldcells = new CCoordsList(0, 50); // weiterfuehrende Zellen anlegen
	CCoordsList *oldcells = new CCoordsList; // weiterfuehrende Zellen anlegen
	CCoordsList *newcells;

// "transitionindex" bestimmen, d.h. den Index des Zellenarrays, in dem die Potenzialwerte
// der globalen Transition mit der Nummer "transition" gespeichert werden
	for (i=0; i<pTransitionCount; ++i) {
		if (pTransitionTable[i] == transition)
			transitionindex = i;
	}

// alle Zellen nach Transition-Zellen  von "transition" durchsuchen, in "contcells"
// merken und ihren Potenzialwert auf 0 setzen
	for (y=0; y<pYCellSize; ++y) {
		for (x=0; x<pXCellSize; ++x) {
			if (pCells[y*pXCellSize+x].GetTransitionID() == transition) {
				oldcells->AddAndCheckCoords(x, y);
				pCells[y*pXCellSize+x].SetPotValue(transitionindex, potvalue);
			}
		}
	}
// Potenzial von "contcells" aus ausbreiten
	do {
		++potvalue;
//    newcells = new CCoordsList(0,50);
		newcells = new CCoordsList;
// alle alten Zellen durchgehen und nur ueber die Kanten neue Nachbarn suchen
		for (i=0; i<oldcells->GetCoordsCount(); ++i) {
			for (j=1; j<9; j+=2) {
				oldcells->GetCoords(i, x, y);
// "unentdeckte" Zelle gefunden, also in "newcells" merken und Potenzialwert setzen
				if (LookPotValue(transition, j, x, y)==-2) {
					newcells->AddAndCheckCoords(x, y);
					pCells[y*pXCellSize+x].SetPotValue(transitionindex, potvalue);
				}
			} // Ende 'drumherum ueber Kanten gucken
		} // Ende 'oldcells[]' durchgehen
// Neue Zellen zu alten Zellen umschaufeln
		for (i=0; i<newcells->GetCoordsCount(); ++i) {
			newcells->GetCoords(i, x, y);
			oldcells->AddCoords(x, y);
		}
		newcells->Clear();
		++potvalue;
// alle alten Zellen durchgehen und ueber die Kanten & Ecken (2. Schritt) neue Nachbarn suchen
		for (i=0; i<oldcells->GetCoordsCount(); ++i) {
			for (j=1; j<9; j+=1) {
				oldcells->GetCoords(i, x, y);
// Unentdeckte normale Zelle gefunden
				if (LookPotValue(transition, j, x, y)==-2) {
					newcells->AddCoords(x, y);
					pCells[y*pXCellSize+x].SetPotValue(transitionindex, potvalue);
				} // Ende "unentdeckte Zelle" gefunden
			} // Ende 'drumherum ueber Ecken & Kanten gucken
		} // Ende 'oldcells[]' durchgehen
// Neue Zellen in alte Zellen und nach neuen Nachbarzellen suchen
		delete oldcells;
		oldcells = NULL;
		oldcells = newcells;
		newcells = NULL;
	} while (oldcells->GetCoordsCount() != 0);
	if (oldcells != NULL) {
		delete oldcells;
		oldcells = NULL;
	}
}
//---------------------------------------------------------------------------

int CRoom::LookPotValue(int transition, int direction, int &x, int &y)
// Gibt den Wert des Potenzials 'transition' in Richtung 'direction' von (x,y) aus zurueck
// x, y werden dabei entsprechend geaendert
// Wenn vor Wand oder vor Rand des Rechenfelds, dann wird -1 zurueckgegeben
// Richtung: im Uhrzeigersinn hochgezaehlt, 1 = Zelle in Richtung "oben", also (0,-1)
{
  int transitionindex = -1;

// "transitionindex" bestimmen
  for (int i=0; i<pTransitionCount; ++i) {
    if (pTransitionTable[i] == transition)
      transitionindex = i;
  }
// Rechenrand:
	if ((x==0)&&((direction==6)||(direction==7)||(direction==8)))
		return -1;
	else if ((x==pXCellSize-1)&&((direction==2)||(direction==3)||(direction==4)))
		return -1;
	else if ((y==0)&&((direction==8)||(direction==1)||(direction==2)))
		return -1;
	else if ((y==pYCellSize-1)&&((direction==4)||(direction==5)||(direction==6)))
		return -1;
// im Rechengebiet:
	else {
// jetzige Zelle
		switch (direction) {
			case 1: --y; break;
			case 2: ++x; --y; break;
			case 3: ++x; break;
			case 4: ++x; ++y; break;
			case 5: ++y; break;
			case 6: --x; ++y; break;
			case 7: --x; break;
			case 8: --x; --y; break;
			default: pErrorLog->AddEntry("ERROR: Direction error in CRoom::GetPotValue()");
		}
    return GetPotValue(transition, x, y); // pCells[y*pXCellSize+x].GetPotValue(transitionindex);
	}
}
//---------------------------------------------------------------------------

int CRoom::GetPotValue(int transition, int x, int y)
// Gibt den Wert des Potenzials 'transition' auf (x,y) zurueck
// Wenn Wandzelle, wird -1 zurueckgegeben
{
  int transitionindex = -1;

// "transitionindex" bestimmen
  for (int i=0; i<pTransitionCount; ++i) {
    if (pTransitionTable[i] == transition)
      transitionindex = i;
  }
// ausserhalb des Raumes
	if ((x<0)||(x>pXCellSize)||(y<0)||(y>pYCellSize))
		return -1;
// im Rechengebiet:
	else if ((pCells[y*pXCellSize+x].GetType()==tFloor) || (pCells[y*pXCellSize+x].GetType()==tStep) || (pCells[y*pXCellSize+x].GetType()==tTransition)) {
    return pCells[y*pXCellSize+x].GetPotValue(transitionindex);
	}
  else
    return -1;
}
//---------------------------------------------------------------------------

int CRoom::GetTransitionTableValue(int index)
// gibt Wert von Position "index" zurueck
{
  if ((index>=0) && (index<pTransitionCount))
    return pTransitionTable[index];
  else {
    pErrorLog->AddEntry("ERROR: Wrong Index in CRoom::GetTransitionTableValue()");
    return -1;
  }
}
//---------------------------------------------------------------------------

void CRoom::SavePotsToFile(FILE *potfile)
// Speichert Potenziale int "potfile", zum Debuggen
{
  int x, y;
  for (int transcount=0; transcount<pTransitionCount; ++transcount) {
		fprintf(potfile, "Transition %i:\n", pTransitionTable[transcount]);
    for (y=0; y<pYCellSize; ++y) {
      for (x=0; x<pXCellSize; ++x) {
				fprintf(potfile, "%4i,", pCells[y*pXCellSize+x].GetPotValue(transcount));
      }
      fprintf(potfile, "\n");
    }
    fprintf(potfile, "\n");
  }
  fprintf(potfile, "\n");
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
