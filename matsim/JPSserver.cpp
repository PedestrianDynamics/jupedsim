/*
 * JPSserver.cpp
 *
 *  Created on: Apr 21, 2015
 *      Author: piccolo
 */

#include "JPSserver.h"

JPSserver::JPSserver()
{
     // TODO Auto-generated constructor stub

}

JPSserver::~JPSserver()
{
     // TODO Auto-generated destructor stub
}

Status JPSserver::reqMATSim2ExternHasSpace(ServerContext* context,
          const MATSim2ExternHasSpace* request,
          MATSim2ExternHasSpaceConfirmed* response)
{
}

Status JPSserver::reqMATSim2ExternPutAgent(ServerContext* context,
          const MATSim2ExternPutAgent* request,
          MATSim2ExternPutAgentConfirmed* response)
{
}

Status JPSserver::reqExternDoSimStep(ServerContext* context,
          const ExternDoSimStep* request, ExternDoSimStepReceived* response)
{
}

Status JPSserver::reqExternOnPrepareSim(ServerContext* context,
          const ExternOnPrepareSim* request,
          ExternOnPrepareSimConfirmed* response)
{
}

Status JPSserver::reqExternAfterSim(ServerContext* context,
          const ExternAfterSim* request, ExternAfterSimConfirmed* response)
{
}
