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

        RectGrid* getGrid() { return grid; }
        void setGrid(RectGrid val) { grid = val; }

        int * getSpeedvalue() { return speedvalue; }
        void setSpeedvalue(int * val) { speedvalue = val; }

        Point * getGradientT() { return gradientT; }
        void setGradientT(Point * val) { gradientT = val; }

        double * getCost() { return cost;}
        void setCost(double * val) {cost = val;}

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

int main(int argc, char** argv) {

    ftest = new Fastmarchtest();
    ftest.run("./outputFMTest.txt");
    delete ftest;

}

#endif // MAIN
