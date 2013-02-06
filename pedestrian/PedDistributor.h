/**
 * File:   PedDistributor.h
 *
 * Created on 12. October 2010, 10:52
 *
 * @section LICENSE
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * @section DESCRIPTION
 *
 *
 *
 */

#ifndef _PEDDISTRIBUTOR_H
#define	_PEDDISTRIBUTOR_H

#include <vector>
#include <string>

#include "../math/Distribution.h"
#include "../routing/Router.h"
#include "../geometry/Building.h"



/************************************************************
 StartDistributionRoom
 ************************************************************/
class StartDistributionRoom {
private:
	std::string _roomCaption;
	int _nPeds;

public:
	StartDistributionRoom();
	StartDistributionRoom(const StartDistributionRoom& orig);
	virtual ~StartDistributionRoom();

	// Getter-Funktionen
	std::string GetRoomCaption() const;
	int GetNumberOfPedestrian() const;

	// Setter-Funktionen
	void SetRoomCaption(std::string caption);
	void SetNumberOfPedestrians(int N);

};

class StartDistributionSubroom : public StartDistributionRoom {
private:
	int _subroomID;

public:
	StartDistributionSubroom();
	StartDistributionSubroom(const StartDistributionSubroom& orig);
	virtual ~StartDistributionSubroom();
	// Getter-Funktionen
	int GetSubroomID() const;

	// Setter-Funktionen
	void SetSubroomID(int i);
};

/************************************************************
 PedDistributor
 ************************************************************/
class PedDistributor {
private:
	Distribution* _v0; // Gauss - Verteilung für v0
	Distribution* _Bmax;
	Distribution* _Bmin;
	Distribution* _Atau;
	Distribution* _Amin;
	Distribution* _Tau;
	std::vector<StartDistributionRoom> _start_dis; // ID startraum, subroom und Anz
	std::vector<StartDistributionSubroom> _start_dis_sub; // ID startraum, subroom und Anz
	std::string _initialisationFile; // store the file for later user

	// find aped in a subroom and delete him
	bool FindPedAndDeleteFromRoom(Building* building,Pedestrian*ped) const;

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
	std::vector<Point> PositionsOnFixX(double max_x, double min_x, double max_y, double min_y,
			SubRoom* r, double bufx, double bufy, double dy) const;
	std::vector<Point> PositionsOnFixY(double max_x, double min_x, double max_y, double min_y,
			SubRoom* r, double bufx, double bufy, double dx) const;
	std::vector<Point> PossiblePositions(SubRoom* r) const;
	void DistributeInSubRoom(SubRoom* r, int N, std::vector<Point>& positions, int roomID, int* pid)const;
	std::string writeParameter() const;

	void InitDistributor(std::string start_file);
	int Distribute(Building* building) const;
};

#endif	/* _PEDDISTRIBUTOR_H */

