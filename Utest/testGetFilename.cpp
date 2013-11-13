#include<cstdlib>
#include "../Analysis.h"

OutputHandler* Log;

int testGetFilename()
{
    Analysis analysis = Analysis();    
    std::string strWin("c:\\windows\\winhelp.exe");
    std::string str;
    str = analysis.GetFilename(strWin);
    return (str.compare("winhelp.exe")==0)?EXIT_SUCCESS:EXIT_FAILURE;
}

int main (int argc, char * argv[] )
{
    return testGetFilename();
}
