#include "Trajectories.h"

#include "general/Format.h"
#include "general/Logger.h"
#include "geometry/SubRoom.h"
#include "mpi/LCGrid.h"
#include "pedestrian/Pedestrian.h"

#include <tinyxml.h>

static fs::path getSourceFileName(const fs::path & projectFile)
{
    fs::path ret{};

    TiXmlDocument doc(projectFile.string());
    if(!doc.LoadFile()) {
        Logging::Error(doc.ErrorDesc());
        Logging::Error("GetSourceFileName could not parse the project file");
        return ret;
    }
    TiXmlNode * xRootNode = doc.RootElement()->FirstChild("agents");
    if(!xRootNode) {
        Logging::Error("GetSourceFileName could not load persons attributes");
        return ret;
    }

    TiXmlNode * xSources = xRootNode->FirstChild("agents_sources");
    if(xSources) {
        TiXmlNode * xFileNode = xSources->FirstChild("file");
        //------- parse sources from external file
        if(xFileNode) {
            ret = xFileNode->FirstChild()->ValueStr();
        }
        return ret;
    }
    return ret;
}

static fs::path getEventFileName(const fs::path & projectFile)
{
    fs::path ret{};

    TiXmlDocument doc(projectFile.string());
    if(!doc.LoadFile()) {
        Logging::Error(doc.ErrorDesc());
        Logging::Error("GetEventFileName could not parse the project file");
        return ret;
    }
    TiXmlNode * xMainNode = doc.RootElement();
    if(xMainNode->FirstChild("events_file")) {
        ret = xMainNode->FirstChild("events_file")->FirstChild()->ValueStr();
        Logging::Info(fmt::format("events <{}>", ret.string()));
    } else {
        Logging::Info("No events found");
        return ret;
    }
    return ret;
}
// <train_constraints>
//   <train_time_table>ttt.xml</train_time_table>
//   <train_types>train_types.xml</train_types>
// </train_constraints>


static fs::path getTrainTimeTableFileName(const fs::path & projectFile)
{
    fs::path ret{};

    TiXmlDocument doc(projectFile.string());
    if(!doc.LoadFile()) {
        Logging::Error(doc.ErrorDesc());
        Logging::Error("GetTrainTimeTable could not parse the project file");
        return ret;
    }
    TiXmlNode * xMainNode = doc.RootElement();
    if(xMainNode->FirstChild("train_constraints")) {
        TiXmlNode * xFileNode =
            xMainNode->FirstChild("train_constraints")->FirstChild("train_time_table");

        if(xFileNode)
            ret = xFileNode->FirstChild()->ValueStr();
        Logging::Info(fmt::format("train_time_table <{}>", ret.string()));
    } else {
        Logging::Info("No ttt file found");
        return ret;
    }
    return ret;
}

static fs::path getTrainTypeFileName(const fs::path & projectFile)
{
    fs::path ret{};

    TiXmlDocument doc(projectFile.string());
    if(!doc.LoadFile()) {
        Logging::Error(doc.ErrorDesc());
        Logging::Error("GetTrainType could not parse the project file");
        return ret;
    }
    TiXmlNode * xMainNode = doc.RootElement();
    if(xMainNode->FirstChild("train_constraints")) {
        auto xFileNode = xMainNode->FirstChild("train_constraints")->FirstChild("train_types");
        if(xFileNode)
            ret = xFileNode->FirstChild()->ValueStr();
        Logging::Info(fmt::format("train_types <{}>", ret.string()));
    } else {
        Logging::Info("No train types file found");
        return ret;
    }
    return ret;
}

static fs::path getGoalFileName(const fs::path & projectFile)
{
    fs::path ret{};

    TiXmlDocument doc(projectFile.string());
    if(!doc.LoadFile()) {
        Logging::Error(doc.ErrorDesc());
        Logging::Error("GetGoalFileName could not parse the project file");
        return ret;
    }
    TiXmlNode * xRootNode = doc.RootElement();
    if(!xRootNode->FirstChild("routing")) {
        return ret;
    }
    //load goals and routes
    TiXmlNode * xGoalsNode     = xRootNode->FirstChild("routing")->FirstChild("goals");
    TiXmlNode * xGoalsNodeFile = xGoalsNode->FirstChild("file");
    if(xGoalsNodeFile) {
        ret = xGoalsNodeFile->FirstChild()->ValueStr();
        Logging::Info(fmt::format("goal file <{}>", ret.string()));
    }
    return ret;
}

/**
 * TXT format implementation
 */
TrajectoriesTXT::TrajectoriesTXT() : Trajectories()
{
    // Add header, info and output for speed
    _optionalOutputHeader[OptionalOutput::speed] = "V\t";
    _optionalOutputInfo[OptionalOutput::speed]   = "#V: speed of the pedestrian (in m/s)\n";
    _optionalOutput[OptionalOutput::speed]       = [](const Pedestrian * ped) {
        return fmt::format(check_fmt("{:.2f}\t"), ped->GetV().Norm());
    };

    // Add header, info and output for velocity
    _optionalOutputHeader[OptionalOutput::velocity] = "Vx\tVy\t";
    _optionalOutputInfo[OptionalOutput::velocity] =
        "#Vx: x component of the pedestrian's velocity\n"
        "#Vy: y component of the pedestrian's velocity\n";
    _optionalOutput[OptionalOutput::velocity] = [](const Pedestrian * ped) {
        return fmt::format(check_fmt("{:.2f}\t{:.2f}\t"), ped->GetV()._x, ped->GetV()._y);
    };

    // Add header, info and output for final_goal
    _optionalOutputHeader[OptionalOutput::final_goal] = "FG\t";
    _optionalOutputInfo[OptionalOutput::final_goal]   = "#FG: id of final goal\n";
    _optionalOutput[OptionalOutput::final_goal]       = [](const Pedestrian * ped) {
        return fmt::format(check_fmt("{}\t"), ped->GetFinalDestination());
    };

    // Add header, info and output for intermediate_goal
    _optionalOutputHeader[OptionalOutput::intermediate_goal] = "CG\t";
    _optionalOutputInfo[OptionalOutput::intermediate_goal]   = "#CG: id of current goal\n";
    _optionalOutput[OptionalOutput::intermediate_goal]       = [](const Pedestrian * ped) {
        return fmt::format(check_fmt("{}\t"), ped->GetExitIndex());
    };

    // Add header, info and output for desired direction
    _optionalOutputHeader[OptionalOutput::desired_direction] = "Dx\tDy\t";
    _optionalOutputInfo[OptionalOutput::desired_direction] =
        "#Dx: x component of the pedestrian's desired direction\n"
        "#Dy: y component of the pedestrian's desired direction\n";
    _optionalOutput[OptionalOutput::desired_direction] = [](const Pedestrian * ped) {
        return fmt::format(check_fmt("{:.2f}\t{:.2f}\t"), ped->GetLastE0()._x, ped->GetLastE0()._y);
    };

    // Add header, info and output for spotlight
    _optionalOutputHeader[OptionalOutput::spotlight] = "SPOT\t";
    _optionalOutputInfo[OptionalOutput::spotlight]   = "#SPOT: ped is highlighted\n";
    _optionalOutput[OptionalOutput::spotlight]       = [](Pedestrian * ped) {
        return fmt::format(check_fmt("{}\t"), (int) ped->GetSpotlight());
    };

    // Add header, info and output for router
    _optionalOutputHeader[OptionalOutput::router] = "ROUTER\t";
    _optionalOutputInfo[OptionalOutput::router] =
        "#ROUTER: routing strategy used during simulation\n";
    _optionalOutput[OptionalOutput::router] = [](const Pedestrian * ped) {
        return fmt::format(check_fmt("{}\t"), ped->GetRoutingStrategy());
    };

    // Add header, info and output for group
    _optionalOutputHeader[OptionalOutput::group] = "GROUP\t";
    _optionalOutputInfo[OptionalOutput::group]   = "#GROUP: group of the pedestrian\n";
    _optionalOutput[OptionalOutput::group]       = [](const Pedestrian * ped) {
        return fmt::format(check_fmt("{}\t"), ped->GetGroup());
    };
}

void TrajectoriesTXT::WriteHeader(long, double fps, Building * building, int, int count)
{
    const fs::path & projRoot(building->GetProjectRootDir());
    const fs::path tmpGeo = projRoot / building->GetGeometryFilename();

    std::string header = fmt::format("#description: jpscore ({:s})\n", JPSCORE_VERSION);
    header.append(fmt::format("#count: {:d}\n", count));
    header.append(fmt::format("#framerate: {:0.2f}\n", fps));
    header.append(fmt::format("#geometry: {:s}\n", tmpGeo.string()));

    // if used: add source file name
    if(const fs::path sourceFileName = getSourceFileName(building->GetProjectFilename());
       !sourceFileName.empty()) {
        const fs::path tmpSource = projRoot / sourceFileName;
        header.append(fmt::format("#sources: {:s}\n", tmpSource.string()));
    }

    // if used: add goal file name
    if(const fs::path goalFileName = getGoalFileName(building->GetProjectFilename());
       !goalFileName.empty()) {
        const fs::path tmpGoal = projRoot / goalFileName;
        header.append(fmt::format("#goals: {:s}\n", tmpGoal.string()));
    }

    // if used: add event file name
    if(const fs::path eventFileName = getEventFileName(building->GetProjectFilename());
       !eventFileName.empty()) {
        const fs::path tmpEvent = projRoot / eventFileName;
        header.append(fmt::format("#events: {:s}\n", tmpEvent.string()));
    }

    // if used: add trainTimeTable file name
    if(const fs::path trainTimeTableFileName =
           getTrainTimeTableFileName(building->GetProjectFilename());
       !trainTimeTableFileName.empty()) {
        const fs::path tmpTTT = projRoot / trainTimeTableFileName;
        header.append(fmt::format("#trainTimeTable: {:s}\n", tmpTTT.string()));
    }

    // if used: add trainType file name
    if(const fs::path trainTypeFileName = getTrainTypeFileName(building->GetProjectFilename());
       !trainTypeFileName.empty()) {
        const fs::path tmpTT = projRoot / trainTypeFileName;
        header.append(fmt::format("#trainType: {:s}\n", tmpTT.string()));
    }

    header.append("#ID: the agent ID\n");
    header.append("#FR: the current frame\n");
    header.append("#X,Y,Z: the agents coordinates (in metres)\n");
    header.append("#A, B: semi-axes of the ellipse\n");
    header.append("#ANGLE: orientation of the ellipse\n");
    header.append("#COLOR: color of the ellipse\n");

    // Add info for optional output options
    for(const auto & option : _optionalOutputOptions) {
        header.append(_optionalOutputInfo[option]);
    }

    header.append("\n#ID\tFR\tX\tY\tZ\tA\tB\tANGLE\tCOLOR\t");
    // Add header for optional output options
    for(const auto & option : _optionalOutputOptions) {
        header.append(_optionalOutputHeader[option]);
    }
    Write(header);
}

void TrajectoriesTXT::WriteGeometry(Building *) {}

void TrajectoriesTXT::WriteFrame(int frameNr, Building * building)
{
    const std::vector<Pedestrian *> & allPeds = building->GetAllPedestrians();
    for(auto ped : allPeds) {
        double x       = ped->GetPos()._x;
        double y       = ped->GetPos()._y;
        double z       = ped->GetElevation();
        int color      = ped->GetColor();
        double a       = ped->GetLargerAxis();
        double b       = ped->GetSmallerAxis();
        double phi     = atan2(ped->GetEllipse().GetSinPhi(), ped->GetEllipse().GetCosPhi());
        double RAD2DEG = 180.0 / M_PI;

        std::string frame = fmt::format(
            "{:d}\t{:d}\t{:0.2f}\t{:0.2f}\t{:0.2f}\t{:0.2f}\t{:0.2f}\t{:0.2f}\t{:d}\t",
            ped->GetID(),
            frameNr,
            x,
            y,
            z,
            a,
            b,
            phi * RAD2DEG,
            color);

        for(const auto & option : _optionalOutputOptions) {
            frame.append(_optionalOutput[option](ped));
        }

        Write(frame);
    }
}

void TrajectoriesXML::WriteHeader(
    long nPeds,
    double fps,
    Building * building,
    int seed,
    int /*count*/)
{
    building->GetCaption();
    std::string tmp;
    tmp = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\n"
          "<trajectories>\n";
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

void TrajectoriesXML::WriteSources(const std::vector<std::shared_ptr<AgentsSource>> & sources)
{
    std::string tmp("");

    for(const auto & src : sources) {
        auto BB = src->GetBoundaries();
        tmp += "<source  id=\"" + std::to_string(src->GetId()) + "\"  caption=\"" +
               src->GetCaption() + "\"" + "  x_min=\"" + std::to_string(BB[0]) + "\"" +
               "  x_max=\"" + std::to_string(BB[1]) + "\"" + "  y_min=\"" + std::to_string(BB[2]) +
               "\"" + "  y_max=\"" + std::to_string(BB[3]) + "\" />\n";
    }
    _outputHandler->Write(tmp);
}

void TrajectoriesXML::WriteGeometry(Building * building)
{
    // just put a link to the geometry file
    std::string embed_geometry;
    embed_geometry.append("\t<geometry>\n");
    char file_location[CLENGTH] = "";
    sprintf(
        file_location,
        "\t<file location= \"%s\"/>\n",
        building->GetGeometryFilename().string().c_str());
    embed_geometry.append(file_location);
    //embed_geometry.append("\t</geometry>\n");

    for(auto hline : building->GetAllHlines()) {
        embed_geometry.append(hline.second->GetDescription());
    }

    for(auto goal : building->GetAllGoals()) {
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

void TrajectoriesXML::WriteFrame(int frameNr, Building * building)
{
    std::string data;
    char tmp[CLENGTH] = "";
    double RAD2DEG    = 180.0 / M_PI;

    sprintf(tmp, "<frame ID=\"%d\">\n", frameNr);
    data.append(tmp);

    const std::vector<Pedestrian *> & allPeds = building->GetAllPedestrians();
    for(unsigned int p = 0; p < allPeds.size(); p++) {
        Pedestrian * ped    = allPeds[p];
        Room * r            = building->GetRoom(ped->GetRoomID());
        std::string caption = r->GetCaption();
        char s[CLENGTH]     = "";
        int color           = ped->GetColor();
        double a            = ped->GetLargerAxis();
        double b            = ped->GetSmallerAxis();
        double phi          = atan2(ped->GetEllipse().GetSinPhi(), ped->GetEllipse().GetCosPhi());
        sprintf(
            s,
            "<agent ID=\"%d\"\t"
            "x=\"%.6f\"\ty=\"%.6f\"\t"
            "z=\"%.6f\"\t"
            "rA=\"%.2f\"\trB=\"%.2f\"\t"
            "eO=\"%.2f\" eC=\"%d\"/>\n",
            ped->GetID(),
            (ped->GetPos()._x) * FAKTOR,
            (ped->GetPos()._y) * FAKTOR,
            (ped->GetElevation()) * FAKTOR,
            a * FAKTOR,
            b * FAKTOR,
            phi * RAD2DEG,
            color);
        data.append(s);
    }
    data.append("</frame>\n");
    _outputHandler->Write(data);
}

void TrajectoriesXML::WriteFooter()
{
    _outputHandler->Write("</trajectories>\n");
}
