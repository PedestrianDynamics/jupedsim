#pragma once

#include "Knot.h"

#include <vector>
#include <string>
//log output
class OutputHandler;
extern OutputHandler* Log;

using Matrix = std::vector<std::vector<Knot> >;

class FDSMesh
{
public:
    FDSMesh();
    FDSMesh(const double &xmin, const double &ymin, const double &xmax, const double &ymax, const double &cellsize);
    FDSMesh(const std::string &filename);
    ~FDSMesh();

    void SetUpMesh(const double &xmin, const double &ymin, const double &xmax, const double &ymax, const double &cellsize);
    const Matrix &GetMesh() const;

    double GetKnotValue(const double &x, const double &y) const;
    void SetKnotValuesFromFile(const std::string &filename);


    bool statusMesh() const;

    int GetColumn(const double &x, int &col) const;
    int GetRow(int &row, const double &y) const;
    void ReadMatrix(std::string line, std::ifstream &pFile);

private:
    Matrix _matrix;
    double _cellsize;
    double _xmin;
    double _xmax;
    double _ymin;
    double _ymax;
    bool _statMesh;
};
