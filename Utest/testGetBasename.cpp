#include<cstdlib>
#include "../Analysis.h"

extern OutputHandler* Log;

int testGetBasename()
{
    Analysis analysis = Analysis();
    std::string strWin("winhelp.exe");
    std::string str;
    str = analysis.GetBasename(strWin);
    return (str.compare("winhelp")==0)?EXIT_SUCCESS:EXIT_FAILURE;
}

int main ()
{
    return testGetBasename();
}
