/*
 * HybridSimulationManager.h
 *
 *  Created on: Apr 20, 2015
 *      Author: piccolo
 */

#ifndef HYBRIDSIMULATIONMANAGER_H_
#define HYBRIDSIMULATIONMANAGER_H_

class HybridSimulationManager
{
public:
     HybridSimulationManager();
     virtual ~HybridSimulationManager();

     bool Init();
     bool Run();
     bool RunClient();
     bool RunServer();
     void Shutdown();

private:
     bool _shutdown=false;
};

#endif /* HYBRIDSIMULATIONMANAGER_H_ */
