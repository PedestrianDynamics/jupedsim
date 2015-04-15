/*
 * AgentSource.h
 *
 *  Created on: 14.04.2015
 *      Author: piccolo
 */

#ifndef AGENTSOURCE_H_
#define AGENTSOURCE_H_

#include <string>
#include <vector>

class Pedestrian;
class OutputHandler;

// external variables
extern OutputHandler* Log;


class AgentsSource
{
public:
     /**
      * Constructor
      */
     AgentsSource(int id,std::string caption,int max_agents,int group_id,int frequency);

     /**
      * Destructor
      */
     virtual ~AgentsSource();

     /**
      * Add a new agent to this source
      * @param ped
      */
     void Add(Pedestrian* ped);

     /**
      * Generate a number of agents, based on the frequency given in the constructor.
      * No agents are generated if the the maximum (_maxAgents) is reached.
      * @see _maxAgents
      * @param ped
      */
     void GenerateByFrequency(std::vector<Pedestrian*>& ped);

     /**
      * @return the number of agents remaining
      */
     int GetPoolSize() const;

     /**
      * Print relevant information
      */
     void Dump() const;

     int GetGroupID() const;
     int GetAgentsGenerated() const;
     void SetAgentsGenerated(int agentsGenerated);
     const double* GetBoundaries() const;
     const std::string& GetCaption() const;
     int GetFrequency() const;
     int GetGroupId() const;
     int GetId() const;
     int GetMaxAgents() const;


private:
     int _id=-1;
     int _frequency=0;
     int _maxAgents=0;
     int _agentsGenerated=0;
     int _groupID=-1;
     double _boundaries [4] = {0,0,0,0};
     std::string _caption="no caption";
     std::vector<Pedestrian*> _agents;

};

#endif /* AGENTSOURCE_H_ */

