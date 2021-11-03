#include "Trajectories.h"

#include "IO/OutputHandler.h"
#include "general/Macros.h"

#include <fmt/format.h>

TrajectoryWriter::TrajectoryWriter(
    unsigned int precision,
    std::set<OptionalOutput> options,
    std::unique_ptr<OutputHandler> outputHandler) :
    _precision(precision), _options(std::move(options)), _outputHandler(std::move(outputHandler))
{
    // Add header, info and output for speed
    _optionalOutputHeader[OptionalOutput::speed] = "V\t";
    _optionalOutputInfo[OptionalOutput::speed]   = "#V: speed of the pedestrian (in m/s)\n";
    _optionalOutput[OptionalOutput::speed]       = [](const Pedestrian * ped) {
        return fmt::format(FMT_STRING("{:.2f}\t"), ped->GetV().Norm());
    };

    // Add header, info and output for velocity
    _optionalOutputHeader[OptionalOutput::velocity] = "Vx\tVy\t";
    _optionalOutputInfo[OptionalOutput::velocity] =
        "#Vx: x component of the pedestrian's velocity\n"
        "#Vy: y component of the pedestrian's velocity\n";
    _optionalOutput[OptionalOutput::velocity] = [](const Pedestrian * ped) {
        return fmt::format(FMT_STRING("{:.2f}\t{:.2f}\t"), ped->GetV()._x, ped->GetV()._y);
    };

    // Add header, info and output for final_goal
    _optionalOutputHeader[OptionalOutput::final_goal] = "FG\t";
    _optionalOutputInfo[OptionalOutput::final_goal]   = "#FG: id of final goal\n";
    _optionalOutput[OptionalOutput::final_goal]       = [](const Pedestrian * ped) {
        return fmt::format(FMT_STRING("{}\t"), ped->GetFinalDestination());
    };

    // Add header, info and output for intermediate_goal
    _optionalOutputHeader[OptionalOutput::intermediate_goal] = "CG\t";
    _optionalOutputInfo[OptionalOutput::intermediate_goal]   = "#CG: id of current goal\n";
    _optionalOutput[OptionalOutput::intermediate_goal]       = [](const Pedestrian * ped) {
        return fmt::format(FMT_STRING("{}\t"), ped->GetExitIndex());
    };

    // Add header, info and output for desired direction
    _optionalOutputHeader[OptionalOutput::desired_direction] = "Dx\tDy\t";
    _optionalOutputInfo[OptionalOutput::desired_direction] =
        "#Dx: x component of the pedestrian's desired direction\n"
        "#Dy: y component of the pedestrian's desired direction\n";
    _optionalOutput[OptionalOutput::desired_direction] = [](const Pedestrian * ped) {
        return fmt::format(
            FMT_STRING("{:.2f}\t{:.2f}\t"), ped->GetLastE0()._x, ped->GetLastE0()._y);
    };

    // Add header, info and output for spotlight
    _optionalOutputHeader[OptionalOutput::spotlight] = "SPOT\t";
    _optionalOutputInfo[OptionalOutput::spotlight]   = "#SPOT: ped is highlighted\n";
    _optionalOutput[OptionalOutput::spotlight]       = [](Pedestrian * ped) {
        return fmt::format(FMT_STRING("{}\t"), static_cast<int>(ped->GetSpotlight()));
    };

    // Add header, info and output for router
    _optionalOutputHeader[OptionalOutput::router] = "ROUTER\t";
    _optionalOutputInfo[OptionalOutput::router] =
        "#ROUTER: routing strategy used during simulation\n";
    _optionalOutput[OptionalOutput::router] = [](const Pedestrian * ped) {
        return fmt::format(FMT_STRING("{}\t"), ped->GetRoutingStrategy());
    };

    // Add header, info and output for group
    _optionalOutputHeader[OptionalOutput::group] = "GROUP\t";
    _optionalOutputInfo[OptionalOutput::group]   = "#GROUP: group of the pedestrian\n";
    _optionalOutput[OptionalOutput::group]       = [](const Pedestrian * ped) {
        return fmt::format(FMT_STRING("{}\t"), ped->GetGroup());
    };
}

void TrajectoryWriter::WriteHeader(size_t nPeds, double fps, const Configuration & cfg, int count)
{
    std::string header = fmt::format("#description: jpscore ({:s})\n", JPSCORE_VERSION);
    header.append(fmt::format("#agents: {:d}\n", nPeds));
    header.append(fmt::format("#count: {:d}\n", count));
    header.append(fmt::format("#framerate: {:0.2f}\n", fps));
    header.append(fmt::format("#geometry: {:s}\n", cfg.GetGeometryFile().filename().string()));

    // if used: add two necessary files time table and train types
    if(!cfg.GetTrainTimeTableFile().empty() && !cfg.GetTrainTypeFile().empty()) {
        header.append(fmt::format(
            "#trainTimeTable: {:s}\n", cfg.GetTrainTimeTableFile().filename().string()));
        header.append(
            fmt::format("#trainType: {:s}\n", cfg.GetTrainTypeFile().filename().string()));
    }
    // if used: add source file name
    if(!cfg.GetSourceFile().empty()) {
        header.append(fmt::format("#sources: {:s}\n", cfg.GetSourceFile().filename().string()));
    }

    // if used: add goal file name
    if(!cfg.GetGoalFile().empty()) {
        header.append(fmt::format("#goals: {:s}\n", cfg.GetGoalFile().filename().string()));
    }

    // if used: add event file name
    if(!cfg.GetEventFile().empty()) {
        header.append(fmt::format("#events: {:s}\n", cfg.GetEventFile().filename().string()));
    }

    header.append("#ID: the agent ID\n");
    header.append("#FR: the current frame\n");
    header.append("#X,Y,Z: the agents coordinates (in metres)\n");
    header.append("#A, B: semi-axes of the ellipse\n");
    header.append("#ANGLE: orientation of the ellipse\n");
    header.append("#COLOR: color of the ellipse\n");

    // Add info for optional output options
    for(const auto & option : _options) {
        header.append(_optionalOutputInfo[option]);
    }

    header.append("\n#ID\tFR\tX\tY\tZ\tA\tB\tANGLE\tCOLOR\t");
    // Add header for optional output options
    for(const auto & option : _options) {
        header.append(_optionalOutputHeader[option]);
    }
    _outputHandler->Write(header);
}

void TrajectoryWriter::WriteFrame(
    int frameNr,
    const std::vector<std::unique_ptr<Pedestrian>> & pedestrian)
{
    for(const auto & ped : pedestrian) {
        double x          = ped->GetPos()._x;
        double y          = ped->GetPos()._y;
        double z          = ped->GetElevation();
        int color         = ped->GetColor();
        double a          = ped->GetLargerAxis();
        double b          = ped->GetSmallerAxis();
        double phi        = atan2(ped->GetEllipse().GetSinPhi(), ped->GetEllipse().GetCosPhi());
        double RAD2DEG    = 180.0 / M_PI;
        std::string frame = fmt::format(
            "{:d}\t{:d}\t{:0.{}f}\t{:0.{}f}\t{:0.{}f}\t{:0.2f}\t{:0.2f}\t{:0.2f}\t{:d}\t",
            ped->GetID(),
            frameNr,
            x,
            _precision,
            y,
            _precision,
            z,
            _precision,
            a,
            b,
            phi * RAD2DEG,
            color);
        for(const auto & option : _options) {
            frame.append(_optionalOutput[option](ped.get()));
        }

        _outputHandler->Write(frame);
    }
}
