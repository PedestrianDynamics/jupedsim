/*
 * File:   IODispatcher.cpp
 * Author: andrea
 *
 * Created on 20. November 2010, 15:20
 */

#include <mpi.h>
#include "IODispatcher.h"

// private Funktionen

string IODispatcher::WritePed(Pedestrian* ped) {
	double v, a, b, phi;
	double RAD2DEG = 180.0 / M_PI;
	char tmp[CLENGTH] = "";

	v = ped->GetV().Norm();
	int color;
	double v0 = ped->GetV0Norm();
	if (v0 == 0) {
		Log->write("ERROR: IODispatcher::WritePed()\t v0=0");
		exit(0);
	}
	color = (int) (v / v0 * 255);
	a = ped->GetLargerAxis();
	b = ped->GetSmallerAxis();
	phi = atan2(ped->GetEllipse().GetSinPhi(), ped->GetEllipse().GetCosPhi());
	sprintf(tmp, "<agent ID=\"%d\"\t"
			"xPos=\"%.2f\"\tyPos=\"%.2f\""
			"radiusA=\"%.2f\"\tradiusB=\"%.2f\"\t"
			"ellipseOrientation=\"%.2f\" ellipseColor=\"%d\"/>\n",
			ped->GetPedIndex(), (ped->GetPos().GetX()) * FAKTOR,
			(ped->GetPos().GetY()) * FAKTOR,
			a*FAKTOR, b*FAKTOR, phi * RAD2DEG, color);
	return tmp;
}


// Konstruktoren

IODispatcher::IODispatcher() {
	pHandlers = vector<OutputHandler* > ();
}

IODispatcher::IODispatcher(const IODispatcher& orig) {
}

IODispatcher::~IODispatcher() {
	for (int i = 0; i < (int) pHandlers.size(); i++)
		delete pHandlers[i];
	pHandlers.clear();
}

void IODispatcher::AddIO(OutputHandler* ioh) {
	pHandlers.push_back(ioh);
};

const vector<OutputHandler*>& IODispatcher::GetIOHandlers() {
	return pHandlers;
}

void IODispatcher::Write(string str) {
	for (vector<OutputHandler* >::iterator it = pHandlers.begin(); it != pHandlers.end(); ++it) {
		(*it)->write(str);
	}

};

void IODispatcher::WriteHeader(int nPeds, int fps, Building* building, int seed, int szenarioID) {

	nPeds=building->GetMPIDispatcher()->GetGlobalPedestriansCount();
	string tmp;
	tmp = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\n""<trajectoriesDataset>\n";
	tmp.append("\t<header formatVersion = \"1.0\">\n");
	char agents[50] = "";
	sprintf(agents, "\t\t<agents>%d</agents>\n", nPeds);
	tmp.append(agents);
	sprintf(agents, "\t\t<seed>%d</seed>\n", seed);
	tmp.append(agents);
	sprintf(agents, "\t\t<frameRate>%d</frameRate>\n", fps);
	tmp.append(agents);
	tmp.append("\t</header>\n");
	Write(tmp);

}

void IODispatcher::WriteGeometry(Building* building) {
	string geometry;
	geometry.append("\t<geometry>\n");

	bool plotHlines=false;
	bool plotCrossings=false;
	bool plotTransitions=true;
	bool plotPlayingField=true;
	vector<string> rooms_to_plot;

	//Promenade
	//rooms_to_plot.push_back("outside");
	rooms_to_plot.push_back("010");
	rooms_to_plot.push_back("020");
	rooms_to_plot.push_back("030");
	rooms_to_plot.push_back("040");
	rooms_to_plot.push_back("050");

	//tribuene
//	rooms_to_plot.push_back("060");
//	rooms_to_plot.push_back("070");
//	rooms_to_plot.push_back("080");
//	rooms_to_plot.push_back("090");
//	rooms_to_plot.push_back("100");
//	rooms_to_plot.push_back("110");
//	rooms_to_plot.push_back("120");
//	rooms_to_plot.push_back("130");
//	rooms_to_plot.push_back("140");
	//rooms_to_plot.push_back("150");

	// first the rooms
	for (int i = 0; i < building->GetAnzRooms(); i++) {
		Room* r = building->GetRoom(i);
		string caption=r->GetCaption();
		if(rooms_to_plot.empty()==false)
			if(IsElementInVector(rooms_to_plot,caption)==false) continue;

		for (int k = 0; k < r->GetAnzSubRooms(); k++) {
			SubRoom* s = r->GetSubRoom(k);
			geometry.append(s->WriteSubRoom());
		}
	}

	for (int i = 0; i < building->GetRouting()->GetAnzGoals(); i++) {
		Crossing* goal = building->GetRouting()->GetGoal(i);
		Room* room1 = goal->GetRoom1();
		string caption=room1->GetCaption();

		if(plotCrossings){
			if(goal->IsTransition()==false)
				if(goal->GetSubRoom1()!=goal->GetSubRoom2())
					if(rooms_to_plot.empty() || IsElementInVector(rooms_to_plot,caption)){
						geometry.append(goal->WriteElement());
					}
		}

		if (plotHlines){
			if(goal->IsTransition()==false)
				if(goal->GetSubRoom1()==goal->GetSubRoom2())
					if(rooms_to_plot.empty() || IsElementInVector(rooms_to_plot,caption)){
						geometry.append(goal->WriteElement());
					}
		}

		if (plotTransitions){
			if(goal->IsTransition()){
				if (rooms_to_plot.empty()){
					geometry.append(goal->WriteElement());

				} else {

					Transition* tr = (Transition*)(goal);
					Room* room1 = tr->GetRoom1();
					Room* room2 = tr->GetRoom2();
					string caption1=room1->GetCaption();

					if(room2){
						string caption2=room2->GetCaption();
						if(IsElementInVector(rooms_to_plot,caption1) ||
								IsElementInVector(rooms_to_plot,caption2)){
							geometry.append(goal->WriteElement());
						}

					}else{
						if(IsElementInVector(rooms_to_plot,caption1)){
							geometry.append(goal->WriteElement());
						}
					}
				}
			}
		}
		//if(goal->Length()<1.4) continue;
		//if(goal->Length()>3.4) continue;
		//geometry.append(goal->WriteElement());
	}

	if(plotPlayingField){
		//add the playing area
		double width=3282;
		double length=5668;
		char tmp[100];
		geometry.append("\t\t<wall>\n");
		sprintf(tmp, "\t\t\t<point xPos=\"%.2f\" yPos=\"%.2f\"/>\n",-length,width);
		geometry.append(tmp);
		sprintf(tmp, "\t\t\t<point xPos=\"%.2f\" yPos=\"%.2f\"/>\n",-length,-width);
		geometry.append(tmp);
		sprintf(tmp, "\t\t\t<point xPos=\"%.2f\" yPos=\"%.2f\"/>\n",length,-width);
		geometry.append(tmp);
		sprintf(tmp, "\t\t\t<point xPos=\"%.2f\" yPos=\"%.2f\"/>\n",length,width);
		geometry.append(tmp);
		sprintf(tmp, "\t\t\t<point xPos=\"%.2f\" yPos=\"%.2f\"/>\n",-length,width);
		geometry.append(tmp);
		geometry.append("\t\t</wall>\n");

		geometry.append("\t\t<wall>\n");
		sprintf(tmp, "\t\t\t<point xPos=\"%.2f\" yPos=\"%.2f\"/>\n",0.0,width);
		geometry.append(tmp);
		sprintf(tmp, "\t\t\t<point xPos=\"%.2f\" yPos=\"%.2f\"/>\n",0.0,-width);
		geometry.append(tmp);
		geometry.append("\t\t</wall>\n");
	}

	geometry.append("\t</geometry>\n");
	Write(geometry);
	//exit(0);

	//	//extract some special rooms from the promenade
	//		ofstream myfile ("geometry_pza.txt");
	//		if (myfile.is_open())
	//		{
	//			myfile.precision(2);
	//			for (int i = 0; i < building->GetAnzRooms(); i++) {
	//				Room* r = building->GetRoom(i);
	////				if(AreaLevel(r->GetCaption())==1) continue;
	//				if(r->GetCaption()!="010") continue;
	//				myfile<<"#room "<<r->GetCaption()<<endl;
	//				for (int k = 0; k < r->GetAnzSubRooms(); k++) {
	//					SubRoom* s = r->GetSubRoom(k);
	//					myfile<<"## new subroom"<<endl;
	//					const vector<Point> poly=s->GetPolygon();
	//					for(int i=0;i<poly.size();i++){
	//						myfile<<fixed<<poly[i].GetX()<<" "<<poly[i].GetY()<<endl;
	//					}
	//
	//				}
	//			}
	//
	//			myfile.close();exit(0);
	//		}

	//extract the transitions for the analysis
	//			ofstream myfile ("geometry_pza_doors.txt");
	//			if (myfile.is_open())
	//			{
	//				myfile.precision(2);
	//				for (int i = 0; i < building->GetRouting()->GetAnzGoals(); i++) {
	//					Crossing* goal = building->GetRouting()->GetGoal(i);
	//					if(goal->GetCaption()=="") continue;
	//					Room* r1= goal->GetRoom1();
	//					Room* r2 = ((Transition*)goal)->GetRoom2();
	//
	//
	//					if((r2)&&(AreaLevel(r1->GetCaption())==1) && (AreaLevel(r2->GetCaption())==1)) continue;
	//					else if((!r2)&&(AreaLevel(r1->GetCaption())==1)) continue;
	//
	//					if(goal->GetRoom1()->GetCaption()=="150") continue;
	//
	//					myfile<<fixed<<"#camera "<<goal->GetCaption()<<endl;
	//					myfile<<goal->GetPoint1().GetX()<<" "<<goal->GetPoint1().GetY()<<" "
	//							<<goal->GetPoint2().GetX()<<" "<<goal->GetPoint2().GetY()<<endl;
	//
	//					//myfile<<goal->GetCentre().GetX() <<" "<<goal->GetCentre().GetY() <<endl;
	//
	//				}
	//			}
	//			exit(0);

	//	//extract all transitions per room
	//		ofstream myfile ("geometry_pza_trans.txt");
	//		if (myfile.is_open())
	//		{
	//			myfile.precision(2);
	//			for (int i = 0; i < building->GetAnzRooms(); i++) {
	//				Room* r = building->GetRoom(i);
	//				if(AreaLevel(r->GetCaption())==1) continue;
	//				if(r->GetCaption()=="150") continue;
	//				myfile<<"#room "<<r->GetCaption()<<endl;
	//
	//				vector<int>goals=r->GetAllTransitionsIDs();
	//				for(int g=0;g<goals.size();g++){
	//					Crossing* goal = building->GetRouting()->GetGoal(goals[g]);
	//					myfile<<goal->GetCaption()<<endl;
	//				}
	//			}
	//
	//			myfile.close();
	//		}

//	//plot the mapping Room+Transition-->room
//	ofstream myfile ("mapping_door_exit_room.txt");
//	if (myfile.is_open())
//	{
//		myfile.precision(2);
//		myfile<<"#room transition nextroom";
//		for (int i = 0; i < building->GetRouting()->GetAnzGoals(); i++) {
//			Crossing* goal = building->GetRouting()->GetGoal(i);
//			if(goal->IsTransition()){
//
//				Room* r1= goal->GetRoom1();
//				Room* r2 = ((Transition*)goal)->GetRoom2();
//
//				if(r2){
//					myfile<<r1->GetCaption()<<" "<<goal->GetCaption()<<" "<<r2->GetCaption()<<endl;
//					myfile<<r2->GetCaption()<<" "<<goal->GetCaption()<<" "<<r1->GetCaption()<<endl;
//
//				}else{
//					myfile<<r1->GetCaption()<<" "<<goal->GetCaption()<<" outside "<<endl;
//				}
//
//			}
//		}
//		exit(0);
//	}
}

void IODispatcher::WriteFrame(int frameNr, Building* building) {
	string data;
	char tmp[CLENGTH] = "";
	vector<string> rooms_to_plot;

	//promenade
	rooms_to_plot.push_back("010");
	rooms_to_plot.push_back("020");
	rooms_to_plot.push_back("030");
	rooms_to_plot.push_back("040");
	rooms_to_plot.push_back("050");

	//tribuene
	//rooms_to_plot.push_back("060");
	//rooms_to_plot.push_back("070");
	//rooms_to_plot.push_back("080");
	//rooms_to_plot.push_back("090");
	//rooms_to_plot.push_back("100");
	//rooms_to_plot.push_back("110");
	//rooms_to_plot.push_back("120");
	//rooms_to_plot.push_back("130");
	//rooms_to_plot.push_back("140");
	//srooms_to_plot.push_back("150");

	sprintf(tmp, "<frame ID=\"%d\">\n", frameNr);
	data.append(tmp);

	//		const vector<int>& workingArea=building->GetMPIDispatcher()->GetMyWorkingArea();
	//		for (unsigned int i = 0; i < workingArea.size(); i++) {
	//			Room* r = building->GetRoom(workingArea[i]);


	for (int roomindex = 0; roomindex < building->GetAnzRooms(); roomindex++) {
		Room* r = building->GetRoom(roomindex);
		string caption = r->GetCaption();

		if((rooms_to_plot.empty()==false)&&(IsElementInVector(rooms_to_plot, caption)==false)){
			continue;
		}

		for (int k = 0; k < r->GetAnzSubRooms(); k++) {
			SubRoom* s = r->GetSubRoom(k);
			for (int i = 0; i < s->GetAnzPedestrians(); ++i) {
				Pedestrian* ped = s->GetPedestrian(i);
				data.append(WritePed(ped));
			}
		}
	}
	data.append("</frame>\n");
	Write(data);
}

void IODispatcher::WriteFooter() {
	Write("</trajectoriesDataset>\n");
}


HermesIODispatcher::HermesIODispatcher(string path, int seed, int szenarioID, int fileSection) {
	pFileSectionFrame = fileSection;
	pPath = path+"/juelich/";
	pSeed=seed;
	pScenarioID=szenarioID;

	// traffgo and cologne sind schuldig
	pTranslateX=0;
	pTranslateY=0;

	//try to read the translation coordinates from the file
	string x=ExecuteSystemCommand("sed -n '1p' translation.txt");
	string y=ExecuteSystemCommand("sed -n '2p' translation.txt");
	pTranslateX=atof(x.c_str());
	pTranslateY=atof(y.c_str());

	Log->write("INFO:\t translating all coordinates by [ "+x+", " + y+" ] cm");
}

void HermesIODispatcher::WriteHeader(int nPeds, int fps, Building* building, int seed, int szenarioID) {

	pFPS = fps;
	int fileDelimit = 0;
	//if not supplied, take the one supplied in the constructor
	if(szenarioID==0) szenarioID=pScenarioID;


	//create the directory for the trajectories
	//only the master shall do that

	if(building->GetMPIDispatcher()->GetMyRank()==0){
		string cmd="mkdir -p "+pPath;
		int stat=system(cmd.c_str());
		Log->write("INFO:\t creating the output directory " + pPath);
		if(stat==0)
			Log->write("INFO:\t  done !");
		else
			Log->write("INFO:\t  the dir might be already existing !");
	}
	// wait until the directory has been created
	MPI_Barrier(MPI_COMM_WORLD);

	//get the date
	// setup
	string data;
	string cmd="date";
	char buffer[256];
	// do it
	FILE *stream = popen(cmd.c_str(), "r");
	while ( fgets(buffer, 256, stream) != NULL )
		data.append(buffer);
	pclose(stream);
	data.erase(data.end()-1, data.end());
	const char* date=data.c_str();


	//create the files

	const vector<int>& workingArea=building->GetMPIDispatcher()->GetMyWorkingArea();
	for (unsigned int wa = 0; wa < workingArea.size(); wa++) {
		Room* r = building->GetRoom(workingArea[wa]);

		char fileName[100];
		char tmp[CLENGTH];

		sprintf(fileName, "%s%d-%d_%s.xml", pPath.c_str(), pScenarioID, fileDelimit, r->GetCaption().c_str());
		OutputHandler* tofile = new FileHandler(fileName);
		tofile->write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
		tofile->write("<trajectoriesDataset>");
		tofile->write("\t<header version = \"0.2\">");

		sprintf(tmp, "\t\t<scenarioID>%d</scenarioID>", szenarioID);
		tofile->write(tmp);

		sprintf(tmp, "\t\t<date>%s</date>", date);
		tofile->write(tmp);

		sprintf(tmp, "\t\t<areaCaption>%s</areaCaption>", r->GetCaption().c_str());
		tofile->write(tmp);

		sprintf(tmp, "\t\t<areaLevel>%d</areaLevel>", AreaLevel(r->GetCaption()));
		tofile->write(tmp);

		sprintf(tmp, "\t\t<simCore version=\"%s\">juelich</simCore>", OPS_VERSION);
		tofile->write(tmp);

		sprintf(tmp, "\t\t<seed>%d</seed>", pSeed);
		tofile->write(tmp);

		sprintf(tmp, "\t\t<frameRate>%d</frameRate>", pFPS);
		tofile->write(tmp);

		sprintf(tmp, "\t\t<agents>%d</agents>", r->GetAnzPedestrians());
		tofile->write(tmp);

		sprintf(tmp, "\t\t<simulationFrameStart>%d</simulationFrameStart>", 0);
		tofile->write(tmp);

		sprintf(tmp, "\t\t<simulationFrameStop>%d</simulationFrameStop>", 1500);
		tofile->write(tmp);

		tofile->write("\t</header>");
		r->SetOutputHandler(tofile);
		AddIO(tofile);

	}
}

// no geometry available

void HermesIODispatcher::WriteGeometry(Building* building) {

	return;

	string geometry;
	geometry.append("\t<geometry>\n");
	/// first the rooms
	for (int r = 0; r < building->GetAnzRooms(); r++) {
		Room* room = building->GetRoom(r);
		for (int k = 0; k < room->GetAnzSubRooms(); k++) {
			SubRoom* s = room->GetSubRoom(k);
			//if(k!=71) continue;
			geometry.append(s->WriteSubRoom());
		}
	}
	for (int r = 0; r < building->GetRouting()->GetAnzGoals(); r++) {
		Crossing* goal = building->GetRouting()->GetGoal(r);
		geometry.append(goal->WriteElement());
	}

	geometry.append("\t</geometry>\n");

	const vector<int>& workingArea=building->GetMPIDispatcher()->GetMyWorkingArea();
	//create the files
	for (int i = 0; i < (int)workingArea.size(); i++) {
		Room* r = building->GetRoom(workingArea[i]);
		r->GetOutputHandler()->write(geometry);
	}
}

void HermesIODispatcher::WriteFrame(int frameNr, Building* building) {
	// if the frameNr has run out of file section
	// call the footer
	// flush the pHandlers
	// create new header
	// and continue right here

	const vector<int>& workingArea=building->GetMPIDispatcher()->GetMyWorkingArea();
	for (unsigned int wa = 0; wa < workingArea.size(); wa++) {
		Room* r = building->GetRoom(workingArea[wa]);

		// no trajectories for the playing field
		if(r->GetCaption()=="150") continue;

		string data;
		char tmp[CLENGTH] = "";
		bool anyDataWritten = false;
		sprintf(tmp, "<frame ID=\"%d\">\n", frameNr);
		data.append(tmp);

		for (int k = 0; k < r->GetAnzSubRooms(); k++) {
			SubRoom* s = r->GetSubRoom(k);
			for (int i = 0; i < s->GetAnzPedestrians(); ++i) {
				Pedestrian* ped = s->GetPedestrian(i);
				data.append(WritePed(ped));
				anyDataWritten = true;
			}
		}
		data.append("</frame>\n");
		if (anyDataWritten)
			r->GetOutputHandler()->write(data.c_str());
	}
}

void HermesIODispatcher::WriteFooter() {
	vector<OutputHandler*> io = GetIOHandlers();
	for (unsigned int i = 0; i < io.size(); i++) {
		io[i]->write("</trajectoriesDataset>\n");
	}

}

string HermesIODispatcher::WritePed(Pedestrian* ped) {
	//	double v, a, b, phi;
	//	double RAD2DEG = 180.0 / M_PI;
	//	char tmp[CLENGTH] = "";
	//
	//	v = ped->GetV().Norm();
	//	int color;
	//	double v0 = ped->GetV0Norm();
	//	if (v0 == 0) {
	//		Log->write("ERROR: IODispatcher::WritePed()\t v0=0");
	//		exit(0);
	//	}
	//	color = (int) (v / v0 * 255);
	//	a = ped->GetLargerAxis();
	//	b = ped->GetSmallerAxis();
	//	phi = atan2(ped->GetEllipse().GetSinPhi(), ped->GetEllipse().GetCosPhi());
	//	sprintf(tmp, "<agent ID=\"%d\"\t"
	//			"xPos=\"%.2f\"\tyPos=\"%.2f\""
	//			"radiusA=\"%.2f\"\tradiusB=\"%.2f\"\t"
	//			"ellipseOrientation=\"%.2f\" ellipseColor=\"%d\"/>\n",
	//			ped->GetPedIndex(), (ped->GetPos().GetX()) * FAKTOR,
	//			(ped->GetPos().GetY()) * FAKTOR,
	//			a*FAKTOR, b*FAKTOR, phi * RAD2DEG, color);
	//	return tmp;

	char tmp[CLENGTH];
	sprintf(tmp, "%hd\t%.0f\t%.0f\n",
			ped->GetPedIndex(), (ped->GetPos().GetX()) * FAKTOR +pTranslateX,
			(ped->GetPos().GetY()) * FAKTOR + pTranslateY);
	return tmp;
}

int IODispatcher::AreaLevel(string caption){

	//there are 15 rooms labelled from 010 to 150.
	// 150 is the playing field, 010 to 050 the promenade
	int label = atoi(caption.c_str());

	//	if( (label<60 ) || (label==150)){
	if(label<60 ){
		return 0;
	} else if (label==150){
		return -1;
	}
	else {
		return 1;
	}
}

string HermesIODispatcher::ExecuteSystemCommand(string cmd){
	string data;
	char buffer[256];
	// do it
	FILE *stream = popen(cmd.c_str(), "r");
	while ( fgets(buffer, 256, stream) != NULL )
		data.append(buffer);
	pclose(stream);
	data.erase(data.end()-1, data.end());

	return data;
}


Trajectories::Trajectories() : IODispatcher() {

}

void Trajectories::WriteHeader(int nPeds, int fps, Building* building) {

}

void Trajectories::WriteGeometry(Building* building) {

}

void Trajectories::WriteFrame(int frameNr, Building* building) {
	char tmp[CLENGTH] = "";

	for (int roomindex = 0; roomindex < building->GetAnzRooms(); roomindex++) {
		Room* r = building->GetRoom(roomindex);
		for (int k = 0; k < r->GetAnzSubRooms(); k++) {
			SubRoom* s = r->GetSubRoom(k);
			for (int i = 0; i < s->GetAnzPedestrians(); ++i) {
				Pedestrian* ped = s->GetPedestrian(i);
				double x = ped->GetPos().GetX();
				double y = ped->GetPos().GetY();
				sprintf(tmp, "%d\t%d\t%f\t%f", ped->GetPedIndex(), frameNr, x, y);
				Write(tmp);
			}
		}
	}
}

void Trajectories::WriteFooter() {

}
