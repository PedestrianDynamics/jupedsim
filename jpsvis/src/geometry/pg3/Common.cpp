#include "Common.h"


void ReadLine(FILE *file, char *line)
// Liest die gesamte Zeile aus "file" in "line" ein
{
	do {
	if (fscanf (file,"%[^\r\n]\n",line) == EOF)
			strcpy (line,"END");
	} while ((line[0]) == '#');
}
//---------------------------------------------------------------------------

string DelFreeSpaces(string value)
// Entfernt Leerzeichen am Anfang von "value"
{
//	while (value.IsDelimiter(" ", 1)) {
//		value.Delete(1, 1);
//	}
  while (value.compare(0, 1, " ")==0)
    value.erase(0,1);
	return value;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
