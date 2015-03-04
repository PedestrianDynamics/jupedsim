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
    stepsize = .01;

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

//    for (int yi = 0; yi < (6/stepsize); ++yi) {
//        for (int xi = (5/stepsize); xi < (6/stepsize); ++xi) {
//            speedvalue[(int)(((10/stepsize)+1)*yi + xi)] = .001; // (10/stepsize) + 1 = stride
//        }
//    }
//    for (int yi = (2/stepsize); yi < (3/stepsize); ++yi) {
//        for (int xi = 0; xi < (4/stepsize); ++xi) {
//            speedvalue[(int)(((10/stepsize)+1)*yi + xi)] = .001;
//        }
//    }
//    for (int yi = (7/stepsize); yi < (8/stepsize); ++yi) {
//        for (int xi = 0; xi < (8/stepsize); ++xi) {
//            speedvalue[(int)(((10/stepsize)+1)*yi + xi)] = .001;
//        }
//    }

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
        Log = new FileHandler("./outputFMTestMC.vtk");
    };
    fastmarcher->calculateFloorfield();
    RectGrid* g = fastmarcher->getGrid();
    int numX = (int) (10/stepsize) + 1;
    int numY = (int) (10/stepsize) + 1;
    int numTotal = numX * numY;
    //Log->Write("PosX, PosY, cost,"); //gradientT_x, gradientT_y, speedvalue");
    Log->Write("# vtk DataFile Version 3.0");
    Log->Write("Testdata: Fast Marching: Test: ");
    Log->Write("ASCII");
    Log->Write("DATASET STRUCTURED_POINTS");
    Log->Write(("DIMENSIONS " +
                                std::to_string(numX) +
                                " " +
                                std::to_string(numY) +
                                " 1"));
    Log->Write("ORIGIN 0 0 0");
    Log->Write(("SPACING " + std::to_string(stepsize) + " " + std::to_string(stepsize) + " 1"));
    Log->Write(("POINT_DATA " + std::to_string(numTotal) ));
    Log->Write("SCALARS Cost float 1");
    Log->Write("LOOKUP_TABLE default");
    for (int i = 0; i < g->getNumOfElements(); ++i) {
        Point iPoint = g->getPointFromKey(i);
        Log->Write(std::to_string(fastmarcher->getTimecostAt(iPoint)));
    }
    Log->Write("VECTORS Gradient float");
    for (int i = 0; i < g->getNumOfElements(); ++i) {
        Point iPoint = g->getPointFromKey(i);
        Log->Write(std::to_string((fastmarcher->getFloorfieldAt(iPoint)).GetX())
                    + " "
                    + std::to_string((fastmarcher->getFloorfieldAt(iPoint)).GetY())
                    + " 0.0");
    }

    OutputHandler* Log2;
    Log2 = new FileHandler("./forpython.txt");

    for (int i = 0; i < g->getNumOfElements(); ++i) {
        Point iPoint = g->getPointFromKey(i);
        Log2->Write(std::to_string(iPoint.GetX()) + " " + std::to_string(iPoint.GetY()) + " " + std::to_string(fastmarcher->getTimecostAt(iPoint)));
        //Log2->Write(std::to_string(fastmarcher->getTimecostAt(iPoint)) + ", ");
        //Log2->Write(std::to_string((fastmarcher->getFloorfieldAt(iPoint)).GetX()) + ", " + std::to_string((fastmarcher->getFloorfieldAt(iPoint)).GetY())  + ", ");
        //Log2->Write(std::to_string(fastmarcher->getSpeedAt(iPoint)) + ", ");
    }
    delete Log;
    delete Log2;
}
