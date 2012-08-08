#ifndef ARGUMENTPARSER_H_
#define ARGUMENTPARSER_H_

#include <string>
#include <vector>
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
	int pTravisto;
	string pErrorLogFile;
	string pTrajOutputDir;
	unsigned int pSeed;
	int pMaxOpenmpThreads;

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
	int GetMaxOmpThreads() const;
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

	//TODO: check which methods are stil needed?
	unsigned int GetSeed() const;
	string GetTrajOutputDir() const;
	string GetErrorLogFile() const;
	string GetTrafficFile() const;
	string GetRoutingFile() const;

	void ParseArgs(int argc, char **argv);

	/**
	 * parse the initialization file
	 * @param inifile
	 */
	void ParseIniFile(string inifile);
};

#endif /*ARGPARSER_H_*/
