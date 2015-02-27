#include "Fastmarchtest.h"

#include "../geometry/Point.h"
#include "../math/FastMarching.h"
#include "../IO/OutputHandler.h"
#include <limits>


Fastmarchtest::Fastmarchtest()
{
    fastmarcher = new FastMarcher();
    grid = new RectGrid();
    x_max = 10.;
    y_max = 10.;
    stepsize = .02;

    //init grid
    grid->setBoundaries(0., 0., x_max, y_max);
    grid->setSpacing(stepsize, stepsize);
    grid->createGrid();

    //value tables (later using the keys given by grid as index)
    int nGridpoints = grid->getNumOfElements();
    speedvalue = new double[nGridpoints];
    gradientT = new Point[nGridpoints];
    cost = new double[nGridpoints];
    if ((speedvalue == 0) || (gradientT == 0) || (cost == 0)) {
        std::cerr << "\nzu wenig Speicher fuer Value-Tables\n";
        exit(1);
    }

    //init value tables using grid information
    for (int i = 0; i < grid->getNumOfElements(); ++i) {
        cost[i] = -2.; // neg. value to indicate a not calculated value
        speedvalue[i] = 1.;
        gradientT[i].SetX(0.); // (0, 0) indicate a not calculated value
        gradientT[i].SetY(0.);
    }

    for (int yi = 0; yi < 300; ++yi) {
        for (int xi = 250; xi < 300; ++xi) {
            speedvalue[(501*yi + xi)] = .001;
        }
    }
    for (int yi = 100; yi < 150; ++yi) {
        for (int xi = 0; xi < 200; ++xi) {
            speedvalue[(501*yi + xi)] = .001;
        }
    }
    for (int yi = 350; yi < 400; ++yi) {
        for (int xi = 0; xi < 400; ++xi) {
            speedvalue[(501*yi + xi)] = .001;
        }
    }

    //setting target bound
    //cost[grid->getKeyAtXY(0., 0.)] = 0.;
    //cost[550] = 0.; //just to make sure.... ;)
    cost[0] = 0.;

    //give fastmarcher the arrays
    fastmarcher->setSpeedArray(speedvalue);
    fastmarcher->setGradientArray(gradientT);
    fastmarcher->setCostArray(cost);
    fastmarcher->setGrid(grid);
    //std::cerr << fastmarcher->getGrid()->getNumOfElements();
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
        Log = new FileHandler("./outputFMTestMC");
    };
    fastmarcher->calculateFloorfield();
    RectGrid* g = fastmarcher->getGrid();
    //Log->Write("PosX, PosY, cost,"); //gradientT_x, gradientT_y, speedvalue");
    Log->Write("# vtk DataFile Version 3.0");
    Log->Write("Testdata: Fast Marching: Test: ");
    Log->Write("ASCII");
    Log->Write("DATASET STRUCTURED_POINTS");
    Log->Write("DIMENSIONS 501 501 1");
    Log->Write("ORIGIN 0 0 0");
    Log->Write("SPACING 0.02 0.02 1");
    Log->Write("POINT_DATA 251001");
    Log->Write("SCALARS Cost float 1");
    Log->Write("LOOKUP_TABLE default");
    for (int i = 0; i < g->getNumOfElements(); ++i) {
        Point iPoint = g->getPointFromKey(i);
        Log->Write(std::to_string(fastmarcher->getTimecostAt(iPoint)));
    }
    Log->Write("VECTORS Gradient float");
    for (int i = 0; i < g->getNumOfElements(); ++i) {
        Point iPoint = g->getPointFromKey(i);
        Log->Write(std::to_string((fastmarcher->getFloorfieldAt(iPoint)).GetX()) + " " + std::to_string((fastmarcher->getFloorfieldAt(iPoint)).GetY()) + " 0.0");
    }

//    for (int i = 0; i < g->getNumOfElements(); ++i) {
//        Point iPoint = g->getPointFromKey(i);
//        Log->Write(std::to_string(iPoint.GetX()) + " " + std::to_string(iPoint.GetY()) + " " + std::to_string(fastmarcher->getTimecostAt(iPoint)));
//        Log->Write(std::to_string(fastmarcher->getTimecostAt(iPoint)) + ", ");
//        Log->Write(std::to_string((fastmarcher->getFloorfieldAt(iPoint)).GetX()) + ", " + std::to_string((fastmarcher->getFloorfieldAt(iPoint)).GetY())  + ", ");
//        Log->Write(std::to_string(fastmarcher->getSpeedAt(iPoint)) + ", ");
//    }
    delete Log;
}
