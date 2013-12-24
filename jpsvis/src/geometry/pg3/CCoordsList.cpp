#include "CCoordsList.h"


/*
CCoordsList::CCoordsList(int startsize, int increment)
// Constructor
// startsize = erste Anzahl an Elementen
// increment = Anzahl an Elementen um die die Liste vergroessert wird
{
	pIncrement = increment;
	pElementCount = 0;
	pCoordsArraySize = startsize;
//	pCoordsList = (CoordsListTyp*) calloc(1, sizeof(CoordsListTyp));
}
*/
//---------------------------------------------------------------------------

CCoordsList::CCoordsList()
// Constructor ohne Parameter
{
//	pIncrement = 1;
//	pElementCount = 0;
//	pCoordsArraySize = 0;
//	pCoordsList = NULL;
//	pCoordsList = (CoordsListTyp*) calloc(1, sizeof(CoordsListTyp));
}
//---------------------------------------------------------------------------

CCoordsList::~CCoordsList(void)
// Destructor
{
//	if (pCoordsList!=NULL) {
//		free(pCoordsList);
//		pCoordsList = NULL;
//	}
	if (pCoordsList.size()>0) {
    pCoordsList.clear();
	}
}
//---------------------------------------------------------------------------

void CCoordsList::Clear()
// Loescht die gesamte Elementen-Liste
{
//	free(pCoordsList);
//	pCoordsList = NULL;
	pCoordsList.clear();
//	pElementCount = 0;
//	pCoordsArraySize = 0;
//	pCoordsList = (CoordsListTyp*) calloc(1, sizeof(CoordsListTyp));
}

//---------------------------------------------------------------------------

bool CCoordsList::AddAndCheckCoords(int x, int y)
// (x, y, z) einfuegen, wenn noch nicht enthalten
// Rueckgabe: "false" wenn nocht nicht, "true", wenn wohl schon enthalten.
{
	bool returnvalue = false;

  returnvalue = IsCoordsAdded(x, y);
// wenn (x, y, z) noch nicht enthalten
  if (!returnvalue)
    AddCoords(x, y);
  return returnvalue;
}
//---------------------------------------------------------------------------

void CCoordsList::AddCoordsAt(int i, int x, int y)
// Fuegt Koordinaten (x, y, z) an Position i ein, wenn i>FElementCount, anhaengen
{
	if (i>=0) {
//		if (i<pElementCount) {
		if (i<(int)pCoordsList.size()) {
			pCoordsList[i].x = x;
			pCoordsList[i].y = y;
		}
		else  // wenn i groesser als Anzahl der Elemente -> erweiter
			AddCoords(x, y);
  }
}
//---------------------------------------------------------------------------

bool CCoordsList::IsCoordsAdded(int x, int y)
// Pruefen, ob (x, y, z) schon enthalten ist, wenn ja, true zurueck
{
	bool returnvalue = false;
  int i;

//	for (i=0; i<pElementCount; ++i) {
	for (i=0; i<(int)pCoordsList.size(); ++i) {
		if (x == pCoordsList[i].x)
			if (y == pCoordsList[i].y)
					returnvalue = true;
	}
	return returnvalue;
}

//---------------------------------------------------------------------------
void CCoordsList::GetCoords(int position, int &x, int &y)
// Koordinaten an "position" abfragen
// wenn "position" Bloedsinn, dann (-1, -1, -1) zurueckgeben
{
//	if ((position > -1) && (position < pElementCount)) {
	if ((position > -1) && (position < (int)pCoordsList.size())) {
		x = pCoordsList[position].x;
		y = pCoordsList[position].y;
	}
	else {
		x = -1;
		y = -1;
	}
}

//---------------------------------------------------------------------------
void CCoordsList::AddCoords(int x, int y)
// Einfuegen von Koordinaten (x, y, z) ohne Pruefung
{
//  ++pElementCount;
// einfuegen
//	if (pElementCount > pCoordsArraySize) {
//		pCoordsArraySize += pIncrement;
//		pCoordsList = (CoordsListTyp*) realloc(pCoordsList, pCoordsArraySize*sizeof(CoordsListTyp));
//	}
	CoordsListTyp coords;
	coords.x = x;
	coords.y = y;
	pCoordsList.push_back(coords);
}

//---------------------------------------------------------------------------
void CCoordsList::SaveCoordsList(string filename)
// Liste in ASCII-Datei "filename" speichern (evtl. zum debuggen)
{
	FILE *file;
	int i;

	file = fopen(filename.c_str(), "wt");
	fprintf(file, "Content of CCoordsList:\n");
	for (i=0; i<(int)pCoordsList.size(); ++i) {
		fprintf(file, "%i, %i\n", pCoordsList[pCoordsList.size()-1].x, pCoordsList[pCoordsList.size()-1].y);
	}
	fclose(file);
}

//---------------------------------------------------------------------------
void CCoordsList::SaveCoordsList(FILE *file)
// Liste in ASCII-Datei "filename" speichern (evtl. zum debuggen)
{
	int i;

//	for (i=0; i<pElementCount; ++i) {
	for (i=0; i<(int)pCoordsList.size(); ++i) {
		fprintf(file, "%i, %i\n", pCoordsList[pCoordsList.size()-1].x, pCoordsList[pCoordsList.size()-1].y);
	}
}

//---------------------------------------------------------------------------
void CCoordsList::DeleteCoords(int x, int y)
// Loescht das Element (x, y, z)
{
	int i=0;
	bool isfound = false;

// alle Elemente durchsuchen bis Koordinaten (x, y, z) gefunden sind oder alles durchsucht ist
	do {
		if ((pCoordsList[i].x == x) && (pCoordsList[i].y == y))
			isfound = true;
		++i;
//	} while (!isfound && (i<pElementCount));
	} while (!isfound && (i<(int)pCoordsList.size()));
	--i;
// Entsprechende Koordinaten loeschen, wenn sie vorhanden sind
	if (isfound)
		DeleteElement(i);
}

//---------------------------------------------------------------------------
void CCoordsList::DeleteElement(int i)
// Loescht die Koordinaten an Position i
{
// Letzten Listeneintrag nehmen und auf Position i setzen, Groesse des
// Arrays um 1 reduzieren
//	pCoordsList[i].x = pCoordsList[pElementCount-1].x;
// 	pCoordsList[i].y = pCoordsList[pElementCount-1].y;
//	--pElementCount;
	pCoordsList.erase(pCoordsList.begin()+i);
}
