/*
 * File:   IODispatcher.h
 * Author: andrea
 *
 * Created on 20. November 2010, 15:20
 */

#ifndef _IODISPATCHER_H
#define	_IODISPATCHER_H

#include "OutputHandler.h"
#include "../geometry/Building.h"


extern OutputHandler* Log;

class IODispatcher {
private:
	vector<OutputHandler* > pHandlers;
	// private Funktionen
	string WritePed(Pedestrian* ped);

public:
	IODispatcher();
	IODispatcher(const IODispatcher& orig);
	virtual ~IODispatcher();

	void AddIO(OutputHandler* ioh);
	const vector<OutputHandler*>& GetIOHandlers();
	void Write(string str);
	virtual void WriteHeader(int nPeds, int fps, Building* building, int seed=0, int szenarioID=0);
	virtual void WriteGeometry(Building* building);
	virtual void WriteFrame(int frameNr, Building* building);
	virtual void WriteFooter();
	int AreaLevel(string caption);

	template<typename A>
	bool IsElementInVector(const std::vector<A> &vec, A& el) {
		typename std::vector<A>::const_iterator it;
		it = find (vec.begin(), vec.end(), el);
		if(it==vec.end()){
			return false;
		}else{
			return true;
		}
	}


};

class HermesIODispatcher:public IODispatcher {

private:
	int pFileSectionFrame;
	string pPath;
	int pFPS;
	int pSeed;
	int pScenarioID;

	// somehow no one knows where the origin is situated
	double pTranslateX;
	double pTranslateY;

	string WritePed(Pedestrian* ped);

public:
	HermesIODispatcher(string path="./", int seed=0, int szenarioID=0, int fileSection=-1);
	virtual ~HermesIODispatcher(){};

	virtual void WriteHeader(int nPeds, int fps, Building* building, int seed=0, int szenarioID=0);
	virtual void WriteGeometry(Building* building);
	virtual void WriteFrame(int frameNr, Building* building);
	virtual void WriteFooter();

private:
	string ExecuteSystemCommand(string cmd);

};


class Trajectories:public IODispatcher {


public:
	Trajectories();
	virtual ~Trajectories(){};

	virtual void WriteHeader(int nPeds, int fps, Building* building);
	virtual void WriteGeometry(Building* building);
	virtual void WriteFrame(int frameNr, Building* building);
	virtual void WriteFooter();

};


#endif	/* _IODISPATCHER_H */

