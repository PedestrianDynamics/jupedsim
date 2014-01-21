/**
 * @file   CognitiveMap.h
 * @author David Haensel (d.haensel@fz-juelich.de)
 * @date   January, 2014
 * @brief  Cognitive Map models the pedestrian knowledge of building space in simulation.
 *
 */

#ifndef COGNITIVEMAP_H_
#define COGNITIVEMAP_H_


/**
 * @brief Cognitive Map
 *
 * The CognitiveMap is a simplified representation of human knowledge of building space.
 * It takes the different quantity and certainty of knowledge into account.
 * It is the basis for further developments like individual behavior and information sharing algorithms.
 *
 */

class CognitiveMap {

public:
    /****************************
     * Constructors & Destructors
     ****************************/

    CognitiveMap();

    virtual ~CognitiveMap();


private:


};


#endif /* COGNITIVEMAP_H_ */
