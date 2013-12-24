#include "CTextLog.h"



CTextLog::CTextLog(void)
// Constructor
{
//  pTextEntries = 0;
//  pTextEntryCount = 0;
}
//---------------------------------------------------------------------------

CTextLog::~CTextLog(void)
// Destructor
{
	if (pTextEntries.size() > 0) pTextEntries.clear();
}
//---------------------------------------------------------------------------

void CTextLog::AddEntry(string text)
// fuegt einen Eintrag hinzu
{
	pTextEntries.push_back(text);
/*
	string *tempvector;
	int i;

	tempvector = pTextEntries; // Temp-Vektor anlegen
	++pTextEntryCount;
	pTextEntries = new string [pTextEntryCount];
	for (i=0; i<pTextEntryCount-1; ++i) // Daten aus Temp-Vektor in neuen Vektor kopieren
		pTextEntries[i] = tempvector[i];
	delete [] tempvector;
	pTextEntries[pTextEntryCount-1] = text; // Eintrag hinten anf�gen
*/
}
//---------------------------------------------------------------------------

void CTextLog::SaveToFile(string path)
// speichert alle Eintr�ge in einer txt-Datei
{
	FILE *logfile;
	int i;

	logfile = fopen(path.c_str(), "wt");

	if(logfile != NULL){
			// Header:
			fprintf(logfile, "Simulator Log-Datei\n");
			fprintf(logfile, "===================\n\n");
			// Inhalt
			fprintf(logfile, "Meldungen:\n");
			fprintf(logfile, "----------\n");
			for (i=0; i<(int)pTextEntries.size(); ++i)
					fprintf(logfile, "%i) %s\n\n",i, pTextEntries[i].c_str());
			fclose(logfile);
	}
	else{
			printf("Can not open file [%s]! (errorlogfile)\n", path.c_str());
			exit(EXIT_FAILURE);
	}
}
//---------------------------------------------------------------------------


