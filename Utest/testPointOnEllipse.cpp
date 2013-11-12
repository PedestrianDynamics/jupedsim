#include <cstdlib>
#include "../math/Mathematics.h"

#include "../pedestrian/Ellipse.h"

OutputHandler* Log;


//int testPointOnEllipse(int argc, char* argv[])
int testPointOnEllipse()
{
    int i, j, ntests=0, res=0;
    float r=2.0;
    JEllipse E;
    E.SetCenter( Point(0,0) );
    E.SetV0(1);
    E.SetV( Point(0,0) );
    E.SetAmin(r);
    E.SetBmax(r);
    
    float sx, sy, l;
    float rx, ry;
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
        }
    return (res==ntests)?EXIT_SUCCESS:EXIT_FAILURE;
}

int main(int argc, char* argv[])
{
    return testPointOnEllipse();
}
