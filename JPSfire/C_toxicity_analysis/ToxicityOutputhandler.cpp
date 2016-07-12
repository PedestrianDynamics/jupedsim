#include "ToxicityOutputhandler.h"


ToxicityOutputHandler::ToxicityOutputHandler(const char *fn):FileHandler(fn)
{

}

ToxicityOutputHandler::~ToxicityOutputHandler()
{
    this->WriteToFileFooter();
}

void ToxicityOutputHandler::WriteToFileHeader()
{
    //nPeds=building->GetAllPedestrians().size();
    std::string tmp;
    tmp = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\n" "<ToxicityAnalysis>\n";
    char agents[CLENGTH] = "";
    sprintf(agents, "\t<header version = \"0.8\">\n");
    tmp.append(agents);
    tmp.append("\t</header>\n");
    Write(tmp);
}

void ToxicityOutputHandler::WriteToFile(std::string &string)
{
    Write(string);
}

void ToxicityOutputHandler::WriteToFileFooter()
{
    Write("\t</frame>\n</ToxicityAnalysis>\n");
}

