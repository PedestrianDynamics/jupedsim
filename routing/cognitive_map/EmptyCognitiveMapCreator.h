/**
 * @file EmptyCognitiveMapCreator.h
 * @author David H. <d.haensel@fz-juelich.de>
 * @date 02, 2014
 * @brief
 */


#ifndef EMPTYCOGNITIVEMAPCREATOR_H
#define EMPTYCOGNITIVEMAPCREATOR_H 1

#include "AbstractCognitiveMapCreator.h"

class Building;




class EmptyCognitiveMapCreator : public AbstractCognitiveMapCreator {

public:
     EmptyCognitiveMapCreator(const Building * b) : AbstractCognitiveMapCreator(b) { }

     virtual ~EmptyCognitiveMapCreator();

     CognitiveMap * CreateCognitiveMap(const Pedestrian * ped);
private:

};

#endif // EMPTYCOGNITIVEMAPCREATOR_H
