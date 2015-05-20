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
class AgentsQueueIn
{
public:
     static void Add(std::vector<Pedestrian*>& ped);
     static void GetandClear(std::vector<Pedestrian*>& peds);
     static bool IsEmpty();
     static int Size();
     //static std::vector<Pedestrian*> GetandClear();

private:
     AgentsQueueIn(){};
     virtual ~AgentsQueueIn(){};
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
     static int Size();

private:
     AgentsQueueOut(){};
     virtual ~AgentsQueueOut(){};
     static std::vector<Pedestrian*> _agentsQueue;
     static std::mutex _queueMutex;
};

#endif /* AGENTSQUEUE_H_ */
