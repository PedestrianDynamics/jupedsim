/*
 * PedData.h
 *
 *  Created on: Jan 6, 2015
 *      Author: piccolo
 */

#ifndef PEDDATA_H_
#define PEDDATA_H_

#include <string>

class PedData
{
public:
     PedData(const std::string& filename);
     virtual ~PedData();
     int GetMinFrame() const;
     int GetMinID() const;


private:
     bool ReadData(const std::string& filename);


private:
     int _minFrame;
     int _minID;

};

#endif /* PEDDATA_H_ */
