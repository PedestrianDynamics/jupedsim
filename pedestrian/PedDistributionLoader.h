//
// Created by laemmel on 31.03.16.
//

#ifndef JPSCORE_PEDDISTRIBUTIONLOADER_H
#define JPSCORE_PEDDISTRIBUTIONLOADER_H

#include "PedDistributor.h"

class PedDistributionLoader {

public:
     virtual ~PedDistributionLoader(){};

    virtual bool LoadPedDistribution(std::vector<std::shared_ptr<StartDistribution>> &startDis,
                                     std::vector<std::shared_ptr<StartDistribution>> &startDisSub,
                                     std::vector<std::shared_ptr<AgentsSource>> &startDisSources) = 0;
    virtual ~PedDistributionLoader() {};
};

#endif //JPSCORE_PEDDISTRIBUTIONLOADER_H

