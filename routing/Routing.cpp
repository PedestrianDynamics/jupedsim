/*
 * File:   Routing.cpp
 * Author: andrea
 *
 * Created on 11. November 2010, 12:55
 */

#include "Routing.h"

/************************************************************
 Routing
 ************************************************************/

Routing::Routing() {
	goals = vector<Crossing* > ();
}

Routing::Routing(const Routing& orig) {
	goals = orig.GetAllGoals();
}

Routing::~Routing() {
	for (int i = 0; i < GetAnzGoals(); i++)
		delete goals[i];
}

// Getter-Funktionen

const vector<Crossing*>& Routing::GetAllGoals() const {
	return goals;
}

Crossing* Routing::GetGoal(int index) const {
	if ((index >= 0) && (index < (int) goals.size()))
		return goals[index];
	else {
		if (index == -1)
			return NULL;
		else {
			char tmp[CLENGTH];
			sprintf(tmp, "ERROR: Wrong 'index' [%d] > [%d] in Routing::GetGoal()",index,goals.size());
			Log->write(tmp);
			exit(0);
		}
	}
}

int Routing::GetAnzGoals() const {
	return goals.size();
}
// Sonstiges

void Routing::AddGoal(Crossing* line) {
	goals.push_back(line);
}

void Routing::LoadCrossings(ifstream* buildingfile, int* i, Room* room) {
	string line;
	getline(*buildingfile, line);
	(*i)++;
	int elements = 0;
	int anz = 0;
	while (line.find("</crossings>") == string::npos) {
		if (line.find("elements") != string::npos) {
			istringstream iss(line, istringstream::in);
			string tmp; // Schlüsselwort
			iss >> tmp >> elements;
		} else if (line.find("<crossing>") != string::npos) {
			LoadCrossing(buildingfile, i, room);
			anz++;
		} else {
			char tmp[CLENGTH];
			sprintf(tmp, "ERROR: \tWrong object in building file <crossing>: [%s] line %d ",
					line.c_str(), *i);
			Log->write(tmp);
			exit(0);
		}
		getline(*buildingfile, line);
		(*i)++;
	}
	if (elements != anz) {
		char tmp[CLENGTH];
		sprintf(tmp, "ERROR: \tWrong number of crossings: %d != %d", elements, anz);
		Log->write(tmp);
		exit(0);
	}
}

void Routing::LoadCrossing(ifstream* buildingfile, int* i, Room* room) {
	string line;
	getline(*buildingfile, line);
	Crossing* c = new Crossing();
	c->SetRoom1(room);
	(*i)++;
	while (line.find("</crossing>") == string::npos) {
		istringstream iss(line, istringstream::in);
		string tmp; // Schlüsselwort
		if (line.find("index") != string::npos) {
			int ID = 88;
			iss >> tmp >> ID;
			c->SetIndex(ID);
		} else if (line.find("line") != string::npos) {
			float x1, y1, x2, y2;
			iss >> tmp >> x1 >> y1 >> x2 >> y2;
			c->SetPoint1(Point(x1, y1));
			c->SetPoint2(Point(x2, y2));
		} else if (line.find("subroom1") != string::npos) {
			int subroom1;
			iss >> tmp >> subroom1;
			SubRoom* subroom = room->GetSubRoom(subroom1);
			subroom->AddGoalID(c->GetIndex());
			c->SetSubRoom1(subroom);
		} else if (line.find("subroom2") != string::npos) {
			int subroom2;
			iss >> tmp >> subroom2;
			SubRoom* subroom = room->GetSubRoom(subroom2);
			subroom->AddGoalID(c->GetIndex());
			c->SetSubRoom2(subroom);
		} else {
			char tmp[CLENGTH];
			sprintf(tmp, "ERROR: \tWrong object in building file <crossings> <crossing>: [%s] line %d ",
					line.c_str(), *i);
			Log->write(tmp);
			exit(0);
		}
		getline(*buildingfile, line);
		(*i)++;
	}
	if (c->GetSubRoom1() == c->GetSubRoom2()) {
		Log->write("ERROR: \t crossing has same subroom IDs!!!");
		exit(0);
	}
	AddGoal(c);

}

void Routing::LoadTransitions(ifstream* buildingfile, int* i, vector<Room*> rooms) {
	string line;
	getline(*buildingfile, line);
	(*i)++;
	int elements = 0;
	int anz = 0;
	while (line.find("</transitions>") == string::npos) {
		if (line.find("elements") != string::npos) {
			istringstream iss(line, istringstream::in);
			string tmp; // Schlüsselwort
			iss >> tmp >> elements;
		} else if (line.find("<transition>") != string::npos) {
			anz++;
			LoadTransition(buildingfile, i, rooms);
		} else {
			char tmp[CLENGTH];
			sprintf(tmp, "ERROR: \tWrong object in building file <transition>: [%s] line %d ",
					line.c_str(), *i);
			Log->write(tmp);
			exit(0);
		}
		getline(*buildingfile, line);
		(*i)++;
	}
	if (elements != anz) {
		char tmp[CLENGTH];
		sprintf(tmp, "ERROR: \tWrong number of transitions: %d != %d", elements, anz);
		Log->write(tmp);
		exit(0);
	}
}

void Routing::LoadTransition(ifstream* buildingfile, int* i, vector<Room*> rooms) {
	//int delme;
	string line;
	int room1, room2, subroom1, subroom2;
	getline(*buildingfile, line);
	Transition* t = new Transition();
	(*i)++;
	while (line.find("</transition>") == string::npos) {
		istringstream iss(line, istringstream::in);
		string tmp; // Schlüsselwort
		if (line.find("index") != string::npos) {
			int ID;
			iss >> tmp >> ID;
			t->SetIndex(ID);
			//delme = ID;
		} else if (line.find("caption") != string::npos) {
			string caption;
			iss >> tmp >> caption;
			t->SetCaption(caption);
		} else if (line.find("trans") != string::npos) {
			double x1, y1, x2, y2;
			iss >> tmp >> x1 >> y1 >> x2 >> y2;
			t->SetPoint1(Point(x1, y1));
			t->SetPoint2(Point(x2, y2));
		} else if (line.find("room1") != string::npos) {
			iss >> tmp >> room1 >> subroom1;
		} else if (line.find("room2") != string::npos) {
			iss >> tmp >> room2 >> subroom2;
		} else {
			char tmp[CLENGTH];
			sprintf(tmp, "ERROR: \tWrong object in building file <transition> <transition>: [%s] line %d ",
					line.c_str(), *i);
			Log->write(tmp);
			exit(0);
		}
		getline(*buildingfile, line);
		(*i)++;
	}

	if (room1 == room2) {
		Log->write("ERROR: \t Building::LoadTransition() Transition has same room IDs!!!");
		exit(0);
	} else {
		// Rooom1, SubRoom1
		if (room1 != -1 && subroom1 != -1) {
			Room* room = rooms[room1];
			SubRoom* subroom = room->GetSubRoom(subroom1);
			subroom->AddGoalID(t->GetIndex());
			//MPI
			room->AddTransitionID(t->GetIndex());
			t->SetRoom1(room);
			t->SetSubRoom1(subroom);
		} else if (room1 == -1 && subroom1 == -1) {
			t->SetRoom1(NULL);
			t->SetSubRoom1(NULL);
		} else {
			Log->write("ERROR: \tRouting::LoadTransition() with room1 or subroom1 == -1");
			exit(0);
		}
		// Rooom2, SubRoom2
		if (room2 != -1 && subroom2 != -1) {
			Room* room = rooms[room2];
			SubRoom* subroom = room->GetSubRoom(subroom2);
			//MPI
			room->AddTransitionID(t->GetIndex());
			subroom->AddGoalID(t->GetIndex());
			t->SetRoom2(room);
			t->SetSubRoom2(subroom);
		} else if (room2 == -1 && subroom2 == -1) {
			t->SetRoom2(NULL);
			t->SetSubRoom2(NULL);
		} else {
			Log->write("ERROR: \tRouting::LoadTransition() with room2 or subroom2 == -1");
			exit(0);
		}

		AddGoal(t);
	}
}

void Routing::LoadOrientationLines(ifstream* buildingfile, int* i, Room* room) {
	string line;
	getline(*buildingfile, line);
	(*i)++;
	int elements = 0;
	int anz = 0;
	while (line.find("</Hlines>") == string::npos) {
		if (line.find("elements") != string::npos) {
			istringstream iss(line, istringstream::in);
			string tmp; // Schlüsselwort
			iss >> tmp >> elements;
		} else if (line.find("<Hline>") != string::npos) {
			LoadOrientationLine(buildingfile, i, room);
			anz++;
		} else {
			char tmp[CLENGTH];
			sprintf(tmp, "ERROR: \tWrong object in building file <Hline>: [%s] line %d ",
					line.c_str(), *i);
			Log->write(tmp);
			exit(0);
		}
		getline(*buildingfile, line);
		(*i)++;
	}
	if (elements != anz) {
		char tmp[CLENGTH];
		sprintf(tmp, "ERROR: \tWrong number of Hline (Orientation lines): %d != %d", elements, anz);
		Log->write(tmp);
		exit(0);
	}
}

void Routing::LoadOrientationLine(ifstream* buildingfile, int* i, Room* room) {
	string line;
	getline(*buildingfile, line);
	Crossing* c = new Crossing();
	c->SetRoom1(room);
	(*i)++;
	while (line.find("</Hline>") == string::npos) {
		istringstream iss(line, istringstream::in);
		string tmp; // Schlüsselwort
		if (line.find("index") != string::npos) {
			int ID = 88;
			iss >> tmp >> ID;
			c->SetIndex(ID);
		} else if (line.find("line") != string::npos) {
			float x1, y1, x2, y2;
			iss >> tmp >> x1 >> y1 >> x2 >> y2;
			c->SetPoint1(Point(x1, y1));
			c->SetPoint2(Point(x2, y2));
		} else if (line.find("subroom") != string::npos) {
			int subroom1;
			iss >> tmp >> subroom1;
			SubRoom* subroom = room->GetSubRoom(subroom1);
			subroom->AddGoalID(c->GetIndex());
			c->SetSubRoom1(subroom);
			c->SetSubRoom2(subroom);
		} else {
			char tmp[CLENGTH];
			sprintf(tmp, "ERROR: \tWrong object in building file <Hlines> <Hline>: [%s] line %d ",
					line.c_str(), *i);
			Log->write(tmp);
			exit(0);
		}
		getline(*buildingfile, line);
		(*i)++;
	}
	AddGoal(c);
}

// Ausgabe

void Routing::WriteToErrorLog() const {
	for (int i = 0; i < GetAnzGoals(); i++)
		goals[i]->WriteToErrorLog();

}



