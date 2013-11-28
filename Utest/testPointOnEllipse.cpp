#include <cstdlib>
#include <stdio.h>
#include "../pedestrian/Ellipse.h"

//#include "Config.h"

#ifdef WINDOWS
#include <direct.h>
    #define GetCurrentDir _getcwd
#else
#include <unistd.h>
    #define GetCurrentDir getcwd
 #endif

OutputHandler* Log;


//int testPointOnEllipse(int argc, char* argv[])
int testPointOnEllipse()
{

    char cCurrentPath[FILENAME_MAX];
    if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath)))
    {
        return EXIT_FAILURE;
    }
    cCurrentPath[sizeof(cCurrentPath) - 1] = '\0'; /* not really required */

    FILE * f;
    char fname[FILENAME_MAX] = "log_testPointOnEllipse.txt";
    // strcpy(fname, cCurrentPath);
    // strcat(fname, "logs/log_testPointOnEllipse.txt");
    f = fopen(fname, "w");
    fprintf (f, "The current working directory is %s\n\n", cCurrentPath);
    int i, j, ntests=0, res=0;
    float r=2.0;
    JEllipse E;
    E.SetCenter( Point(0,0) );
    E.SetV0(1);
    E.SetV( Point(0,0) );
    E.SetAmin(r);
    E.SetBmax(r);
    
    float sx, sy, l;
    Point S(0,0); //test point
    Point P(0,0); // result 
    Point T(0,0); // should be result

    for (i=0;i<2;i++)
        for (j=0;j<2;j++)
        {
            if(!(i|j)) continue;
            sx = i*r;
            sy = j*r;
            S.SetX(sx);
            S.SetY(sy);
            l = S.Norm();
            T.SetX(r*sx/l);
            T.SetY(r*sy/l);
            P = E.PointOnEllipse(S);
            res += (P==T)?1:0;
            ntests++;
            fprintf (f, "%d. Tx=%.2f\t Ty=%.2f\t Px=%.2f\t Py=%.2f\t res=%d\t\n",ntests, T.GetX(), T.GetY(), P.GetX(), P.GetY(), res);
        }

    // test point in the ellipse
    sx = 0.5*r;
    sy = 0.5*r;
    S.SetX(sx);
    S.SetY(sy);
    l = S.Norm();
    T.SetX(r*sx/l);
    T.SetY(r*sy/l);
    P = E.PointOnEllipse(S);
    res += (P==T)?1:0;
    ntests++;
    fprintf (f, "%d. Tx=%.2f\t Ty=%.2f\t Px=%.2f\t Py=%.2f\t res=%d\t\n",ntests, T.GetX(), T.GetY(), P.GetX(), P.GetY(), res);
    // point in the center

    sx = 0;
    sy = 0;
    S.SetX(sx);
    S.SetY(sy);

//    T.SetX(0);
    //  T.SetY(0);
    E.SetCenter( Point(0.4, 1.78) );
    T = E.GetCenter() + Point(r,0);
    P = E.PointOnEllipse(S);
    res += (P==T)?1:0;
    ntests++;
    fprintf (f, "%d. Tx=%.2f\t Ty=%.2f\t Px=%.2f\t Py=%.2f\t res=%d\t\n",ntests, T.GetX(), T.GetY(), P.GetX(), P.GetY(), res);
    
    fclose(f);
    return (res==ntests)?EXIT_SUCCESS:EXIT_FAILURE;
}

int main(int argc, char * argv[])
{
    //   fprintf(stdout, "%s Version %d.%d\n", argv[0], JPSCORE_MINOR_VERSION, JPSCORE_MAJOR_VERSION);
    return testPointOnEllipse();
}
