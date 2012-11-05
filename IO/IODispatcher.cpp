/**
 * File:   IODispatcher.cpp
 *
 * Created on 20. November 2010, 15:20
 *
 * @section LICENSE
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * @section DESCRIPTION
 *
 *
 *
 */

#include "IODispatcher.h"
//#include <boost/foreach.hpp>

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
			"xPos=\"%.2f\"\tyPos=\"%.2f\"\t"
			"radiusA=\"%.2f\"\tradiusB=\"%.2f\"\t"
			"ellipseOrientation=\"%.2f\" ellipseColor=\"%d\"/>\n",
			ped->GetPedIndex(), (ped->GetPos().GetX()) * FAKTOR,
			(ped->GetPos().GetY()) * FAKTOR, a * FAKTOR, b * FAKTOR,
			phi * RAD2DEG, color);
	
	return tmp;
}

// Konstruktoren

IODispatcher::IODispatcher() {
	pHandlers = vector<OutputHandler*>();
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
}
;

const vector<OutputHandler*>& IODispatcher::GetIOHandlers() {
	return pHandlers;
}

void IODispatcher::Write(string str) {
	for (vector<OutputHandler*>::iterator it = pHandlers.begin();
			it != pHandlers.end(); ++it) {
		(*it)->write(str);
	}

}
;

void IODispatcher::WriteHeader(int nPeds, int fps, Building* building, int seed,
		int szenarioID) {

	nPeds = building->GetAnzPedestrians();
	string tmp;
	tmp =
			"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\n" "<trajectoriesDataset>\n";
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

	bool plotHlines = true;
	bool plotCrossings = true;
	bool plotTransitions = true;
	vector<string> rooms_to_plot;

	//Promenade
	//rooms_to_plot.push_back("outside");
	//rooms_to_plot.push_back("010");

	// first the rooms
	//to avoid writing navigation line twice
	vector<int> navLineWritten;

	for (int i = 0; i < building->GetAnzRooms(); i++) {
		Room* r = building->GetRoom(i);
		string caption = r->GetCaption();
		if (rooms_to_plot.empty() == false)
			if (IsElementInVector(rooms_to_plot, caption) == false)
				continue;

		for (int k = 0; k < r->GetAnzSubRooms(); k++) {
			SubRoom* s = r->GetSubRoom(k);
			geometry.append(s->WriteSubRoom());

			// the hlines
			if (plotHlines) {
				const vector<Hline*>& hlines = s->GetAllHlines();
				for (unsigned int i = 0; i < hlines.size(); i++) {
					Hline* goal = hlines[i];
					int uid1 = goal->GetUniqueID();
					if (!IsElementInVector(navLineWritten, uid1)) {
						navLineWritten.push_back(uid1);
						if (rooms_to_plot.empty()
								|| IsElementInVector(rooms_to_plot, caption)) {
							geometry.append(goal->WriteElement());
						}
					}
				}

				// the crossings
				if (plotCrossings) {
					const vector<Crossing*>& crossings = s->GetAllCrossings();
					for (unsigned int i = 0; i < crossings.size(); i++) {
						Crossing* goal = crossings[i];
						int uid1 = goal->GetUniqueID();
						if (!IsElementInVector(navLineWritten, uid1)) {
							navLineWritten.push_back(uid1);
							if (rooms_to_plot.empty()
									|| IsElementInVector(rooms_to_plot,
											caption)) {
								geometry.append(goal->WriteElement());
							}
						}
					}
				}

				// the transitions
				if (plotTransitions) {
					const vector<Transition*>& transitions =
							s->GetAllTransitions();
					for (unsigned int i = 0; i < transitions.size(); i++) {
						Transition* goal = transitions[i];
						int uid1 = goal->GetUniqueID();
						if (!IsElementInVector(navLineWritten, uid1)) {
							navLineWritten.push_back(uid1);

							if (rooms_to_plot.empty()) {
								geometry.append(goal->WriteElement());

							} else {

								Room* room1 = goal->GetRoom1();
								Room* room2 = goal->GetRoom2();
								string caption1 = room1->GetCaption();
								if (room2) {
									string caption2 = room2->GetCaption();
									if (IsElementInVector(rooms_to_plot,
											caption1)
											|| IsElementInVector(rooms_to_plot,
													caption2)) {
										geometry.append(goal->WriteElement());
									}

								} else {
									if (IsElementInVector(rooms_to_plot,
											caption1)) {
										geometry.append(goal->WriteElement());
									}
								}

							}
						}
					}
				}
			}
		}
	}

	geometry.append("\t</geometry>\n");
	Write(geometry);
}

void IODispatcher::WriteGeometryRVO(Building* building) {

	string geometry;
	geometry.append("<geometry>\n");

	bool plotCrossings = false;
	bool plotTransitions = false;
	vector<string> rooms_to_plot;

	//Promenade
	//rooms_to_plot.push_back("outside");
	//rooms_to_plot.push_back("010");

	// first the rooms
	//to avoid writing navigation line twice
	vector<int> navLineWritten;

	for (int i = 0; i < building->GetAnzRooms(); i++) {
		Room* r = building->GetRoom(i);
		string caption = r->GetCaption();
		if (rooms_to_plot.empty() == false)
			if (IsElementInVector(rooms_to_plot, caption) == false)
				continue;

		for (int k = 0; k < r->GetAnzSubRooms(); k++) {
			SubRoom* s = r->GetSubRoom(k);
			geometry.append(s->WritePolyLine());

			// the crossings
			if (plotCrossings) {
				const vector<Crossing*>& crossings = s->GetAllCrossings();
				for (unsigned int i = 0; i < crossings.size(); i++) {
					Crossing* goal = crossings[i];
					int uid1 = goal->GetUniqueID();
					if (!IsElementInVector(navLineWritten, uid1)) {
						navLineWritten.push_back(uid1);
						if (rooms_to_plot.empty()
								|| IsElementInVector(rooms_to_plot, caption)) {
							geometry.append(goal->WriteElement());
						}
					}
				}
			}

			// the transitions
			if (plotTransitions) {
				const vector<Transition*>& transitions = s->GetAllTransitions();
				for (unsigned int i = 0; i < transitions.size(); i++) {
					Transition* goal = transitions[i];
					int uid1 = goal->GetUniqueID();
					if (!IsElementInVector(navLineWritten, uid1)) {
						navLineWritten.push_back(uid1);

						if (rooms_to_plot.empty()) {
							geometry.append(goal->WriteElement());

						} else {

							Room* room1 = goal->GetRoom1();
							Room* room2 = goal->GetRoom2();
							string caption1 = room1->GetCaption();
							if (room2) {
								string caption2 = room2->GetCaption();
								if (IsElementInVector(rooms_to_plot, caption1)
										|| IsElementInVector(rooms_to_plot,
												caption2)) {
									geometry.append(goal->WriteElement());
								}

							} else {
								if (IsElementInVector(rooms_to_plot,
										caption1)) {
									geometry.append(goal->WriteElement());
								}
							}

						}
					}
				}
			}
		}
	}

	geometry.append("</geometry>\n");

	ofstream myfile;
	myfile.open("ArenaRVO.xml");
	myfile << geometry;
	myfile.close();

}

void IODispatcher::WriteNavMeshORCA(Building* building) {

	vector<string> rooms_to_plot;

	//Promenade
	//rooms_to_plot.push_back("outside");
	//rooms_to_plot.push_back("010");

	// first the rooms
	//to avoid writing navigation line twice
	vector<int> navLineWritten;

	//algo
	//	  '''Writes the ascii navigation mesh file'''
	//	        f = open ( fileName, 'w' )
	//	        # vertices
	//	        f.write( '%d' % len( self.vertices) )
	//	        for x,y in self.vertices:
	//	            f.write( '\n\t%.5f %.5f' % ( x, y ) )
	//	        #edges
	//	        f.write( '\n%d' % len( self.edges ) )
	//	        for e in self.edges:
	//	            f.write( e.asciiString() )
	//	        # nodes
	//	        f.write( '\n%d' % len( self.nodes ) )
	//	        for n in self.nodes:
	//	            f.write( n.asciiString() )
	//	        # obstacles
	//	        f.write( '\n%d' % len( self.obstacles ) )
	//	        for o in self.obstacles:
	//	            f.write( '\n\t%d %s' % ( len( o ), ' '.join( map( lambda x: str(x), o ) ) ) )
	//	        f.close()

	//to avoid vertices being written twice

	vector<Point> vertices;

	stringstream str_vertices;
	stringstream str_edges;
	stringstream str_nodes;
	stringstream str_obstacles;

	str_vertices.precision(2);
	str_vertices << fixed;
	str_edges.precision(2);
	str_edges << fixed;
	str_nodes.precision(2);
	str_nodes << fixed;
	str_obstacles.precision(2);
	str_obstacles << fixed;

	int edgesCount = 0;
	int subRoomCount = 0;

	for (int i = 0; i < building->GetAnzRooms(); i++) {
		Room* r = building->GetRoom(i);
		string caption = r->GetCaption();
		if (rooms_to_plot.empty() == false)
			if (IsElementInVector(rooms_to_plot, caption) == false)
				continue;

		for (int k = 0; k < r->GetAnzSubRooms(); k++) {
			SubRoom* s = r->GetSubRoom(k);
			subRoomCount++;
			vector<Line> edges;

			//vertices
			const vector<Point>& pol = s->GetPolygon();

			bool isContain = false;
			for (unsigned int p = 0; p < pol.size(); p++) {
				for (unsigned int v = 0; v < vertices.size(); v++) {

					if (vertices[v] == pol[p]) {
						isContain = true;
						break;
					}
				}
				if (isContain == false) {
					vertices.push_back(pol[p]);
					str_vertices << "\t" << pol[p].GetX() << " "
							<< pol[p].GetY() << endl;
				}
			}
			//edges
			const vector<Wall>& walls = s->GetAllWalls();
			for (unsigned w = 0; w < walls.size(); w++) {
				edges.push_back(walls[w]);

				const Point& p1 = s->GetCentroid();
				const Point& p2 = walls[w].GetCentre() -p1;

				str_edges << " " << walls[w].GetPoint1().GetX() << " "
						<< walls[w].GetPoint1().GetY() << endl << "\t"
						<< p2.GetX() << " "
						<< p2.GetY() << endl << "\t"
						<< " " << "-1 " << s->GetUID() << endl;
			}

			const vector<Crossing*>& crossings = s->GetAllCrossings();
			for (unsigned c = 0; c < crossings.size(); c++) {
				edges.push_back(*crossings[c]);
				int sb1 = crossings[c]->GetSubRoom1()->GetUID();
				int sb2 = crossings[c]->GetSubRoom2()->GetUID();
				Point p1 = crossings[c]->GetSubRoom1()->GetCentroid();
				Point p2 = crossings[c]->GetSubRoom2()->GetCentroid();
				double dist = (p2 - p1).Norm();

				if (sb1 > sb2) {
					swap(sb1, sb2);
				}

				str_edges << " " << crossings[c]->GetPoint1().GetX() << " "
						<< crossings[c]->GetPoint1().GetY() << endl << "\t"
						<< crossings[c]->GetPoint2().GetX() << " "
						<< crossings[c]->GetPoint2().GetY() << endl << "\t"
						<< dist << " " << sb1 << " " << sb2 << endl;
			}

			const vector<Transition*>& transitions = s->GetAllTransitions();
			for (unsigned t = 0; t < transitions.size(); t++) {
				edges.push_back(*transitions[t]);
				int sb1 = transitions[t]->GetSubRoom1()->GetUID();
				int sb2 =
						(transitions[t]->GetSubRoom2() == NULL) ?
								-1 : transitions[t]->GetSubRoom2()->GetUID();

				Point p1 = transitions[t]->GetSubRoom1()->GetCentroid();

				Point p2 =
						(transitions[t]->GetSubRoom2() == NULL) ?
								transitions[t]->GetCentre() :
								transitions[t]->GetSubRoom2()->GetCentroid();

				double dist = (p2 - p1).Norm();

				if (sb1 > sb2) {
					swap(sb1, sb2);
				}

				str_edges << " " << transitions[t]->GetPoint1().GetX() << " "
						<< transitions[t]->GetPoint1().GetY() << endl << "\t"
						<< transitions[t]->GetPoint2().GetX() << " "
						<< transitions[t]->GetPoint2().GetY() << endl << "\t"
						<< dist << " " << sb1 << " " << sb2 << endl;
			}

			//Nodes
			//writing the IDs
			stringstream tmp;
			int vcount = 0;
			for (unsigned int p = 0; p < pol.size(); p++) {
				for (unsigned int v = 0; v < vertices.size(); v++) {
					if (vertices[v] == pol[p]) {
						vcount++;
						tmp << " " << v;
					}
				}
			}

			str_nodes << vcount << tmp.rdbuf() << endl;

			str_nodes << "\t" << "0.00 0.00 0.00 " << endl;

			Point c = s->GetCentroid();
			str_nodes << "\t" << c.GetX() << " " << c.GetY() << endl;
			str_nodes << "\t";

			edgesCount += edges.size();
			str_nodes << edges.size() << " ";
			for (unsigned int e = 0; e < edges.size(); e++) {
				str_nodes << edges[e].GetUniqueID() << " ";
			}
			str_nodes << endl;
		}
	}

	ofstream myfile;
	myfile.open("ArenaORCA.nav");
	//	myfile<<vertices.size()<<endl;
	//	myfile<<str_vertices.rdbuf()<<endl;
	myfile << edgesCount << endl;
	myfile << str_edges.rdbuf() << endl;
	//	myfile<<subRoomCount<<endl;
	//	myfile<<str_nodes.rdbuf();
	//	myfile<<str_obstacles.rdbuf();
	myfile.close();
}

void IODispatcher::WriteFrame(int frameNr, Building* building) {
	string data;
	char tmp[CLENGTH] = "";
	vector<string> rooms_to_plot;

	//promenade
	//rooms_to_plot.push_back("010");

	sprintf(tmp, "<frame ID=\"%d\">\n", frameNr);
	data.append(tmp);

	for (int roomindex = 0; roomindex < building->GetAnzRooms(); roomindex++) {
		Room* r = building->GetRoom(roomindex);
		string caption = r->GetCaption();

		if ((rooms_to_plot.empty() == false)
				&& (IsElementInVector(rooms_to_plot, caption) == false)) {
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

Trajectories::Trajectories() :
		IODispatcher() {

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
				sprintf(tmp, "%d\t%d\t%f\t%f", ped->GetPedIndex(), frameNr, x,
						y);
				Write(tmp);
			}
		}
	}
}

void Trajectories::WriteFooter() {

}

