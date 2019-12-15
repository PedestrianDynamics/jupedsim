#include "../Analysis.h"

#include <cstdlib>

extern OutputHandler * Log;

int testGetFilename()
{
    int res           = 0;
    Analysis analysis = Analysis();
    std::string strWin("c:\\windows\\winhelp.exe");
    std::string strLin("/home/foo/winhelp.exe");
    std::string str, str1;
    str = analysis.GetFilename(strWin);
    //test windows path
    res = (str.compare("winhelp.exe") == 0) ? 1 : 0;

    str1 = analysis.GetFilename(strLin);
    //test Linux path
    res += (str1.compare("winhelp.exe") == 0) ? 1 : 0;

    return (res == 2) ? EXIT_SUCCESS : EXIT_FAILURE;
}

int main()
{
    return testGetFilename();
}
