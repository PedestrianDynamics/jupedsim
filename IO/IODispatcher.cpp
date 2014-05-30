/**
 * @file   IODispatcher.cpp
 *
 * @date Created on 20. November 2010, 15:20
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
#include "../pedestrian/Pedestrian.h"
#include "../routing/NavMesh.h"

#include "../geometry/SubRoom.h"
#define _USE_MATH_DEFINES
#include <math.h>


using namespace std;


IODispatcher::IODispatcher()
{
     pHandlers = vector<OutputHandler*>();
}

IODispatcher::~IODispatcher()
{
     for (int i = 0; i < (int) pHandlers.size(); i++)
          delete pHandlers[i];
     pHandlers.clear();
}


void IODispatcher::AddIO(OutputHandler* ioh)
{
     pHandlers.push_back(ioh);
}


const vector<OutputHandler*>& IODispatcher::GetIOHandlers()
{
     return pHandlers;
}

void IODispatcher::Write(const std::string& str)
{
     for (vector<OutputHandler*>::iterator it = pHandlers.begin();
               it != pHandlers.end(); ++it) {
          (*it)->Write(str);
     }

}

string IODispatcher::WritePed(Pedestrian* ped)
{
     double RAD2DEG = 180.0 / M_PI;
     char tmp[CLENGTH] = "";

     double v0 = ped->GetV0Norm();
     if (v0 == 0.0) {
          Log->Write("ERROR: IODispatcher::WritePed()\t v0=0");
          exit(0);
     }
     double v = ped->GetV().Norm();
     int color = (int) (v / v0 * 255);
     if(ped->GetSpotlight()==false) color=-1;

     double a = ped->GetLargerAxis();
     double b = ped->GetSmallerAxis();
     double phi = atan2(ped->GetEllipse().GetSinPhi(), ped->GetEllipse().GetCosPhi());
     sprintf(tmp, "<agent ID=\"%d\"\t"
             "xPos=\"%.2f\"\tyPos=\"%.2f\"\t"
             "zPos=\"%.2f\"\t"
             "radiusA=\"%.2f\"\tradiusB=\"%.2f\"\t"
             "ellipseOrientation=\"%.2f\" ellipseColor=\"%d\"/>\n",
             ped->GetID(), (ped->GetPos().GetX()) * FAKTOR,
             (ped->GetPos().GetY()) * FAKTOR,(ped->GetElevation()+0.3) * FAKTOR ,a * FAKTOR, b * FAKTOR,
             phi * RAD2DEG, color);
     return tmp;
}

void IODispatcher::WriteHeader(int nPeds, double fps, Building* building, int seed      )
{

     nPeds = building->GetNumberOfPedestrians();
     string tmp;
     tmp =
          "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\n" "<trajectories>\n";
     char agents[CLENGTH] = "";
     sprintf(agents, "\t<header version = \"%s\">\n", JPS_VERSION);
     tmp.append(agents);
     sprintf(agents, "\t\t<agents>%d</agents>\n", nPeds);
     tmp.append(agents);
     sprintf(agents, "\t\t<seed>%d</seed>\n", seed);
     tmp.append(agents);
     sprintf(agents, "\t\t<frameRate>%f</frameRate>\n", fps);
     tmp.append(agents);
     tmp.append("\t</header>\n");
     Write(tmp);

}

void IODispatcher::WriteGeometry(Building* building)
{
     // just put a link to the geometry file
     string embed_geometry;
     embed_geometry.append("\t<geometry>\n");
     char file_location[CLENGTH] = "";
     sprintf(file_location, "\t<file location= \"%s\"/>\n", building->GetGeometryFilename().c_str());
     embed_geometry.append(file_location);
     embed_geometry.append("\t</geometry>\n");
     //Write(embed_geometry);
     //return;
     //
     string geometry;
     geometry.append("\t<geometry>\n");

     bool plotHlines = true;
     bool plotCrossings = true;
     bool plotTransitions = true;
     bool plotPlayingField=false;
     vector<string> rooms_to_plot;

     //Promenade
     //rooms_to_plot.push_back("outside");
     //rooms_to_plot.push_back("010");

     // first the rooms
     //to avoid writing navigation line twice
     vector<int> navLineWritten;

     for (int i = 0; i < building->GetNumberOfRooms(); i++) {
          Room* r = building->GetRoom(i);
          string caption = r->GetCaption(); //if(r->GetID()!=1) continue;
          if (rooms_to_plot.empty() == false)
               if (IsElementInVector(rooms_to_plot, caption) == false)
                    continue;

          for (int k = 0; k < r->GetNumberOfSubRooms(); k++) {
               SubRoom* s = r->GetSubRoom(k); //if(s->GetSubRoomID()!=0) continue;
               geometry.append(s->WriteSubRoom());

               // the hlines
               if (plotHlines) {
                    const vector<Hline*>& hlines = s->GetAllHlines();
                    for (unsigned int i = 0; i < hlines.size(); i++) {
                         Hline* hline = hlines[i];
                         int uid1 = hline->GetUniqueID();
                         if (!IsElementInVector(navLineWritten, uid1)) {
                              navLineWritten.push_back(uid1);
                              if (rooms_to_plot.empty()
                                        || IsElementInVector(rooms_to_plot, caption)) {
                                   geometry.append(hline->WriteElement());
                              }
                         }
                    }

                    // the crossings
                    if (plotCrossings) {
                         const vector<Crossing*>& crossings = s->GetAllCrossings();
                         for (unsigned int i = 0; i < crossings.size(); i++) {
                              Crossing* crossing = crossings[i];
                              int uid1 = crossing->GetUniqueID();
                              if (!IsElementInVector(navLineWritten, uid1)) {
                                   navLineWritten.push_back(uid1);
                                   if (rooms_to_plot.empty()
                                             || IsElementInVector(rooms_to_plot,
                                                                  caption)) {
                                        geometry.append(crossing->WriteElement());
                                   }
                              }
                         }
                    }

                    // the transitions
                    if (plotTransitions) {
                         const vector<Transition*>& transitions =
                              s->GetAllTransitions();
                         for (unsigned int i = 0; i < transitions.size(); i++) {
                              Transition* transition = transitions[i];
                              int uid1 = transition->GetUniqueID();
                              if (!IsElementInVector(navLineWritten, uid1)) {
                                   navLineWritten.push_back(uid1);

                                   if (rooms_to_plot.empty()) {
                                        geometry.append(transition->WriteElement());

                                   } else {

                                        Room* room1 = transition->GetRoom1();
                                        Room* room2 = transition->GetRoom2();
                                        string caption1 = room1->GetCaption();
                                        if (room2) {
                                             string caption2 = room2->GetCaption();
                                             if (IsElementInVector(rooms_to_plot,
                                                                   caption1)
                                                       || IsElementInVector(rooms_to_plot,
                                                                            caption2)) {
                                                  geometry.append(transition->WriteElement());
                                             }

                                        } else {
                                             if (IsElementInVector(rooms_to_plot,
                                                                   caption1)) {
                                                  geometry.append(transition->WriteElement());
                                             }
                                        }

                                   }
                              }
                         }
                    }
               }
          }
     }

     //eventually write any goal
     for (map<int, Goal*>::const_iterator itr = building->GetAllGoals().begin();
               itr != building->GetAllGoals().end(); ++itr) {
          geometry.append(itr->second->Write());
     }

     if(plotPlayingField) {
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
}

void IODispatcher::WriteFrame(int frameNr, Building* building)
{
     string data;
     char tmp[CLENGTH] = "";
     vector<string> rooms_to_plot;

     //promenade
     //rooms_to_plot.push_back("010");

     sprintf(tmp, "<frame ID=\"%d\">\n", frameNr);
     data.append(tmp);

     for (int roomindex = 0; roomindex < building->GetNumberOfRooms(); roomindex++) {
          Room* r = building->GetRoom(roomindex);
          string caption = r->GetCaption();

          if ((rooms_to_plot.empty() == false)
                    && (IsElementInVector(rooms_to_plot, caption) == false)) {
               continue;
          }

          for (int k = 0; k < r->GetNumberOfSubRooms(); k++) {
               SubRoom* s = r->GetSubRoom(k);
               for (int i = 0; i < s->GetNumberOfPedestrians(); ++i) {
                    Pedestrian* ped = s->GetPedestrian(i);
                    data.append(WritePed(ped));
               }
          }
     }
     data.append("</frame>\n");
     Write(data);
}

void IODispatcher::WriteFooter()
{
     Write("</trajectories>\n");
}


/**
 * FLAT format implementation
 */

TrajectoriesFLAT::TrajectoriesFLAT() :
     IODispatcher()
{
}

void TrajectoriesFLAT::WriteHeader(int nPeds, double fps, Building* building, int seed)
{

}

void TrajectoriesFLAT::WriteGeometry(Building* building)
{

}

void TrajectoriesFLAT::WriteFrame(int frameNr, Building* building)
{
     char tmp[CLENGTH] = "";

     for (int roomindex = 0; roomindex < building->GetNumberOfRooms(); roomindex++) {
          Room* r = building->GetRoom(roomindex);
          for (int k = 0; k < r->GetNumberOfSubRooms(); k++) {
               SubRoom* s = r->GetSubRoom(k);
               for (int i = 0; i < s->GetNumberOfPedestrians(); ++i) {
                    Pedestrian* ped = s->GetPedestrian(i);
                    double x = ped->GetPos().GetX();
                    double y = ped->GetPos().GetY();
                    sprintf(tmp, "%d\t%d\t%f\t%f", ped->GetID(), frameNr, x,
                            y);
                    Write(tmp);
               }
          }
     }
}

void TrajectoriesFLAT::WriteFooter()
{

}


/**
 *  VTK Implementation of the geometry and trajectories
 */


TrajectoriesVTK::TrajectoriesVTK()
{
}

void TrajectoriesVTK::WriteHeader(int nPeds, double fps, Building* building, int seed)
{
     Write("# vtk DataFile Version 4.0");
     Write(building->GetCaption());
     Write("ASCII");
     Write("");
}

void TrajectoriesVTK::WriteGeometry(Building* building)
{
     stringstream tmp;

     NavMesh* nv= new NavMesh(building);
     nv->BuildNavMesh();
     //nv->WriteToFile("../pedunc/examples/stadium/arena.nav");
     Write("DATASET UNSTRUCTURED_GRID");

     //writing the vertices
     const vector<NavMesh::JVertex*>& vertices= nv->GetVertices() ;
     tmp<<"POINTS "<<vertices.size()<<" FLOAT"<<endl;
     for (unsigned int v=0; v<vertices.size(); v++) {
          tmp<<vertices[v]->pPos.GetX()<<" " <<vertices[v]->pPos.GetY() <<" 0.0"<<endl;
     }
     Write(tmp.str());
     tmp.str(std::string());

     //writing the cells data
     const vector<NavMesh::JNode*>& cells= nv->GetNodes();
     int nComponents=cells.size();
     stringstream tmp1;
     for (unsigned int n=0; n<cells.size(); n++) {
          int hSize=cells[n]->pHull.size();

          tmp1<<hSize<<"";
          for(unsigned int i=0; i<cells[n]->pHull.size(); i++) {
               tmp1<<" "<<cells[n]->pHull[i].id;
          }
          tmp1<<endl;
          nComponents+= hSize;
     }
     tmp<<"CELLS "<<cells.size()<<" "<<nComponents<<endl;
     tmp<<tmp1.str();
     Write(tmp.str());
     tmp.str(std::string());

     // writing the cell type
     tmp<<"CELL_TYPES "<<cells.size()<<endl;
     for (unsigned int n=0; n<cells.size(); n++) {
          tmp<<"9"<<endl;
     }

     Write(tmp.str());
     delete nv;
}

void TrajectoriesVTK::WriteFrame(int frameNr, Building* building)
{
}

void TrajectoriesVTK::WriteFooter()
{
}






void TrajectoriesJPSV06::WriteHeader(int nPeds, double fps, Building* building, int seed)
{
     nPeds = building->GetNumberOfPedestrians();
     string tmp;
     tmp =
          "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\n" "<trajectories>\n";
     char agents[CLENGTH] = "";
     sprintf(agents, "\t<header version = \"0.6\">\n");
     tmp.append(agents);
     sprintf(agents, "\t\t<agents>%d</agents>\n", nPeds);
     tmp.append(agents);
     sprintf(agents, "\t\t<seed>%d</seed>\n", seed);
     tmp.append(agents);
     sprintf(agents, "\t\t<frameRate>%f</frameRate>\n", fps);
     tmp.append(agents);
     tmp.append("\t\t<!-- Frame count HACK\n");
     tmp.append("replace me\n");
     tmp.append("\t\tFrame count HACK -->\n");
//      tmp.append("<frameCount>xxxxxxx</frameCount>\n");
     tmp.append("\t</header>\n");
     Write(tmp);

}

void TrajectoriesJPSV06::WriteGeometry(Building* building)
{
     // just put a link to the geometry file
     string embed_geometry;
     embed_geometry.append("\t<geometry>\n");
     char file_location[CLENGTH] = "";
     sprintf(file_location, "\t<file location= \"%s\"/>\n", building->GetGeometryFilename().c_str());
     embed_geometry.append(file_location);
     embed_geometry.append("\t</geometry>\n");
     Write(embed_geometry);

     Write("\t<AttributeDescription>");
     Write("\t\t<property tag=\"x\" description=\"xPosition\"/>");
     Write("\t\t<property tag=\"y\" description=\"yPosition\"/>");
     Write("\t\t<property tag=\"z\" description=\"zPosition\"/>");
     Write("\t\t<property tag=\"rA\" description=\"radiusA\"/>");
     Write("\t\t<property tag=\"rB\" description=\"radiusB\"/>");
     Write("\t\t<property tag=\"eC\" description=\"ellipseColor\"/>");
     Write("\t\t<property tag=\"eO\" description=\"ellipseOrientation\"/>");
     Write("\t</AttributeDescription>\n");
}

void TrajectoriesJPSV06::WriteFrame(int frameNr, Building* building)
{
     string data;
     char tmp[CLENGTH] = "";
     double RAD2DEG = 180.0 / M_PI;
     vector<string> rooms_to_plot;

     //promenade
     //rooms_to_plot.push_back("010");

     sprintf(tmp, "<frame ID=\"%d\">\n", frameNr);
     data.append(tmp);

     for (int roomindex = 0; roomindex < building->GetNumberOfRooms(); roomindex++) {
          Room* r = building->GetRoom(roomindex);
          string caption = r->GetCaption();

          if ((rooms_to_plot.empty() == false)
                    && (IsElementInVector(rooms_to_plot, caption) == false)) {
               continue;
          }

          for (int k = 0; k < r->GetNumberOfSubRooms(); k++) {
               SubRoom* s = r->GetSubRoom(k);
               for (int i = 0; i < s->GetNumberOfPedestrians(); ++i) {
                    Pedestrian* ped = s->GetPedestrian(i);


                    char tmp[CLENGTH] = "";

                    double v0 = ped->GetV0Norm();
                    if (v0 == 0.0) {
                         Log->Write("ERROR: TrajectoriesJPSV06()\t v0=0");
                         exit(0);
                    }
                    double v = ped->GetV().Norm();
                    int color = (int) (v / v0 * 255);
                    if(ped->GetSpotlight()==false) color=-1;

                    double a = ped->GetLargerAxis();
                    double b = ped->GetSmallerAxis();
                    double phi = atan2(ped->GetEllipse().GetSinPhi(), ped->GetEllipse().GetCosPhi());
                    sprintf(tmp, "<agent ID=\"%d\"\t"
                            "x=\"%.2f\"\ty=\"%.2f\"\t"
                            "z=\"%.2f\"\t"
                            "rA=\"%.2f\"\trB=\"%.2f\"\t"
                            "eO=\"%.2f\" eC=\"%d\"/>\n",
                            ped->GetID(), (ped->GetPos().GetX()) * FAKTOR,
                            (ped->GetPos().GetY()) * FAKTOR,(ped->GetElevation()+0.3) * FAKTOR ,a * FAKTOR, b * FAKTOR,
                            phi * RAD2DEG, color);
                    data.append(tmp);
               }
          }
     }
     data.append("</frame>\n");
     Write(data);
}

void TrajectoriesJPSV06::WriteFooter()
{
     Write("</trajectories>\n");
}


void TrajectoriesXML_MESH::WriteGeometry(Building* building)
{
     //Navigation mesh implementation
     NavMesh* nv= new NavMesh(building);
     nv->BuildNavMesh();
     string geometry;
     nv->WriteToString(geometry);
     Write("<geometry>");
     Write(geometry);
     Write("</geometry>");
     nv->WriteToFile(building->GetProjectFilename()+".full.nav");
     delete nv;
}
