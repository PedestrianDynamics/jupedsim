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
        void setGrid(RectGrid* val) { grid = val; }

        double* getSpeedvalue() { return speedvalue; }
        void setSpeedvalue(double* val) { speedvalue = val; }

        Point* getGradientT() { return gradientT; }
        void setGradientT(Point* val) { gradientT = val; }

        double* getCost() { return cost;}
        void setCost(double* val) {cost = val;}

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
#define MAIN

int main(int argc, char** argv) {

    Fastmarchtest* ftest = new Fastmarchtest();
    ftest->run("./outputFMTestMCfine.vtk");
    delete ftest;

}

#endif // MAIN
