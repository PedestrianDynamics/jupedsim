/**
 * @file CompleteCognitiveMapCreator.h
 * @author David H. <d.haensel@fz-juelich.de>
 * @date 02, 2014
 * @brief
 */


#ifndef COMPLETECOGNITIVEMAPCREATOR_H
#define COMPLETECOGNITIVEMAPCREATOR_H 1

#include "AbstractCognitiveMapCreator.h"

class Building;




class CompleteCognitiveMapCreator : public AbstractCognitiveMapCreator
{

 public:
 CompleteCognitiveMapCreator(const Building * b) : AbstractCognitiveMapCreator(b) { }

  virtual ~CompleteCognitiveMapCreator();

  CognitiveMap * CreateCognitiveMap(const Pedestrian * ped);
 private:

};

#endif // COMPLETECOGNITIVEMAPCREATOR_H
