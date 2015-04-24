/*
 * JPSclient.h
 *
 *  Created on: Apr 21, 2015
 *      Author: piccolo
 */

#ifndef MATSIM_JPSCLIENT_H_
#define MATSIM_JPSCLIENT_H_


#include <grpc/grpc.h>
#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <grpc++/server_context.h>
#include <grpc++/server_credentials.h>
#include <grpc++/status.h>
#include "MATSimInterface.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
//using helloworld::HelloRequest;
//using helloworld::HelloReply;
//using helloworld::Greeter;
using namespace hybrid;


class JPSclient
{
public:
     JPSclient();
     virtual ~JPSclient();
};

#endif /* MATSIM_JPSCLIENT_H_ */
