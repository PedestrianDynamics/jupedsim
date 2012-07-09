/*---------------------------------------------------------------------------
CTextLog:
=========
Autor: Tim Meyer-K�nig (TraffGo HT)
Beschreibung: Textlog verwaltet Text-Eintr�ge und speichert sie bei Bedarf in eine
Datei. So k�nnen z.B. Fehler mitgelogged werden.

---------------------------------------------------------------------------*/

//---------------------------------------------------------------------------
#ifndef CTextLogH
#define CTextLogH

// Systemheader:
#include <string>
#include <vector>
using namespace std;
#include <stdio.h>
#include <cstdlib>

// Eigene Header:

//---------------------------------------------------------------------------

class CTextLog {
private:
	vector<string> pTextEntries;
//	int pTextEntryCount;
public:
	CTextLog(void); // Constructor
	~CTextLog(void); // Destructor
	void AddEntry(string text); // f�gt einen Eintrag hinzu
  void SaveToFile(string path); // speichert alle Eintr�ge in einer txt-Datei
};

#endif
 