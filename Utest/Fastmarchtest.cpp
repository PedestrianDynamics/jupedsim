#include "Fastmarchtest.h"

#include "../geometry/Point.h"
#include "../math/FastMarching.h"
#include "../IO/OutputHandler.h"
#include <limits>


Fastmarchtest::Fastmarchtest()
{
    fastmarcher = new FastMarcher();
    grid = new RectGrid();
    x_max = 100.;
    y_max = 100.;
    stepsize = .2;

    //value tables (later using the keys given by grid as index)
    speedvalue = new double[(int)(x_max/stepsize+1)*(int)(y_max/stepsize+1)];
    gradientT = new Point[(int)(x_max/stepsize+1)*(int)(y_max/stepsize+1)];
    cost = new double[(int)(x_max/stepsize+1)*(int)(y_max/stepsize+1)];
    if ((speedvalue == 0) || (gradientT == 0) || (cost == 0)) {
        std::cerr << "\nzu wenig Speicher fuer Value-Tables\n";
        exit(1);
    }

    //init grid
    grid->setBoundaries(0., 0., x_max, y_max);
    grid->setSpacing(stepsize, stepsize);
    grid->createGrid();

    //init value tables using grid information
    for (int i = 0; i < grid->getNumOfElements(); ++i) {
        cost[i] = -2.; // neg. value to indicate a not calculated value
        speedvalue[i] = 1.;
        gradientT[i].SetX(0.); // (0, 0) indicate a not calculated value
        gradientT[i].SetY(0.);
    }

    //setting target bound
    cost[grid->getKeyAtXY(0., 0.)] = 0.;

    //give fastmarcher the arrays
    fastmarcher->setSpeedArray(speedvalue);
    fastmarcher->setGradientArray(gradientT);
    fastmarcher->setCostArray(cost);
}

Fastmarchtest::~Fastmarchtest()
{
    delete[] speedvalue;
    delete[] gradientT;
    delete[] cost;
    delete fastmarcher;
    delete grid;
}

void Fastmarchtest::run(char* outputfile) {
    //Log = new STDIOHandler();
    OutputHandler* Log;
    if (outputfile != 0) {
        Log = new FileHandler(outputfile);
    } else {
        Log = new FileHandler("./outputFMTest");
    };
    fastmarcher->calculateFloorfield();
    RectGrid* g = fastmarcher->getGrid();
    Log->Write("\nPosX, PosY, cost, gradientT_x, gradientT_y, speedvalue");
    for (int i = 0; i < g->getNumOfElements(); ++i) {
        Point iPoint = g->getPointFromKey(i);
        Log->Write("\n" + std::to_string(iPoint.GetX()) + ", " + std::to_string(iPoint.GetY()) + ", ");
        Log->Write(std::to_string(fastmarcher->getTimecostAt(iPoint)) + ", ");
        Log->Write(std::to_string((fastmarcher->getFloorfieldAt(iPoint)).GetX()) + ", " + std::to_string((fastmarcher->getFloorfieldAt(iPoint)).GetY())  + ", ");
        Log->Write(std::to_string(fastmarcher->getSpeedAt(iPoint)) + ", ");
    }
    delete Log;
}
