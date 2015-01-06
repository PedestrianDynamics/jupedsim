/*
 * PedData.cpp
 *
 *  Created on: Jan 6, 2015
 *      Author: piccolo
 */

#include <PedData.h>

PedData::PedData(const std::string& filename)
{
     ReadData(filename);
}

PedData::~PedData()
{

}

bool PedData::ReadData(const std::string& filename)
{
     return true;
}


int PedData::GetMinFrame() const
{
     return _minFrame;
}

int PedData::GetMinID() const
{
     return _minID;
}
