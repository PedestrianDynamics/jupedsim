/**
 * \file        IODispatcher.cpp
 * \date        Nov 20, 2010
 * \version     v0.7
 * \copyright   <2009-2015> Forschungszentrum Jülich GmbH. All rights reserved.
 *
 * \section License
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * \section Description
 *
 *
 **/
#include "IODispatcher.h"

#include "OutputHandler.h"

#include "general/Filesystem.h"
#include "geometry/SubRoom.h"
#include "mpi/LCGrid.h"
#include "pedestrian/Pedestrian.h"
#include "pedestrian/AgentsSource.h"

#include <tinyxml.h>

#define _USE_MATH_DEFINES


IODispatcher::IODispatcher()
{
     _outputHandlers = std::vector<Trajectories*>();
}

IODispatcher::~IODispatcher()
{
     for (int i = 0; i < (int) _outputHandlers.size(); ++i)
          delete _outputHandlers[i];
     _outputHandlers.clear();
}


void IODispatcher::AddIO(Trajectories* ioh)
{
     _outputHandlers.push_back(ioh);
}


const std::vector<Trajectories*>& IODispatcher::GetIOHandlers()
{
     return _outputHandlers;
}

void IODispatcher::WriteHeader(long nPeds, double fps, Building* building, int seed, int count)
{
     for (auto const & it : _outputHandlers)
     {
          it->WriteHeader(nPeds, fps, building, seed, count);
     }
}
void IODispatcher::WriteGeometry(Building* building)
{
     for(auto const & it : _outputHandlers)
     {
          it->WriteGeometry(building);
     }
}
void IODispatcher::WriteFrame(int frameNr, Building* building)
{
     for (auto const & it : _outputHandlers)
     {
          it->WriteFrame(frameNr, building);
     }
}
void IODispatcher::WriteFooter()
{
     for(auto const it : _outputHandlers)
     {
          it->WriteFooter();
     }
}
 void IODispatcher::WriteSources(const std::vector<std::shared_ptr<AgentsSource> > sources)
 {
      for(auto const it : _outputHandlers)
      {
           it->WriteSources(sources);
      }
 }


std::string TrajectoriesJPSV04::WritePed(Pedestrian* ped)
{
     double RAD2DEG = 180.0 / M_PI;
     char tmp[CLENGTH] = "";
     int color=ped->GetColor();
     double a = ped->GetLargerAxis();
     double b = ped->GetSmallerAxis();
     double phi = atan2(ped->GetEllipse().GetSinPhi(), ped->GetEllipse().GetCosPhi());
     sprintf(tmp, "<agent ID=\"%d\"\t"
               "xPos=\"%.2f\"\tyPos=\"%.2f\"\t"
               "zPos=\"%.2f\"\t"
               "radiusA=\"%.2f\"\tradiusB=\"%.2f\"\t"
               "ellipseOrientation=\"%.2f\" ellipseColor=\"%d\"/>\n",
               ped->GetID(), (ped->GetPos()._x) * FAKTOR,
               (ped->GetPos()._y) * FAKTOR,(ped->GetElevation()) * FAKTOR ,a * FAKTOR, b * FAKTOR,
               phi * RAD2DEG, color);

     return std::string(tmp);
}

void TrajectoriesJPSV04::WriteHeader(long nPeds, double fps, Building* building, int seed, int /*count*/)
{
     building->GetCaption();
     std::string tmp;
     tmp =
               "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\n" "<trajectories>\n";
     char agents[CLENGTH] = "";
     sprintf(agents, "\t<header version = \"%s\">\n", JPS_VERSION);
     tmp.append(agents);
     sprintf(agents, "\t\t<agents>%ld</agents>\n", nPeds);
     tmp.append(agents);
     sprintf(agents, "\t\t<seed>%d</seed>\n", seed);
     tmp.append(agents);
     sprintf(agents, "\t\t<frameRate>%0.2f</frameRate>\n", fps);
     tmp.append(agents);
     tmp.append("\t</header>\n");
     Write(tmp);

}

void TrajectoriesJPSV04::WriteGeometry(Building* building)
{
     // just put a link to the geometry file
     std::string embed_geometry;
     embed_geometry.append("\t<geometry>\n");
     char file_location[CLENGTH] = "";
     sprintf(file_location, "\t<file location= \"%s\"/>\n", building->GetGeometryFilename().string().c_str());
     embed_geometry.append(file_location);
     embed_geometry.append("\t</geometry>\n");
     //Write(embed_geometry);
     //return;
     //
     std::string geometry;
     geometry.append("\t<geometry>\n");

     bool plotHlines = true;
     bool plotCrossings = true;
     bool plotTransitions = true;
     bool plotPlayingField=false;
     std::vector<std::string> rooms_to_plot;
     unsigned int i;
     // first the rooms
     //to avoid writing navigation line twice
     std::vector<int> navLineWritten;
     //rooms_to_plot.push_back("U9");

     for (const auto& it:building->GetAllRooms())
     {
          auto&& r = it.second;
          std::string caption = r->GetCaption(); //if(r->GetID()!=1) continue;
          if (!rooms_to_plot.empty() && !IsElementInVector(rooms_to_plot, caption))
               continue;

          for(auto&& sitr: r->GetAllSubRooms())
          {
               auto&& s = sitr.second; //if(s->GetSubRoomID()!=7) continue;
               geometry.append(s->WriteSubRoom());

               // the hlines
               if (plotHlines) {
                    const std::vector<Hline*>& hlines = s->GetAllHlines();
                    for (i = 0; i < hlines.size(); i++) {
                         Hline* hline = hlines[i];
                         int uid1 = hline->GetUniqueID();
                         if (!IsElementInVector(navLineWritten, uid1)) {
                              navLineWritten.push_back(uid1);
                              if (rooms_to_plot.empty()
                                        || IsElementInVector(rooms_to_plot, caption)) {
                                   geometry.append(hline->GetDescription());
                              }
                         }
                    }

                    // the crossings
                    if (plotCrossings) {
                         const std::vector<Crossing*>& crossings = s->GetAllCrossings();
                         for (i = 0; i < crossings.size(); i++) {
                              Crossing* crossing = crossings[i];
                              int uid1 = crossing->GetUniqueID();
                              if (!IsElementInVector(navLineWritten, uid1)) {
                                   navLineWritten.push_back(uid1);
                                   if (rooms_to_plot.empty()
                                             || IsElementInVector(rooms_to_plot,
                                                       caption)) {
                                        geometry.append(crossing->GetDescription());
                                   }
                              }
                         }
                    }

                    // the transitions
                    if (plotTransitions) {
                         const std::vector<Transition*>& transitions =
                                   s->GetAllTransitions();
                         for (i = 0; i < transitions.size(); i++) {
                              Transition* transition = transitions[i];
                              int uid1 = transition->GetUniqueID();
                              if (!IsElementInVector(navLineWritten, uid1)) {
                                   navLineWritten.push_back(uid1);

                                   if (rooms_to_plot.empty()) {
                                        geometry.append(transition->GetDescription());

                                   } else {

                                        Room* room1 = transition->GetRoom1();
                                        Room* room2 = transition->GetRoom2();
                                        std::string caption1 = room1->GetCaption();
                                        if (room2) {
                                             std::string caption2 = room2->GetCaption();
                                             if (IsElementInVector(rooms_to_plot,
                                                       caption1)
                                                       || IsElementInVector(rooms_to_plot,
                                                                 caption2)) {
                                                  geometry.append(transition->GetDescription());
                                             }

                                        } else {
                                             if (IsElementInVector(rooms_to_plot,
                                                       caption1)) {
                                                  geometry.append(transition->GetDescription());
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
     for (std::map<int, Goal*>::const_iterator itr = building->GetAllGoals().begin();
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

void TrajectoriesJPSV04::WriteFrame(int frameNr, Building* building)
{
     std::string data;
     char tmp[CLENGTH] = "";
     std::vector<std::string> rooms_to_plot;

     if( building->GetAllPedestrians().size() == 0)
          return;

     sprintf(tmp, "<frame ID=\"%d\">\n", frameNr);
     data.append(tmp);

     const std::vector< Pedestrian* >& allPeds = building->GetAllPedestrians();
     for(unsigned int p=0;p<allPeds.size();p++)
     {
          Pedestrian* ped = allPeds[p];
          Room* r = building->GetRoom(ped->GetRoomID());
          std::string caption = r->GetCaption();

          if ((rooms_to_plot.empty() == false)
                    && (IsElementInVector(rooms_to_plot, caption) == false)) {
               continue;
          }
          data.append(WritePed(ped));
     }

     data.append("</frame>\n");
     Write(data);
}

void TrajectoriesJPSV04::WriteFooter()
{
     Write("</trajectories>\n");
}

/**
 * FLAT format implementation
 */

TrajectoriesFLAT::TrajectoriesFLAT() : Trajectories()
{
}

static fs::path getSourceFileName(const fs::path& projectFile)
{
     fs::path ret{};

     TiXmlDocument doc(projectFile.string());
     if (!doc.LoadFile()) {
          Log->Write("ERROR: \t%s", doc.ErrorDesc());
          Log->Write("ERROR: \tGetSourceFileName could not parse the project file");
          return ret;
     }
     TiXmlNode* xRootNode = doc.RootElement()->FirstChild("agents");
     if (!xRootNode) {
          Log->Write("ERROR:\tGetSourceFileName could not load persons attributes");
          return ret;
     }

    TiXmlNode* xSources = xRootNode->FirstChild("agents_sources");
    if (xSources) {
        TiXmlNode* xFileNode = xSources->FirstChild("file");
        //------- parse sources from external file
        if(xFileNode)
        {
             ret = xFileNode->FirstChild()->ValueStr();
        }
        return ret;
    }
    return ret;
}

static fs::path getEventFileName(const fs::path& projectFile)
{
     fs::path ret{};

     TiXmlDocument doc(projectFile.string());
     if (!doc.LoadFile()) {
          Log->Write("ERROR: \t%s", doc.ErrorDesc());
          Log->Write("ERROR: \tGetEventFileName could not parse the project file");
          return ret;
     }
     TiXmlNode* xMainNode = doc.RootElement();
     std::string eventfile = "";
     if (xMainNode->FirstChild("events_file")) {
          ret = xMainNode->FirstChild("events_file")->FirstChild()->ValueStr();
          Log->Write("INFO: \tevents <" + ret.string() + ">");
     } else {
          Log->Write("INFO: \tNo events found");
          return ret;
     }
     return ret;
}
 // <train_constraints>
 //   <train_time_table>ttt.xml</train_time_table>
 //   <train_types>train_types.xml</train_types>
 // </train_constraints>


static fs::path getTrainTimeTableFileName(const fs::path& projectFile)
{
     fs::path ret{};

     TiXmlDocument doc(projectFile.string());
     if (!doc.LoadFile()) {
          Log->Write("ERROR: \t%s", doc.ErrorDesc());
          Log->Write("ERROR: \tGetTrainTimeTable could not parse the project file");
          return ret;
     }
     TiXmlNode* xMainNode = doc.RootElement();
     std::string tttfile = "";
     if (xMainNode->FirstChild("train_constraints")) {
          TiXmlNode * xFileNode = xMainNode->FirstChild("train_constraints")->FirstChild("train_time_table");

          if(xFileNode)
               ret = xFileNode->FirstChild()->ValueStr();
          Log->Write("INFO: \ttrain_time_table <" + ret.string() + ">");
     } else {
          Log->Write("INFO: \tNo events no ttt file found");
          return ret;
     }
     return ret;
}

static fs::path getTrainTypeFileName(const fs::path& projectFile)
{
     fs::path ret{};

     TiXmlDocument doc(projectFile.string());
     if (!doc.LoadFile()) {
          Log->Write("ERROR: \t%s", doc.ErrorDesc());
          Log->Write("ERROR: \tGetTrainType could not parse the project file");
          return ret;
     }
     TiXmlNode* xMainNode = doc.RootElement();
     std::string tttfile = "";
     if (xMainNode->FirstChild("train_constraints")) {
          auto xFileNode = xMainNode->FirstChild("train_constraints")->FirstChild("train_types");
          if(xFileNode)
               ret = xFileNode->FirstChild()->ValueStr();
          Log->Write("INFO: \ttrain_types <" + ret.string() + ">");
     } else {
          Log->Write("INFO: \tNo events no train types file found");
          return ret;
     }
     return ret;
}

static fs::path getGoalFileName(const fs::path& projectFile)
{
     fs::path ret{};

     TiXmlDocument doc(projectFile.string());
     if (!doc.LoadFile()) {
          Log->Write("ERROR: \t%s", doc.ErrorDesc());
          Log->Write("ERROR: \tGetSourceFileName could not parse the project file");
          return ret;
     }
     TiXmlNode* xRootNode = doc.RootElement();
     if (!xRootNode->FirstChild("routing")) {
          return ret;
     }
     //load goals and routes
     TiXmlNode* xGoalsNode = xRootNode->FirstChild("routing")->FirstChild("goals");
     TiXmlNode* xGoalsNodeFile = xGoalsNode->FirstChild("file");
     if(xGoalsNodeFile)
     {
          ret = xGoalsNodeFile->FirstChild()->ValueStr();
          Log->Write("INFO:\tGoal file <%s> will be parsed", ret.string().c_str());
     }
     return ret;
}

void TrajectoriesFLAT::WriteHeader(long nPeds, double fps, Building* building, int seed, int count)
{
     const fs::path projRoot(building->GetProjectRootDir());

     (void) seed; (void) nPeds;
     char tmp[500] = "";
     sprintf(tmp, "#description: jpscore (%s)", JPSCORE_VERSION);
     Write(tmp);
     sprintf(tmp, "#count: %d", count);
     Write(tmp);
     sprintf(tmp, "#framerate: %0.2f",fps);
     Write(tmp);
     const fs::path tmpGeo = projRoot / building->GetGeometryFilename();
     sprintf(tmp,"#geometry: %s",  tmpGeo.string().c_str());
     Write(tmp);

     if(const fs::path sourceFileName = getSourceFileName(building->GetProjectFilename());
          !sourceFileName.empty())
     {
          const fs::path tmpSource = projRoot / sourceFileName;
          sprintf(tmp,"#sources: %s", tmpSource.string().c_str());
          Write(tmp);
     }

     if(const fs::path goalFileName = getGoalFileName(building->GetProjectFilename());
          !goalFileName.empty())
     {
          const fs::path tmpGoal = projRoot / goalFileName;
          sprintf(tmp,"#goals: %s", tmpGoal.string().c_str());
          Write(tmp);
     }

     if(const fs::path eventFileName = getEventFileName(building->GetProjectFilename());
          !eventFileName.empty())
     {
          const fs::path tmpEvent = projRoot / eventFileName;
          sprintf(tmp,"#events: %s", tmpEvent.string().c_str());
          Write(tmp);
     }

     if(const fs::path  trainTimeTableFileName = getTrainTimeTableFileName(building->GetProjectFilename());
          !trainTimeTableFileName.empty())
     {
          const fs::path tmpTTT = projRoot / trainTimeTableFileName;
          sprintf(tmp,"#trainTimeTable: %s", tmpTTT.string().c_str());
          Write(tmp);
     }

     if(const fs::path  trainTypeFileName = getTrainTypeFileName(building->GetProjectFilename());
          !trainTypeFileName.empty())
     {
          const fs::path tmpTT = projRoot / trainTypeFileName;
          sprintf(tmp,"#trainType: %s", tmpTT.string().c_str());
          Write(tmp);
     }
     Write("#ID: the agent ID");
     Write("#FR: the current frame");
     Write("#X,Y,Z: the agents coordinates (in metres)");
     Write("#A, B: semi-axes of the ellipse");
     Write("#ANGLE: orientation of the ellipse");
     Write("#COLOR: color of the ellipse");

     Write("\n");
     //Write("#ID\tFR\tX\tY\tZ");// @todo: maybe use two different formats
     Write("#ID\tFR\tX\tY\tZ\tA\tB\tANGLE\tCOLOR");// a b angle color
}

void TrajectoriesFLAT::WriteGeometry(Building* building)
{
     (void) building;
}

void TrajectoriesFLAT::WriteFrame(int frameNr, Building* building)
{
     char tmp[CLENGTH] = "";
     const std::vector< Pedestrian* >& allPeds = building->GetAllPedestrians();
     for(unsigned int p=0;p<allPeds.size();p++){
          Pedestrian* ped = allPeds[p];
          double x = ped->GetPos()._x;
          double y = ped->GetPos()._y;
          double z = ped->GetElevation();
          int color=ped->GetColor();
          double a = ped->GetLargerAxis();
          double b = ped->GetSmallerAxis();
          double phi = atan2(ped->GetEllipse().GetSinPhi(), ped->GetEllipse().GetCosPhi());
          double RAD2DEG = 180.0 / M_PI;
          // @todo: maybe two different formats
          //sprintf(tmp, "%d\t%d\t%0.2f\t%0.2f\t%0.2f", ped->GetID(), frameNr, x, y, z);
          sprintf(tmp, "%d\t%d\t%0.2f\t%0.2f\t%0.2f\t%0.2f\t%0.2f\t%0.2f\t%d", ped->GetID(), frameNr, x, y, z, a, b, phi * RAD2DEG, color);
          Write(tmp);
     }
}

void TrajectoriesFLAT::WriteFooter()
{

}
void TrajectoriesFLAT::WriteSources(const std::vector<std::shared_ptr<AgentsSource> > /*sources*/)
{

}
void TrajectoriesVTK::WriteSources(const std::vector<std::shared_ptr<AgentsSource> > /*sources*/)
{

}
void TrajectoriesJPSV06::WriteSources(const std::vector<std::shared_ptr<AgentsSource> > /*sources*/)
{

}
void TrajectoriesXML_MESH::WriteSources(const std::vector<std::shared_ptr<AgentsSource> > /*sources*/)
{

}

/**
 *  VTK Implementation of the geometry and trajectories
 */


TrajectoriesVTK::TrajectoriesVTK()
{
}

void TrajectoriesVTK::WriteHeader(long nPeds, double fps, Building* building, int seed, int count)
{
     //suppress unused warnings
     (void) nPeds; (void) fps ; (void) seed; (void) count;

     Write("# vtk DataFile Version 4.0");
     Write(building->GetCaption());
     Write("ASCII");
     Write("");
}

void TrajectoriesVTK::WriteGeometry(Building* building)
{
    (void)building; // avoid warning
    Log->Write("WARNING:\t Creating NavMesh is deprecated. Please have a look at old files on git lab if you want to use this!");
//     stringstream tmp;

//     NavMesh* nv= new NavMesh(building);
//     nv->BuildNavMesh();
//     //nv->WriteToFile("../pedunc/examples/stadium/arena.nav");
//     Write("DATASET UNSTRUCTURED_GRID");

//     //writing the vertices
//     const vector<NavMesh::JVertex*>& vertices= nv->GetVertices() ;
//     tmp<<"POINTS "<<vertices.size()<<" FLOAT"<<endl;
//     for (unsigned int v=0; v<vertices.size(); v++) {
//          tmp<<vertices[v]->pPos._x<<" " <<vertices[v]->pPos._y <<" 0.0"<<endl;
//     }
//     Write(tmp.str());
//     tmp.str(std::string());

//     //writing the cells data
//     const vector<NavMesh::JNode*>& cells= nv->GetNodes();
//     int nComponents= (int) cells.size();
//     stringstream tmp1;
//     for (unsigned int n=0; n<cells.size(); n++) {
//          int hSize= (int) cells[n]->pHull.size();

//          tmp1<<hSize<<"";
//          for(unsigned int i=0; i<cells[n]->pHull.size(); i++) {
//               tmp1<<" "<<cells[n]->pHull[i].id;
//          }
//          tmp1<<endl;
//          nComponents+= hSize;
//     }
//     tmp<<"CELLS "<<cells.size()<<" "<<nComponents<<endl;
//     tmp<<tmp1.str();
//     Write(tmp.str());
//     tmp.str(std::string());

//     // writing the cell type
//     tmp<<"CELL_TYPES "<<cells.size()<<endl;
//     for (unsigned int n=0; n<cells.size(); n++) {
//          tmp<<"9"<<endl;
//     }

//     Write(tmp.str());
//     delete nv;
}

void TrajectoriesVTK::WriteFrame(int frameNr, Building* building)
{
     (void) frameNr; (void)building;
}

void TrajectoriesVTK::WriteFooter()
{
}


void TrajectoriesJPSV06::WriteHeader(long nPeds, double fps, Building* building, int seed, int /*count*/)
{
     building->GetCaption();
     std::string tmp;
     tmp =
               "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\n" "<trajectories>\n";
     char agents[CLENGTH] = "";
     sprintf(agents, "\t<header version = \"0.6\">\n");
     tmp.append(agents);
     sprintf(agents, "\t\t<agents>%ld</agents>\n", nPeds);
     tmp.append(agents);
     sprintf(agents, "\t\t<seed>%d</seed>\n", seed);
     tmp.append(agents);
     sprintf(agents, "\t\t<frameRate>%0.2f</frameRate>\n", fps);
     tmp.append(agents);
     tmp.append("\t\t<!-- Frame count HACK\n");
     tmp.append("replace me\n");
     tmp.append("\t\tFrame count HACK -->\n");
     //tmp.append("<frameCount>xxxxxxx</frameCount>\n");
     tmp.append("\t</header>\n");
     Write(tmp);

}

void TrajectoriesJPSV06::WriteGeometry(Building* building)
{
     // just put a link to the geometry file
     //     string embed_geometry;
     //     embed_geometry.append("\t<geometry>\n");
     //     char file_location[CLENGTH] = "";
     //     sprintf(file_location, "\t<file location= \"%s\"/>\n", building->GetGeometryFilename().c_str());
     //     embed_geometry.append(file_location);
     //     //embed_geometry.append("\t</geometry>\n");
     //
     //     const map<int, Hline*>& hlines=building->GetAllHlines();
     //     if(hlines.size()>0){
     //          //embed_geometry.append("\t<geometry>\n");
     //          for (std::map<int, Hline*>::const_iterator it=hlines.begin(); it!=hlines.end(); ++it)
     //          {
     //               embed_geometry.append(it->second->WriteElement());
     //          }
     //          //embed_geometry.append("\t</geometry>\n");
     //     }
     //     embed_geometry.append("\t</geometry>\n");
     //     Write(embed_geometry);

     //set the content of the file
     const fs::path fileName =
          building->GetProjectRootDir() / building->GetGeometryFilename();
     std::ifstream t(fileName.string());
     std::string dropedLine;
     std::getline(t, dropedLine); //drop the first line <?xml version="1.0" encoding="UTF-8"?>
     std::stringstream buffer;
     buffer << t.rdbuf();
     std::string embed_geometry = buffer.str();

     //write the hlines
     std::string embed_hlines;
     embed_hlines.append("\n\t<hlines>");
     for (const auto& hline: building->GetAllHlines()) {
          embed_hlines.append(hline.second->GetDescription());
     }
     embed_hlines.append("\n\t</hlines>");
     embed_hlines.append("\n\t<goals>");
     for (const auto& goal: building->GetAllGoals()) {
          embed_hlines.append(goal.second->Write());
     }
     embed_hlines.append("\n\t</goals>");
     embed_hlines.append("\t</geometry>\n");

     //append the new string hlines and goals to the old one
     ReplaceStringInPlace(embed_geometry,"</geometry>",embed_hlines);

     Write(embed_geometry);
}

void TrajectoriesJPSV06::WriteFrame(int frameNr, Building* building)
{
     std::string data;
     char tmp[CLENGTH] = "";
     double RAD2DEG = 180.0 / M_PI;
     std::vector<std::string> rooms_to_plot;

     sprintf(tmp, "<frame ID=\"%d\">\n", frameNr);
     data.append(tmp);


     const std::vector< Pedestrian* >& allPeds = building->GetAllPedestrians();
     for(unsigned int p=0;p<allPeds.size();++p)
     {
          Pedestrian* ped = allPeds[p];
          Room* r = building->GetRoom(ped->GetRoomID());
          std::string caption = r->GetCaption();

          if (!IsElementInVector(rooms_to_plot, caption)) {
               if (!rooms_to_plot.empty()) {
                    continue;
               }
          }

          char tmp1[CLENGTH] = "";

          int color=ped->GetColor();
          double a = ped->GetLargerAxis();
          double b = ped->GetSmallerAxis();
          double phi = atan2(ped->GetEllipse().GetSinPhi(), ped->GetEllipse().GetCosPhi());
          sprintf(tmp1, "<agent ID=\"%d\"\t"
                    "x=\"%.6f\"\ty=\"%.6f\"\t"
                    "z=\"%.6f\"\t"
                    "rA=\"%.2f\"\trB=\"%.2f\"\t"
                    "eO=\"%.2f\" eC=\"%d\"/>\n",
                    ped->GetID(), (ped->GetPos()._x) * FAKTOR,
                    (ped->GetPos()._y) * FAKTOR,(ped->GetElevation()) * FAKTOR ,a * FAKTOR, b * FAKTOR,
                    phi * RAD2DEG, color);
          data.append(tmp1);

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
    (void)building; // avoid warning
    Log->Write("WARNING:\t Creating NavMesh is deprecated. Please have a look at old files on git lab if you want to use this!");
//     //Navigation mesh implementation
//     NavMesh* nv= new NavMesh(building);
//     nv->BuildNavMesh();
//     string geometry;
//     nv->WriteToString(geometry);
//     Write("<geometry>");
//     Write(geometry);
//     Write("</geometry>");
//     nv->WriteToFile(building->GetProjectFilename()+".full.nav");
//     delete nv;
}


void TrajectoriesJPSV05::WriteHeader(long nPeds, double fps, Building* building, int seed, int /*count*/)
{
     building->GetCaption();
     std::string tmp;
     tmp = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\n" "<trajectories>\n";
     char agents[CLENGTH] = "";
     sprintf(agents, "\t<header version = \"0.5.1\">\n");
     tmp.append(agents);
     sprintf(agents, "\t\t<agents>%ld</agents>\n", nPeds);
     tmp.append(agents);
     sprintf(agents, "\t\t<seed>%d</seed>\n", seed);
     tmp.append(agents);
     sprintf(agents, "\t\t<frameRate>%0.2f</frameRate>\n", fps);
     tmp.append(agents);
     //tmp.append("\t\t<!-- Frame count HACK\n");
     //tmp.append("replace me\n");
     //tmp.append("\t\tFrame count HACK -->\n");
     //tmp.append("<frameCount>xxxxxxx</frameCount>\n");
     tmp.append("\t</header>\n");
     _outputHandler->Write(tmp);
}
void TrajectoriesJPSV05::WriteSources(const std::vector<std::shared_ptr<AgentsSource> > sources)
{
     std::string tmp("");

     for (const auto& src: sources) {
          auto BB =  src->GetBoundaries();
          tmp += "<source  id=\"" + std::to_string(src->GetId()) +
               "\"  caption=\"" + src->GetCaption() + "\"" +
               "  x_min=\"" + std::to_string(BB[0]) + "\"" +
               "  x_max=\"" + std::to_string(BB[1]) + "\"" +
               "  y_min=\"" + std::to_string(BB[2]) + "\"" +
               "  y_max=\"" + std::to_string(BB[3]) + "\" />\n";
    }
     _outputHandler->Write(tmp);
}
void TrajectoriesJPSV05::WriteGeometry(Building* building)
{
     // just put a link to the geometry file
     std::string embed_geometry;
     embed_geometry.append("\t<geometry>\n");
     char file_location[CLENGTH] = "";
     sprintf(file_location, "\t<file location= \"%s\"/>\n", building->GetGeometryFilename().string().c_str());
     embed_geometry.append(file_location);
     //embed_geometry.append("\t</geometry>\n");

     for (auto hline: building->GetAllHlines())
     {
          embed_geometry.append(hline.second->GetDescription());
     }

     for (auto goal: building->GetAllGoals()) {
          embed_geometry.append(goal.second->Write());
     }

     //write the grid
     //embed_geometry.append(building->GetGrid()->ToXML());

     embed_geometry.append("\t</geometry>\n");
     _outputHandler->Write(embed_geometry);
     //write sources
    // if(building->G )
     //
     _outputHandler->Write("\t<AttributeDescription>");
     _outputHandler->Write("\t\t<property tag=\"x\" description=\"xPosition\"/>");
     _outputHandler->Write("\t\t<property tag=\"y\" description=\"yPosition\"/>");
     _outputHandler->Write("\t\t<property tag=\"z\" description=\"zPosition\"/>");
     _outputHandler->Write("\t\t<property tag=\"rA\" description=\"radiusA\"/>");
     _outputHandler->Write("\t\t<property tag=\"rB\" description=\"radiusB\"/>");
     _outputHandler->Write("\t\t<property tag=\"eC\" description=\"ellipseColor\"/>");
     _outputHandler->Write("\t\t<property tag=\"eO\" description=\"ellipseOrientation\"/>");
     _outputHandler->Write("\t</AttributeDescription>\n");
}

void TrajectoriesJPSV05::WriteFrame(int frameNr, Building* building)
{
     std::string data;
     char tmp[CLENGTH] = "";
     double RAD2DEG = 180.0 / M_PI;

     sprintf(tmp, "<frame ID=\"%d\">\n", frameNr);
     data.append(tmp);

     const std::vector< Pedestrian* >& allPeds = building->GetAllPedestrians();
     for(unsigned int p=0;p<allPeds.size();p++)
     {
          Pedestrian* ped = allPeds[p];
          Room* r = building->GetRoom(ped->GetRoomID());
          std::string caption = r->GetCaption();
          char s[CLENGTH] = "";
          int color=ped->GetColor();
          double a = ped->GetLargerAxis();
          double b = ped->GetSmallerAxis();
          double phi = atan2(ped->GetEllipse().GetSinPhi(), ped->GetEllipse().GetCosPhi());
          sprintf(s, "<agent ID=\"%d\"\t"
                    "x=\"%.6f\"\ty=\"%.6f\"\t"
                    "z=\"%.6f\"\t"
                    "rA=\"%.2f\"\trB=\"%.2f\"\t"
                    "eO=\"%.2f\" eC=\"%d\"/>\n",
                    ped->GetID(), (ped->GetPos()._x) * FAKTOR,
                    (ped->GetPos()._y) * FAKTOR,(ped->GetElevation()) * FAKTOR ,a * FAKTOR, b * FAKTOR,
                    phi * RAD2DEG, color);
          data.append(s);
     }
     data.append("</frame>\n");
     _outputHandler->Write(data);
}

void TrajectoriesJPSV05::WriteFooter()
{
     _outputHandler->Write("</trajectories>\n");
}
