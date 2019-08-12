#include "FDSMesh.h"

#include "IO/OutputHandler.h"

#include <cnpy.h>
#include <cmath>

std::vector<std::string> &split2(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}



FDSMesh::FDSMesh() : _statMesh(false)
{
    //statHeaderRead=false;
}

FDSMesh::FDSMesh(const double &xmin, const double &ymin, const double &xmax, const double &ymax, const double &cellsize) : _statMesh(false)
{
    SetUpMesh(xmin,ymin,xmax,ymax,cellsize);
    std::cout << "FDSMesh set up!" << std::endl;
    //statHeaderRead=false;
}

FDSMesh::FDSMesh(const std::string &filename) : _statMesh(false)
{
    //std::cout << filename << std::endl;
    SetKnotValuesFromFile(filename);

    //statHeaderRead=false;
}

FDSMesh::~FDSMesh()
{

}

void FDSMesh::SetUpMesh(const double &xmin, const double &ymin, const double &xmax, const double &ymax, const double &cellsize)
{
    _cellsize=cellsize;

    /// as knot is the middle of a cell
    _xmin=xmin+0.5*cellsize;
    _xmax=xmax-0.5*cellsize;
    _ymin=ymin+0.5*cellsize;
    _ymax=ymax-0.5*cellsize;

    int cols = (xmax - xmin) / cellsize+1;
    int rows = (ymax - ymin) / cellsize+1;

    /// set up matrix

    _matrix.resize(rows);
    for (auto &elem : _matrix)
    {
        elem.resize(cols);
    }

    for (unsigned int i=0; i<_matrix.size(); ++i)
    {
        for (unsigned int j=0; j<_matrix[0].size(); ++j)
        {
             Knot k(xmin+i*cellsize, j*cellsize);
             _matrix[i][j]=k;
        }
    }
}

const Matrix &FDSMesh::GetMesh() const
{
    return _matrix;
}

int FDSMesh::GetColumn(const double &x, int &col) const
{
    double restx;

    if (x>_xmin && x<_xmax)
    {
        restx = fmod((x-_xmin),_cellsize);

        col = (x-_xmin)/_cellsize;

        if (restx>_cellsize/2.0)
        {
            ++col;
        }
    }

    else if (x>=_xmax)
    {
        restx=0;
        col=_matrix.size()-1;
    }
    else
    {
        restx=0;
        col=0;
    }
    return col;
}

int FDSMesh::GetRow(int &row, const double &y) const
{
    double resty;

    if (y>_ymin && y<_ymax)
    {
        resty = fmod((y-_ymin),_cellsize);
        row = (y-_ymin)/_cellsize;

        if (resty>_cellsize/2.0)
        {
            ++row;
        }
    }
    else if (y>=_ymax)
    {
        resty=0;
        row=_matrix[0].size()-1;
    }
    else
    {
        resty=0;
        row=0;
    }
    return row;
}

double FDSMesh::GetKnotValue(const double &x, const double &y) const
{
    /// To Do: exception / warning when no knot is available for the pedestrian position
    int col=0;
    int row=0;
    double value;
    /// Which knot is the nearest one to (x,y)?

    GetColumn(x, col);
    GetRow(row, y);

    // Exception if a mesh can not provide an appropriately located value
    if((unsigned int)row < _matrix.size() && (unsigned int)col < _matrix[0].size())
          value = _matrix[row][col].GetValue();
    else
        // needs to be fixed!!!
          value = 0.0;//std::numeric_limits<double>::quiet_NaN();

    return value;

}

void FDSMesh::ReadMatrix(std::string line, std::ifstream &pFile)
{
    int m = 0;
    int n;
    std::vector<std::string> strVec;
    while (std::getline(pFile, line)) {
        n = 0;
        strVec = split2(line, ',', strVec);
        for (auto &elem : strVec) {
            //std::cout << elem << " col " << n  << " line " << m << std::endl;
            if (elem=="nan" || elem=="NAN" || elem=="NaN" || elem=="-inf" || elem=="inf") {
                _matrix[m][n].SetValue(1.0);
                //Log->Write("ERROR: Mesh values consist of nan!");
                //exit(EXIT_FAILURE);
            }
            else {
                double temp = 0;
                try {
                    temp = std::stod(elem);
                }
                catch (...) {
                    std::cout << "can not convert " << elem << std::endl;
                }
            _matrix[m][n].SetValue(temp);
            }
            ++n;
        }
        strVec.clear();
        ++m;
    }

    pFile.close();
    _statMesh=true;
}

void FDSMesh::SetKnotValuesFromFile(const std::string &filename)
{
    ///open File (reading)
    std::ifstream pFile(filename);
    if(!pFile)
    {
        Log->Write("ERROR:\tCould not open FDS slicefile: %s",filename.c_str());
        //return false;
        exit(EXIT_FAILURE);
    }

    cnpy::NpyArray Header = cnpy::npz_load(filename,"header");
    cnpy::NpyArray smoke_factor_grid_norm = cnpy::npz_load(filename,"smoke_factor_grid_norm");
    auto c_header = Header.data<double>();
    auto c_matrix = smoke_factor_grid_norm.data<double>();

    // for (int i=0; i< Header.num_vals ; i++)
    //     std::cout << "Header i: " << i << ": " << c_header[i] << std::endl;
    // keep these for loops for debugging purpose

    // for (int j=0; j< smoke_factor_grid_norm.num_vals; j++)
    //     std::cout << "Matrix j: "<< c_matrix[j] << std::endl;

    /// read header
    double cellsize = c_header[0];
    double xmin = c_header[1];
    double xmax = c_header[2];
    double ymin = c_header[3];
    double ymax = c_header[4];

    //std::cout << "xmin=" << xmin << " , xmax=" << xmax << " , ymin=" << ymin << ", ymax=" << ymax << " , dx=" << cellsize << std::endl;

    SetUpMesh(xmin,ymin,xmax,ymax,cellsize);

    //Read matrix
    unsigned int ncol = smoke_factor_grid_norm.shape[1];
    unsigned int nrow = smoke_factor_grid_norm.shape[0];
    for (unsigned int i=0; i< nrow; i++)
        for (unsigned int j=0; j< ncol; j++) {
             double tmp_value = c_matrix[i*ncol+j];
             if(std::isnan(tmp_value) || std::isinf(tmp_value))
                  tmp_value = 1.0;
             //std::cout << "i =  " << i  << "  j = " << j << ": " << tmp_value << std::endl;
            _matrix[i][j].SetValue(tmp_value); //TODO: implement =operator
        }
    pFile.close();
    _statMesh=true;
}

bool FDSMesh::statusMesh() const
{
    return _statMesh;
}
