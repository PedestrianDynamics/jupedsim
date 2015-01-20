#ifndef FASTMARCHTEST_H
#define FASTMARCHTEST_H

#include "../geometry/Point.h"
#include "../math/FastMarching.h"
#include "../IO/OutputHandler.h"

class Fastmarchtest
{
    public:
        Fastmarchtest();
        virtual ~Fastmarchtest();
        RectGrid Getgrid() { return grid; }
        void Setgrid(RectGrid val) { grid = val; }
        int * Getspeedvalue() { return speedvalue; }
        void Setspeedvalue(int * val) { speedvalue = val; }
        Point * GetgradientT() { return gradientT; }
        void SetgradientT(Point * val) { gradientT = val; }

        void run(char* outputfile);
    protected:
    private:
        FastMarcher* fastmarcher;
        RectGrid* grid;
        double* speedvalue;
        Point* gradientT;
        double* cost;
        double x_max, y_max;
        double stepsize;

};

#endif // FASTMARCHTEST_H

#ifndef MAIN
#def MAIN

int public main(int argc, char** argv) {

ftest = new Fastmarchtest();
ftest.run("./outputFMTest.txt");
delete ftest;

}

#endif // MAIN
