#include "cogmapoutputhandler.h"

CogMapOutputHandler::CogMapOutputHandler(const char *fn):FileHandler(fn)
{

}

CogMapOutputHandler::~CogMapOutputHandler()
{
    this->WriteToFileFooter();
}

void CogMapOutputHandler::WriteToFileHeader(const int &id, const double& fps)
{
    //nPeds=building->GetAllPedestrians().size();
    std::string tmp;
    tmp = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\n" "<cognitiveMap>\n";
    char agents[CLENGTH] = "";
    sprintf(agents, "\t<header version = \"0.8\">\n");
    tmp.append(agents);
    sprintf(agents, "\t\t<agent>%d</agent>\n", id);
    tmp.append(agents);
    sprintf(agents, "\t\t<frameRate>%0.2f</frameRate>\n",fps );
    tmp.append(agents);
    tmp.append("\t</header>\n");
    Write(tmp);
}

void CogMapOutputHandler::WriteToFile(std::string &string)
{
    Write(string);
}

void CogMapOutputHandler::WriteToFileFooter()
{
    Write("</cognitiveMap>\n");
}

