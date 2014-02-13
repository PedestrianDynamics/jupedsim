/**
 * @file AbstractCognitiveMapCreator.h
 * @author David H. <d.haensel@fz-juelich.de>
 * @date 02, 2014
 * @brief
 */


#ifndef ABSTRACTCOGNITIVEMAPCREATOR_H
#define ABSTRACTCOGNITIVEMAPCREATOR_H 1

class Building;
class Pedestrian;
class CognitiveMap;



class AbstractCognitiveMapCreator
{
public:
    AbstractCognitiveMapCreator(const Building * b) : building(b) {}


    virtual ~AbstractCognitiveMapCreator();
    virtual CognitiveMap * CreateCognitiveMap(const Pedestrian * ped) = 0;
private:
    const Building * const building;
};

#endif // ABSTRACTCOGNITIVEMAPCREATOR_H
