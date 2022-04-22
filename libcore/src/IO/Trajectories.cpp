#include "Trajectories.hpp"

#include "IO/OutputHandler.hpp"
#include "general/Macros.hpp"

#include <fmt/format.h>

TrajectoryWriter::TrajectoryWriter(
    unsigned int precision,
    std::set<OptionalOutput> options,
    std::unique_ptr<OutputHandler> outputHandler,
    AgentColorMode colorMode)
    : _precision(precision)
    , _options(std::move(options))
    , _outputHandler(std::move(outputHandler))
    , _colorMode(colorMode)
{
    // Add header, info and output for speed
    _optionalOutputHeader[OptionalOutput::speed] = "V\t";
    _optionalOutputInfo[OptionalOutput::speed] = "#V: speed of the pedestrian (in m/s)\n";
    _optionalOutput[OptionalOutput::speed] = [](const Pedestrian* ped) {
        return fmt::format(FMT_STRING("{:.2f}\t"), ped->GetV().Norm());
    };

    // Add header, info and output for velocity
    _optionalOutputHeader[OptionalOutput::velocity] = "Vx\tVy\t";
    _optionalOutputInfo[OptionalOutput::velocity] =
        "#Vx: x component of the pedestrian's velocity\n"
        "#Vy: y component of the pedestrian's velocity\n";
    _optionalOutput[OptionalOutput::velocity] = [](const Pedestrian* ped) {
        return fmt::format(FMT_STRING("{:.2f}\t{:.2f}\t"), ped->GetV().x, ped->GetV().y);
    };

    // Add header, info and output for final_goal
    _optionalOutputHeader[OptionalOutput::final_goal] = "FG\t";
    _optionalOutputInfo[OptionalOutput::final_goal] = "#FG: id of final goal\n";
    _optionalOutput[OptionalOutput::final_goal] = [](const Pedestrian* ped) {
        return fmt::format(FMT_STRING("{}\t"), ped->GetFinalDestination());
    };

    // Add header, info and output for intermediate_goal
    _optionalOutputHeader[OptionalOutput::intermediate_goal] = "CG\t";
    _optionalOutputInfo[OptionalOutput::intermediate_goal] = "#CG: id of current goal\n";
    _optionalOutput[OptionalOutput::intermediate_goal] = [](const Pedestrian* ped) {
        return fmt::format(FMT_STRING("{}\t"), ped->GetDestination());
    };

    // Add header, info and output for desired direction
    _optionalOutputHeader[OptionalOutput::desired_direction] = "Dx\tDy\t";
    _optionalOutputInfo[OptionalOutput::desired_direction] =
        "#Dx: x component of the pedestrian's desired direction\n"
        "#Dy: y component of the pedestrian's desired direction\n";
    _optionalOutput[OptionalOutput::desired_direction] = [](const Pedestrian* ped) {
        return fmt::format(FMT_STRING("{:.2f}\t{:.2f}\t"), ped->GetLastE0().x, ped->GetLastE0().y);
    };

    // Add header, info and output for group
    _optionalOutputHeader[OptionalOutput::group] = "GROUP\t";
    _optionalOutputInfo[OptionalOutput::group] = "#GROUP: group of the pedestrian\n";
    _optionalOutput[OptionalOutput::group] = [](const Pedestrian* ped) {
        return fmt::format(FMT_STRING("{}\t"), ped->GetGroup());
    };
}

void TrajectoryWriter::WriteHeader(size_t nPeds, double fps, const Configuration& cfg, int count)
{
    std::string header = fmt::format("#description: jpscore ({:s})\n", JPSCORE_VERSION);
    header.append(fmt::format("#agents: {:d}\n", nPeds));
    header.append(fmt::format("#count: {:d}\n", count));
    header.append(fmt::format("#framerate: {:0.2f}\n", fps));
    header.append(fmt::format("#geometry: {:s}\n", cfg.geometryFile.filename().string()));

    // if used: add two necessary files time table and train types
    if(!cfg.trainTimeTableFile.empty() && !cfg.trainTypeFile.empty()) {
        header.append(
            fmt::format("#trainTimeTable: {:s}\n", cfg.trainTimeTableFile.filename().string()));
        header.append(fmt::format("#trainType: {:s}\n", cfg.trainTypeFile.filename().string()));
    }
    // if used: add source file name
    if(!cfg.sourceFile.empty()) {
        header.append(fmt::format("#sources: {:s}\n", cfg.sourceFile.filename().string()));
    }

    // if used: add goal file name
    if(!cfg.goalFile.empty()) {
        header.append(fmt::format("#goals: {:s}\n", cfg.goalFile.filename().string()));
    }

    // if used: add event file name
    if(cfg.eventFile) {
        header.append(fmt::format("#events: {:s}\n", cfg.eventFile.value().filename().string()));
    }

    header.append("#ID: the agent ID\n");
    header.append("#FR: the current frame\n");
    header.append("#X,Y,Z: the agents coordinates (in metres)\n");
    header.append("#A, B: semi-axes of the ellipse\n");
    header.append("#ANGLE: orientation of the ellipse\n");
    header.append("#COLOR: color of the ellipse\n");

    // Add info for optional output options
    for(const auto& option : _options) {
        header.append(_optionalOutputInfo[option]);
    }

    header.append("\n#ID\tFR\tX\tY\tZ\tA\tB\tANGLE\tCOLOR\t");
    // Add header for optional output options
    for(const auto& option : _options) {
        header.append(_optionalOutputHeader[option]);
    }
    _outputHandler->Write(header);
}

void TrajectoryWriter::WriteFrame(
    int frameNr,
    const std::vector<std::unique_ptr<Pedestrian>>& pedestrian)
{
    for(const auto& ped : pedestrian) {
        double x = ped->GetPos().x;
        double y = ped->GetPos().y;
        double z = ped->GetElevation();
        int color = computeColor(*ped);
        double a = ped->GetLargerAxis();
        double b = ped->GetSmallerAxis();
        double phi = atan2(ped->GetEllipse().GetSinPhi(), ped->GetEllipse().GetCosPhi());
        double RAD2DEG = 180.0 / M_PI;
        std::string frame = fmt::format(
            "{}\t{:d}\t{:0.{}f}\t{:0.{}f}\t{:0.{}f}\t{:0.2f}\t{:0.2f}\t{:0.2f}\t{:d}\t",
            ped->GetUID(),
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
        for(const auto& option : _options) {
            frame.append(_optionalOutput[option](ped.get()));
        }

        _outputHandler->Write(frame);
    }
}

int TrajectoryWriter::computeColor(const Pedestrian& ped) const
{
    static constexpr std::array colors{0, 255, 35, 127, 90};
    std::string key;

    switch(_colorMode) {
        case BY_VELOCITY: {
            int color = -1;
            double v0 = ped.GetEllipse().GetV0();
            if(v0 != 0.0) {
                double v = ped.GetV().Norm();
                color = static_cast<int>(v / v0 * 255);
            }
            return color;
        }

        case BY_ROUTER: {
            key = std::to_string(ped.GetRouterID());
        } break;

        case BY_GROUP: {
            return (colors[ped.GetGroup() % colors.size()]);
        }

        case BY_FINAL_GOAL: {
            key = std::to_string(ped.GetFinalDestination());
        } break;

        case BY_INTERMEDIATE_GOAL: {
            key = std::to_string(ped.GetDestination());
        } break;

        default:
            break;
    }

    std::hash<std::string> hash_fn;
    return static_cast<int>(hash_fn(key)) % 255;
}
