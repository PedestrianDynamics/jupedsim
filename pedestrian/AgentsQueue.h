/*
 * AgentsQueue.h
 *
 *  Created on: 14.04.2015
 *      Author: piccolo
 */

#ifndef AGENTSQUEUE_H_
#define AGENTSQUEUE_H_


#include <mutex>
#include <vector>

//forward declaration
class Pedestrian;

/**
 * Queue for incoming agents
 */
class AgentsQueue
{
public:
     static void Add(std::vector<Pedestrian*>& ped);
     static void GetandClear(std::vector<Pedestrian*>& peds);
     static bool IsEmpty();
     //static std::vector<Pedestrian*> GetandClear();

private:
     AgentsQueue(){};
     virtual ~AgentsQueue(){};
     static std::vector<Pedestrian*> _agentsQueue;
     static std::mutex _queueMutex;
};

/**
 * Queue for outgoing agents
 */
class AgentsQueueOut
{
public:
     static void Add(std::vector<Pedestrian*>& ped);
     static void Add(Pedestrian* ped);
     static void GetandClear(std::vector<Pedestrian*>& peds);
     static bool IsEmpty();

private:
     AgentsQueueOut(){};
     virtual ~AgentsQueueOut(){};
     static std::vector<Pedestrian*> _agentsQueue;
     static std::mutex _queueMutex;
};
#endif /* AGENTSQUEUE_H_ */
