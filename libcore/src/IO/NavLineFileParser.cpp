#include "NavLineFileParser.hpp"

#include "Logger.hpp"
#include "general/Macros.hpp"

#include <string_view>
#include <tinyxml.h>
#include <vector>

std::vector<NavLineParameters> parseNavLines(const std::filesystem::path& filename)
{
    LOG_INFO(
        "Loading extra routing information for the global/quickest path routeri from file {}",
        filename.string());

    TiXmlDocument docRouting(filename.string());
    if(!docRouting.LoadFile()) {
        LOG_ERROR("Error parsing {}: {}", filename, docRouting.ErrorDesc());
        return {};
    }

    TiXmlElement* xRootNode = docRouting.RootElement();
    if(!xRootNode) {
        LOG_ERROR("Error parsing {}: No root element found", filename);
        return {};
    }

    if(const std::string_view rootNodeName = xRootNode->ValueStr(); rootNodeName != "routing") {
        LOG_ERROR(
            "Error parsing {}: Expected root element named '<routing>', instead found {}",
            filename,
            rootNodeName);
        return {};
    }

    std::vector<NavLineParameters> lines{};

    for(TiXmlElement* xHlinesNode = xRootNode->FirstChildElement("Hlines"); xHlinesNode;
        xHlinesNode = xHlinesNode->NextSiblingElement("Hlines")) {
        for(TiXmlElement* hline = xHlinesNode->FirstChildElement("Hline"); hline;
            hline = hline->NextSiblingElement("Hline")) {
            int id = xmltoi(hline->Attribute("id"), -1);
            int roomId = xmltoi(hline->Attribute("room_id"), -1);
            int subroomId = xmltoi(hline->Attribute("subroom_id"), -1);

            double x1 = xmltof(hline->FirstChildElement("vertex")->Attribute("px"));
            double y1 = xmltof(hline->FirstChildElement("vertex")->Attribute("py"));
            double x2 = xmltof(hline->LastChild("vertex")->ToElement()->Attribute("px"));
            double y2 = xmltof(hline->LastChild("vertex")->ToElement()->Attribute("py"));

            lines.push_back(NavLineParameters{x1, y1, x2, y2, id, roomId, subroomId});
        }
    }
    return lines;
}
