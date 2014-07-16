/**
 * \file        DummyRouter.cpp
 * \date        Aug 7, 2012
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


#include "DummyRouter.h"
#include "../geometry/Building.h"
#include "../pedestrian/Pedestrian.h"

DummyRouter::DummyRouter()
{
     _building=NULL;
}

DummyRouter::~DummyRouter()
{

}

int DummyRouter::FindExit(Pedestrian* p)
{
     p->SetExitIndex(1);
     //p->SetExitLine(_building->getGetCrossing(0));
     return 1;
}

void DummyRouter::Init(Building* b)
{
     _building=b;
     Log->Write("ERROR: \tdo not use this  <<Dummy>> router !!");

     //dump all navigation lines

//      cout<<"crossing:"<<endl;
//      for (map<int, Crossing*>::const_iterator iter = pCrossings.begin();
//                      iter != pCrossings.end(); ++iter) {
//              iter->second->WriteToErrorLog();
//      }
//      for (map<int, Transition*>::const_iterator iter = pTransitions.begin();
//                      iter != pTransitions.end(); ++iter) {
//              iter->second->WriteToErrorLog();
//      }
//      for (map<int, Hline*>::const_iterator iter = pHlines.begin();
//                      iter != pHlines.end(); ++iter) {
//              iter->second->WriteToErrorLog();
//      }
//      exit(EXIT_FAILURE);
}


