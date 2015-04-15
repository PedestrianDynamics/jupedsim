/*
 * AgentsSourcesManager.h
 *
 *  Created on: 14.04.2015
 *      Author: piccolo
 */

#ifndef AGENTSSOURCESMANAGER_H_
#define AGENTSSOURCESMANAGER_H_

#include <vector>
#include <memory>

//Forward declarations
class AgentsSource;

class AgentsSourcesManager
{
public:
     /**
      * Constructor
      */
     AgentsSourcesManager();

     /**
      * Destructor
      */
     virtual ~AgentsSourcesManager();

     /**
      * Make the class "runnable" by overloading the operator
      * @param value
      */
     void operator()(int value);

     /**
      *  Add a new agent source
      */
     void AddSource(std::shared_ptr<AgentsSource> src);

     /**
      * @return all sources
      */
     const std::vector<std::shared_ptr<AgentsSource> >& GetSources() const;

     //void operator()();

private:
     /// contain the sources
     std::vector<std::shared_ptr<AgentsSource>> _sources;
};

#endif /* AGENTSSOURCESMANAGER_H_ */
