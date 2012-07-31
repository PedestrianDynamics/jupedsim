/*
 * File:   PedDistributor.h
 * Author: andrea
 *
 * Created on 12. Oktober 2010, 10:52
 */

#ifndef _PEDDISTRIBUTOR_H
#define	_PEDDISTRIBUTOR_H

#include <vector>
#include <string>
using namespace std;

#include "../geometry/Room.h"
#include "../math/Distribution.h"
#include "../routing/Routing.h"
#include "../geometry/Building.h"
#include "../mpi/MPIDispatcher.h"


extern OutputHandler* Log;

/************************************************************
 StartDistributionRoom
 ************************************************************/
class StartDistributionRoom {
private:
	string pRoomCaption; // Raumname
	int pN; // Anzahl Personen

public:
	StartDistributionRoom();
	StartDistributionRoom(const StartDistributionRoom& orig);
	virtual ~StartDistributionRoom();

	// Getter-Funktionen
	string GetRoomCaption() const;
	int GetAnz() const;

	// Setter-Funktionen
	void SetRoomCaption(string caption);
	void SetAnz(int N);

	// Sonstige Funktionen
	virtual string ReadDistribution(string line);
};

class StartDistributionSubroom : public StartDistributionRoom {
private:
	int pSubroomID; // zusätzlich noch SubRoomID

public:
	StartDistributionSubroom();
	StartDistributionSubroom(const StartDistributionSubroom& orig);
	virtual ~StartDistributionSubroom();
	// Getter-Funktionen
	int GetSubroomID() const;

	// Setter-Funktionen
	void SetSubroomID(int i);
	// Sonstige Funktionen
	virtual string ReadDistribution(string line);
};

/************************************************************
 PedDistributor
 ************************************************************/
class PedDistributor {
private:
	Distribution* pv0; // Gauss - Verteilung für v0
	Distribution* pBmax;
	Distribution* pBmin;
	Distribution* pAtau;
	Distribution* pAmin;
	Distribution* pTau;
	vector<StartDistributionRoom> start_dis; // ID startraum, subroom und Anz
	vector<StartDistributionSubroom> start_dis_sub; // ID startraum, subroom und Anz

public:
	// Konstruktoren
	PedDistributor();
	PedDistributor(double v0mu, double v0sigma, double BmaxMu, double BmaxSigma,
			double BminMu, double BminSigma, double AtauMu, double AtauSigma, double AminMu,
			double AminSigma, double tauMu, double tauSigma);
	PedDistributor(const PedDistributor& orig);
	virtual ~PedDistributor();
	// Getter-Funktionen
	Distribution* GetV0() const;
	Distribution* GetBmax() const;
	Distribution* GetBmin() const;
	Distribution* GetAtau() const;
	Distribution* GetAmin() const;
	Distribution* GetTau() const;

	// sonstige Funktionen
	vector<Point> PositionsOnFixX(double max_x, double min_x, double max_y, double min_y,
			SubRoom* r, double bufx, double bufy, double dy, Routing* routing) const;
	vector<Point> PositionsOnFixY(double max_x, double min_x, double max_y, double min_y,
			SubRoom* r, double bufx, double bufy, double dx, Routing* routing) const;
	vector<Point> PossiblePositions(SubRoom* r, Routing * routing) const;
	void DistributeInSubRoom(SubRoom* r, int N, vector<Point> positions, int roomID, int* pid, Routing * routing)const;
	string writeParameter() const;

	void InitDistributor(string start_file);
	int Distribute(Building* building) const;
};

#endif	/* _PEDDISTRIBUTOR_H */

