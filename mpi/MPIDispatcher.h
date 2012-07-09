/*
 * MPIDispatcher.h
 *
 *  Created on: Feb 14, 2011
 *      Author: piccolo
 *      http://stackoverflow.com/questions/329259/how-do-i-debug-an-mpi-program
 */

#ifndef MPIDISPATCHER_H_
#define MPIDISPATCHER_H_


#define TAG_PEDSCOUNT 101
#define TAG_SHUTDOWN 102
#define TAG_DEFAULT 98
#define TAG_NEIGHBORHOOD  99
#define TAG_PROCESSOR_MAP 100


#include "../geometry/Building.h"

#include <iostream>
#include <map>


class MPIDispatcher {

private:
	int pRank;
	int pCommSize;
	int pProcessorMap[256];
	//char* pNeighbourhood;
	Building* pBuilding;

	//shutdown the process
	int pShutdown;
	//int pRoomList[256]; // at most 256 rooms for each processor
	//std::map<int,int> pProcessorMap; // map a room to a processor
	std::vector<int> pWorkingArea; // list of rooms for the actual processor
	std::vector<int> pFriends; // processors with which I will be exchanging information at each time step
	std::map<int, std::vector<Point > > pGhostAreas; // map a transition/processor to a list of cells

private:
	Pedestrian* GetPedWithID(int id);
	void ClearGhostPedestrians();


public:
	MPIDispatcher(int rank, int commSize);
	virtual ~MPIDispatcher();
	void Initialise(Building* pBuilding);
	int GetMyRank();
	const vector<int>& GetMyWorkingArea() const;
	bool IsMyWorkingArea(int roomID);

	// CAUTION:
	// all send are non blocking
	void SendMessage(int to_procID, double** msg, int msgLength);

	// CAUTION
	// all recv are blocking
	void RecvMessage(int from_procID, double** msg, int* msgLength);

	// synchronize ghost areas with neighborhood processes
	// send and receive Neighboourhood
	void Update();

	// shutdown the process

	bool Shutdown();

	// return the neighboorhood for a crossing
	// The neighborhood for a point is already computed in grid
	// void getNeighbourhood(const Crossing* cross, vector<Pedestrian*>& n);
	void getNeighbourhood(int crossID, vector<Pedestrian*>& n);

	//debug
	void Dump();
	void FindNeighbourProcessors();

	// return the toal number of pedestrian in the simulation
	// across all processors
	int GetGlobalPedestriansCount();
};

#endif /* MPIDISPATCHER_H_ */
