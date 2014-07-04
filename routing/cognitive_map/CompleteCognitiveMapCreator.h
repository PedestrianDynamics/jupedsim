/**
 * \file        CompleteCognitiveMapCreator.h
 * \date        Feb 1, 2014
 * \version     v0.5
 * \copyright   <2009-2014> Forschungszentrum Jülich GmbH. All rights reserved.
 *
 * \section License
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * \section Description
 *
 *
 **/


#ifndef COMPLETECOGNITIVEMAPCREATOR_H
#define COMPLETECOGNITIVEMAPCREATOR_H 1

#include "AbstractCognitiveMapCreator.h"

class Building;




class CompleteCognitiveMapCreator : public AbstractCognitiveMapCreator {

public:
     CompleteCognitiveMapCreator(const Building * b) : AbstractCognitiveMapCreator(b) { }

     virtual ~CompleteCognitiveMapCreator();

     CognitiveMap * CreateCognitiveMap(const Pedestrian * ped);
private:

};

#endif // COMPLETECOGNITIVEMAPCREATOR_H
