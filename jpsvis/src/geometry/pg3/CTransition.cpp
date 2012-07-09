#include "CTransition.h"


CTransition::CTransition(void)
// Constructor
{
  pTransitionIndex = -1; // �bergangsindex
  pRoom1 = -1;
  pRoom2 = -1; // Indizes der R�ume, die verbunden werden
  pCaption = "no caption"; // Name, z.B. bei T�ren entsprechend des Raumbuchs
}
//---------------------------------------------------------------------------

CTransition::~CTransition(void)
// Destructor
{
}
//---------------------------------------------------------------------------

void CTransition::Initialize(CTextLog *errorlog, int i)
// Initialisieren
{
  pErrorLog = errorlog;
  pTransitionIndex = i;
}
//---------------------------------------------------------------------------

void CTransition::CopyData(CTransition transition)
// Kopiert die Daten von "transition" in sich selbst
{
  pTransitionIndex = transition.GetTransitionIndex();
  pRoom1 = transition.GetRoom1();
  pRoom2 = transition.GetRoom2();
  pCaption = transition.GetCaption();
}
//---------------------------------------------------------------------------

void CTransition::LoadFromFile(FILE *buildingfile)
// L�dt Transitiondaten aus Datei "buildingfile"
{
  char line[255];

  fscanf(buildingfile,"%s",&line[0]);
  while (strcmp(line,"</transition>")) {
    if (!strcmp(line,"caption")) {  // Raumname
      char value[255];
      ReadLine(buildingfile, value);
      string *tempstring = new string(value);
      pCaption = *tempstring;
      delete tempstring;
      pCaption = DelFreeSpaces(pCaption);
    }
    else if (!strcmp(line,"room1")) // Raum 1 (auf der einen Seite des Transitionpunkts)
      fscanf(buildingfile,"%i", &pRoom1);
    else if (!strcmp(line,"room2")) // Raum 2 (auf der anderen Seite des Transitionpunkts)
      fscanf(buildingfile,"%i", &pRoom2);
    fscanf(buildingfile,"%s",&line[0]);  // n�chsten Eintrag einlesen
  } // Ende: <transition>
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
