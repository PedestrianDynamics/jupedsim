/*
 * HybridSimulationManager.cpp
 *
 *  Created on: Apr 20, 2015
 *      Author: piccolo
 */

#include "HybridSimulationManager.h"
#include <thread>
#include <functional>

HybridSimulationManager::HybridSimulationManager()
{
     // TODO Auto-generated constructor stub

}

HybridSimulationManager::~HybridSimulationManager()
{
     // TODO Auto-generated destructor stub
}

bool HybridSimulationManager::Init()
{

}

bool HybridSimulationManager::Run()
{
     std::thread t1(&HybridSimulationManager::RunClient, this);
     std::thread t2(&HybridSimulationManager::RunServer, this);
     t1.join();
     t2.join();
     return true;

}

bool HybridSimulationManager::RunClient()
{
     //check the message queue and send
     do
     {

     } while (!_shutdown);

     return true;
}

bool HybridSimulationManager::RunServer()
{
     //check the message queue and send
     do
     {

     } while (!_shutdown);

     return true;
}

void HybridSimulationManager::Shutdown()
{
     _shutdown = true;
}

/*
//
// blocking_tcp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>

using boost::asio::ip::tcp;

const int max_length = 1024;

typedef boost::shared_ptr<tcp::socket> socket_ptr;

void session(socket_ptr sock)
{
  try
  {
    for (;;)
    {
      char data[max_length];

      boost::system::error_code error;
      size_t length = sock->read_some(boost::asio::buffer(data), error);
      if (error == boost::asio::error::eof)
        break; // Connection closed cleanly by peer.
      else if (error)
        throw boost::system::system_error(error); // Some other error.

      boost::asio::write(*sock, boost::asio::buffer(data, length));
    }
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception in thread: " << e.what() << "\n";
  }
}

void server(boost::asio::io_service& io_service, short port)
{
  tcp::acceptor a(io_service, tcp::endpoint(tcp::v4(), port));
  for (;;)
  {
    socket_ptr sock(new tcp::socket(io_service));
    a.accept(*sock);
    boost::thread t(boost::bind(session, sock));
  }
}

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 2)
    {
      std::cerr << "Usage: blocking_tcp_echo_server <port>\n";
      return 1;
    }

    boost::asio::io_service io_service;

    using namespace std; // For atoi.
    server(io_service, atoi(argv[1]));
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
*/
