/**
 * @file    main.cpp
 * @author  U.Kemloh, A.Portz
 * @version 0.4
 * Created on: Apr 20, 2019
 * Copyright (C) <2009-2012>
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
#ifndef ARGUMENTPARSER_H_
#define ARGUMENTPARSER_H_

#include <string>
#include <vector>
#include "Macros.h"
using std::string;
using std::vector;

class OutputHandler;
extern OutputHandler* Log;

class ArgumentParser {
private:
	string pRoutingFilename; /// routing information
	string pTrafficFilename; /// traffic information (closed doors,...)
	string pNumberFilename; /// initial distribution and person specifications
	int pSolver; /// solver for the differential equation
	string pGeometryFilename;
	string pPathwayfile; // saving pedestrian path
	double pTmax; // maximale Simulationszeit
	double pdt; // Zeitschritt
	double pfps; //frame rate
	int pExitStrategy; // Strategie zur Richtungswahl (v0)
	//int pRandomize; // Verteilung inerhalb des Raums
	int pRouter; // Routing-Strategie
	bool pLinkedCells; // use of linked-cells neighbourhood list
	double pLinkedCellSize; // cell size of the linkedcell (default to 2.2m)
	double pV0Mu; // mu für die Normalverteilung von v0
	double pV0Sigma; // sigma für die Normalverteilung von v0
	double pBmaxMu; // mu für die Normalverteilung von b_max
	double pBmaxSigma; // sigma für die Normalverteilung von b_max
	double pBminMu; // mu für die Normalverteilung von b_min
	double pBminSigma; // sigma für die Normalverteilung von b_min
	double pAtauMu; // mu für die Normalverteilung von a_tau
	double pAtauSigma; // sigma für die Normalverteilung von a_tau
	double pAminMu; // mu für die Normalverteilung von a_min
	double pAminSigma; // sigma für die Normalverteilung von a_min
	double pTauMu;
	double pTauSigma;
	double pNuPed;
	double pNuWall;
	double pIntPWidthPed;
	double pIntPWidthWall;
	double pMaxFPed;
	double pMaxFWall;
	double pDistEffMaxPed;
	double pDistEffMaxWall;
	int pLog;
	//int pTravisto;
	string pErrorLogFile;
	//string pTrajOutputDir;
	unsigned int pSeed;
	int pMaxOpenMPThreads;
	FileFormat pFormat;
	int pPort;
	string pHostname;
	string pTrajectoriesFile;

	// private Funktionen
	void Usage();

public:
	// Konstruktor
	ArgumentParser(); // gibt die Programmoptionen aus

	// Getter-Funktionen
	string GetPersonsFilename() const;
	string GetPathwayFile() const;
	int GetSolver() const;
	double GetTmax() const;
	double Getdt() const;
	string GetGeometryFilename() const;
	int GetExitStrategy() const;
	int GetRandomize() const;
	bool IsOnline() const;
	int GetRoutingStrategy() const;
	bool GetLinkedCells() const;
	double GetLinkedCellSize() const;
	int GetMaxOpenMPThreads() const;
	double GetV0Mu() const;
	double GetV0Sigma() const;
	double GetBmaxMu() const;
	double GetBmaxSigma() const;
	double GetBminMu() const;
	double GetBminSigma() const;
	double GetAtauMu() const;
	double GetAtauSigma() const;
	double GetAminMu() const;
	double GetAminSigma() const;
	double GetNuPed() const;
	double GetNuWall() const;
	double GetIntPWidthPed() const;
	double GetIntPWidthWall() const;
	double GetMaxFPed() const;
	double GetMaxFWall() const;
	double GetDistEffMaxPed() const;
	double GetDistEffMaxWall() const;
	double GetTauMu() const;
	double GetTauSigma() const;
	int GetLog() const;
	int GetTravisto() const;
	int GetTrajektorien() const;
	double Getfps() const;
	FileFormat GetFileFormat() const;
	const string& GetHostname() const;
	void SetHostname(const string& hostname);
	int GetPort() const;
	void SetPort(int port);
	const string& GetTrajectoriesFile() const;
	void SetTrajectoriesFile(const string& trajectoriesFile);
	string GetErrorLogFile() const;
	string GetTrafficFile() const;
	string GetRoutingFile() const;

	//TODO: check which methods are still needed?
	unsigned int GetSeed() const;

	void ParseArgs(int argc, char **argv);

	/**
	 * parse the initialization file
	 * @param inifile
	 */
	void ParseIniFile(string inifile);
};

#endif /*ARGPARSER_H_*/
