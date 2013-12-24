#include "CBuilding.h"


CBuilding::CBuilding(CTextLog *errorlog)
// Constructor
{
//  pRoomCount = 0; // Anzahl der Raeume
//  pTransitionCount = 0; // Anzahl der Uebergaenge (Transitions)
	pCaption = "no caption"; // Name des Projekts
//  pRooms = 0;
//  pTransitions = 0;
  pCellSize = 0.0; // Kantenlaenge der Zellen
  pErrorLog = errorlog;
}
//---------------------------------------------------------------------------

CBuilding::~CBuilding(void)
// Destructor
{
//	if (pRooms != NULL) {
//		delete [] pRooms;
//	}
	if (pRooms.size()>0) pRooms.clear();
	if (pTransitions.size()>0) pTransitions.clear();
}
//---------------------------------------------------------------------------

void CBuilding::LoadFromFile(string filename)
// Laedt Geometrie-Datei mit dem Pfad "filename" ein
{
  char line[255];
  FILE *buildingfile = NULL;
  //unused???	int value;
  int fileversion;
  int roomamount;
  int transitionamount=0;

  // Zur Sicherheit die Daten zuruecksetzen:
  /*
    if (pRoomCount > 0) {
    delete [] pRooms;
    pRoomCount = 0;
    }
    if (pTransitionCount > 0) {
    delete [] pTransitions;
    pTransitionCount = 0;
    }
  */
  if (pRooms.size()>0) pRooms.clear();
  if (pTransitions.size()>0) pTransitions.clear();
  pCaption = "no caption";

  // Datei oeffnen
  buildingfile = fopen(filename.c_str(), "rt");
  if (buildingfile != NULL) {
    while (fscanf(buildingfile,"%s",&line[0])!=EOF) {
      // Header laden
      if (!strcmp(line,"<header>")) {
	fscanf(buildingfile,"%s",&line[0]);
	while (strcmp(line,"</header>")) {
	  if (!strcmp(line,"caption")) {  // Caption
	    char value[255];
	    ReadLine(buildingfile, value);
	    string *tempstring = new string(value);
	    pCaption = *tempstring;
	    delete tempstring;
	    pCaption = DelFreeSpaces(pCaption);
	  }
	  else if (!strcmp(line,"cellsize")) // CellSize
	    fscanf(buildingfile,"%f", &pCellSize);
	  else if (!strcmp(line,"version")) // Version
	    fscanf(buildingfile,"%i", &fileversion);
	  else if (!strcmp(line,"rooms")) // Version
	    fscanf(buildingfile,"%i", &roomamount);
	  else
	    pErrorLog->AddEntry("ERROR: Unknown entry in <header> of project file.");
	  // naechsten Wert lesen
	  fscanf(buildingfile,"%s",&line[0]);
	}
	pErrorLog->AddEntry("INFO: Caption = "+pCaption);
	char temp[255];
	sprintf(temp, "INFO: Cell size = %f m", pCellSize);
	pErrorLog->AddEntry(temp);
	if (fileversion != 1) pErrorLog->AddEntry("ERROR: File version != 1");
      }
      // Ende: Header laden

      // Raeume laden
      else if (!strcmp(line,"<rooms>")) {
	fscanf(buildingfile,"%s",&line[0]);
	// Anzahl der Raeume einlesen und Zeiger anlegen
	if (!strcmp(line,"elements")) {
	  fscanf(buildingfile,"%i",&roomamount);
	  //					pRooms = new CRoom [pRoomCount];
	  pRooms.resize(roomamount);
	  fscanf(buildingfile,"%s",&line[0]);
	}
	while (strcmp(line,"</rooms>")) {
	  // Raum
	  if (!strcmp(line,"<room>")) {
	    int roomindex;
	    fscanf(buildingfile,"%s",&line[0]); // naechsten Wert lesen
	    if (!strcmp(line,"index")) {
	      fscanf(buildingfile,"%i", &roomindex);
	      printf("Loading room: %i/%i\n", roomindex, roomamount);
	      pRooms[roomindex].Initialize(pErrorLog, roomindex);
	      pRooms[roomindex].LoadFromFile(buildingfile);
	    } // Ende: Vom Index an Raumdaten einlesen
	  } // Ende: einen Raum laden
	  fscanf(buildingfile,"%s",&line[0]); // naechsten Wert lesen
	}
      } // Ende: alle Raeume laden

      // Transitions laden
      else if (!strcmp(line,"<transitions>")) {
	fscanf(buildingfile,"%s",&line[0]);
	// Anzahl der Transitions einlesen und Zeiger anlegen
	if (!strcmp(line,"elements")) {
	  fscanf(buildingfile,"%i",&transitionamount);
	  //					pTransitions = new CTransition [pTransitionCount];
	  pTransitions.resize(transitionamount);
	  fscanf(buildingfile,"%s",&line[0]);
	}
	while (strcmp(line,"</transitions>")) {
	  // Transition
	  if (!strcmp(line,"<transition>")) {
	    int transitionindex;
	    fscanf(buildingfile,"%s",&line[0]); // naechsten Wert lesen
	    if (!strcmp(line,"index")) {
	      fscanf(buildingfile,"%i", &transitionindex);
	      printf("Loading transition: %i/%i\n", transitionindex, transitionamount-1);
	      pTransitions[transitionindex].Initialize(pErrorLog, transitionindex);
	      pTransitions[transitionindex].LoadFromFile(buildingfile);
	    } // Ende: Vom Index an Transitiondaten einlesen
	  } // Ende: ein Transitiondaten laden
	  fscanf(buildingfile,"%s",&line[0]); // naechsten Wert lesen
	}
      } // Ende: alle Transitiondaten laden
    } // Ende: bis an das Dateiende durch buildingfile lesen
    // Transitions durchgehen und in verbundenen Raeumen und entsprechende Potenziale ausbreiten
    // for (int count=0; count < transitionamount; ++count) {
    //   printf("Spreading transition potential: %i/%i\n", count, transitionamount-1);
    //   pRooms[pTransitions[count].GetRoom1()].SpreadPotential(count);
    //   pRooms[pTransitions[count].GetRoom2()].SpreadPotential(count);
    // }

    // // Fehlerpruefungen
    // // ===============

    // // Hat jede Transition ein Potenzial bekommen? (wenn nein, Pot-Wert == -2)
    // int potvalue = -1;
    // // Alle Raeume durchgehen
    // for (int roomcount=0; roomcount<(int)pRooms.size(); ++roomcount) {
    //   printf("Checking for errors in room %i/%i\n", roomcount, pRooms.size()-1);
    //   // Alle Transitions durchgehen
    //   for (int transcount=0; transcount<pRooms[roomcount].GetTransitionCount(); ++transcount) {
    // 	// Alle Zellen durchsuchen
    //     for (int x=0; x<pRooms[roomcount].GetXCellSize(); ++x) {
    //       for (int y=0; y<pRooms[roomcount].GetYCellSize(); ++y) {
    //         if (pRooms[roomcount].GetPotValue(pRooms[roomcount].GetTransitionTableValue(transcount), x, y)<-1)
    //           potvalue = -2;
    //       }
    //     }
    //   }
    // }
    // if (potvalue==-2)
    //   pErrorLog->AddEntry("ERROR: At least one potential is not spread correctly.");

    fclose(buildingfile);
  } // Ende: buildingfile != NULL
  else {
    pErrorLog->AddEntry("ERROR: Cannot load building file: "+filename);
    printf("Datei konnte nicht geöffnet werden!!!\n");
  }
}
//---------------------------------------------------------------------------

CRoom CBuilding::GetRoom(int index)
// Gibt Raum der Nummer "index" zurueck
{
//  if ((index>=0) && (index<pRoomCount))
  if ((index>=0) && (index<(int)pRooms.size()))
    return pRooms[index];
  else {
    pErrorLog->AddEntry("ERROR: Wrong 'index' in CBuiling::GetRoom()");
    printf("ERROR: Wrong 'index' in CBuiling::GetRoom()");
    exit(0);
//    return ???
  }
}
//---------------------------------------------------------------------------

void CBuilding::SavePotData(string filename)
// zum Debuggen die Potenzialdaten der Raeume in "filename" speichern
{
  FILE *potfile;

  potfile = fopen(filename.c_str(), "wt");
//  for (int roomcount=0; roomcount<pRoomCount; ++roomcount) {
	for (int roomcount=0; roomcount<(int)pRooms.size(); ++roomcount) {
    fprintf(potfile, "Room Nr. %i, %s\n", roomcount, pRooms[roomcount].GetCaption().c_str());
		pRooms[roomcount].SavePotsToFile(potfile);
  }
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------



