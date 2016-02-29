#include "FDSMesh.h"
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>

std::vector<std::string> &split2(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}



FDSMesh::FDSMesh()
{
    //statHeaderRead=false;
    _statMesh=false;
}

FDSMesh::FDSMesh(const double &xmin, const double &ymin, const double &xmax, const double &ymax, const double &cellsize)
{
    SetUpMesh(xmin,ymin,xmax,ymax,cellsize);
    std::cout << "FDSMesh set up!" << std::endl;
    //statHeaderRead=false;
    _statMesh=false;
}

FDSMesh::FDSMesh(const std::string &filename)
{
    //std::cout << filename << std::endl;
    SetKnotValuesFromFile(filename);
    //statHeaderRead=false;
    _statMesh=false;

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

double FDSMesh::GetKnotValue(const double &x, const double &y) const
{
    double restx;
    double resty;
    int col;
    int row;

    /// Which knot is the nearest one to (x,y)?
    ///
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

    return _matrix[row][col].GetValue();
}

void FDSMesh::SetKnotValuesFromFile(const std::string &filename)
{
    ///open File (reading)
    std::ifstream pFile(filename);

    if (pFile)
    {
        std::vector<std::string> strVec;
        std::string line;
        ///skip two lines
        std::getline(pFile, line);
        std::getline(pFile, line);

        std::getline(pFile, line);
        //std::cout << line << std::endl;
        /// to avoid multiple reading of the header and mesh setting
        //if (statHeaderRead==false)
        //{
        /// read header
        strVec = split2(line,',', strVec);
        double cellsize = std::stod(strVec[0]);
        double xmin = std::stod(strVec[2]);
        double xmax = std::stod(strVec[3]);
        double ymin = std::stod(strVec[4]);
        double ymax = std::stod(strVec[5]);

        strVec.clear();
        //std::cout << "xmin=" << xmin << " , xmax=" << xmax << " , ymin=" << ymin << ", ymax=" << ymax << " , dx=" << cellsize << std::endl;

        SetUpMesh(xmin,ymin,xmax,ymax,cellsize);

            //statHeaderRead=true;
        //}
        ///Read matrix
        ///
        int m = 0;
        int n;
        while (std::getline(pFile, line)) {
            n = 0;
            strVec = split2(line, ',', strVec);
            for (auto &elem : strVec)
            {
                //std::cout << elem << std::endl;
                _matrix[m][n].SetValue(std::stod(elem));
                ++n;
            }
            strVec.clear();
            ++m;
        }

        pFile.close();
        _statMesh=true;
    }
    else
    {
       Log->Write("ERROR:\tCould not open FDS slicefile: %s",filename.c_str());
       //return false;
       exit(EXIT_FAILURE);
    }

}

bool FDSMesh::statusMesh() const
{
    return _statMesh;
}
