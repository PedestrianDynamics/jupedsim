#include <cstdlib>
#include <stdio.h>
#include "../geometry/Line.h"
#include<math.h>

#ifdef WINDOWS
#include <direct.h>
    #define GetCurrentDir _getcwd
#else
#include <unistd.h>
    #define GetCurrentDir getcwd
 #endif

const double eps = 0.00001;
const double pi= atan(1)*4;


int testIntersectionWith(FILE * f)
{
    fprintf (f, "\t+++++ Enter testIntersectionWith() +++++\n");
    int ntests=1, 
        res=0;
   
    bool bres;

    Line L1 = Line( Point(1, 1), Point(2, 3));
    Line L2 = Line( Point(1, 0), Point(0, 1)); 

    bres = L1.IntersectionWith(L2);
    if (!bres)
        res++;
    fprintf (f, "%2d. res=%2d, L1_P1(%.2f, %.2f), L1_P2(%.2f, %.2f), L2_P1(%.2f, %.2f) L2_P2(%.2f, %.2f)\n", ntests, res, L1.GetPoint1().GetX(),L1.GetPoint1().GetY(),L1.GetPoint2().GetX(),L1.GetPoint2().GetY(), L2.GetPoint1().GetX(),L2.GetPoint1().GetY(),L2.GetPoint2().GetX(),L2.GetPoint2().GetY());

    

    fprintf (f, "\t+++++ Leave testIntersectionWith() +++++\n\n");
    return (res==ntests)?1:0;
}



int main()
{
    FILE * f;
    char fname[FILENAME_MAX] = "log_testClassLine.txt";
    char cCurrentPath[FILENAME_MAX];
    f = fopen(fname, "w");
    if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath)))
    {
        return EXIT_FAILURE;
    }
    cCurrentPath[sizeof(cCurrentPath) - 1] = '\0'; /* not really required */

    fprintf (f, "The current working directory is %s\n\n", cCurrentPath);

    int res = 0, ntests = 0;

    res += testIntersectionWith(f);
    ntests++;

    fclose(f);
    return (res==ntests)?EXIT_SUCCESS:EXIT_FAILURE;
}


