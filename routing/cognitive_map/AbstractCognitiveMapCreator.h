/**
 * \file        AbstractCognitiveMapCreator.h
 * \date        Feb 1, 2014
 * \version     v0.6
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


#ifndef ABSTRACTCOGNITIVEMAPCREATOR_H
#define ABSTRACTCOGNITIVEMAPCREATOR_H 1

class Building;
class Pedestrian;
class CognitiveMap;



class AbstractCognitiveMapCreator {
public:
     AbstractCognitiveMapCreator(const Building * b) : _building(b) {}


     virtual ~AbstractCognitiveMapCreator();
     virtual CognitiveMap * CreateCognitiveMap(const Pedestrian * ped) = 0;
protected:
     const Building * const _building;
};

#endif // ABSTRACTCOGNITIVEMAPCREATOR_H
