/*---------------------------------------------------------------------------
CCoordsList:
============
Autor: Tim Meyer-Koenig (TraffGo HT)
Beschreibung: CCoordsList verwaltet eine Liste zweidimensionaler Koordinaten.
---------------------------------------------------------------------------*/


#ifndef CCoordsListH
#define CCoordsListH

// Systemheader:
#include <string>
#include <stdio.h>
#include <cstdlib>
#include <vector>
using namespace std;

// Eigene Header:

//---------------------------------------------------------------------------

class CCoordsList {
private:
//	int pIncrement; // Anzahl von Eintraegen, um die die Liste vergroessert wird
//	int pElementCount; // Anzahl der Elemente in der Liste
//	int pCoordsArraySize; // Groesse des Arrays (wird ja immer um "FIncrement" erweitert)
	struct CoordsListTyp { // Koordinaten
		int x;							 // kann mal auf beliebig viele Elemente erweitert werden.
		int y;
	};
	vector<CoordsListTyp> pCoordsList;
public:
	CCoordsList(int startsize, int increment); // Constructor 1
  CCoordsList(); // Constructor 2
	~CCoordsList(void); // Destructor
  bool AddAndCheckCoords(int x, int y); // Einfuegen, wenn noch nicht 'drin
  bool IsCoordsAdded(int x, int y); // Prueft, ob (x, y, z) enthalten ist, wenn ja, true zurueck
  void GetCoords(int position, int &x, int &y); // Gibt Koordinaten zurueck
  void AddCoords(int x, int y); // Einfuegen von Koordinaten
  void AddCoordsAt(int i, int x, int y); // ueberschreibt Koordinaten an Stelle "i"
  void SaveCoordsList(string filename); // Speichert Liste in Datei "filename"
  void SaveCoordsList(FILE *file); // // Speichert Liste in Datei "file"
  void DeleteCoords(int x, int y); // Loescht Koordinaten (x, y, z)
	void DeleteElement(int i); // Loescht Element "i"
	void Clear(); // Loescht gesamte Liste

//	int GetIncrement() {return pIncrement;}; // Wert um den Liste verlaengert wird
//  int GetCoordsCount() {return pElementCount;}; // Anzahl Elemente
	int GetCoordsCount() {return pCoordsList.size();}; // Anzahl Elemente
};

#endif
