/**
 * \file        travistoclient.cpp
 * \date        jul 4, 2014
 * \version     v0.7
 * \copyright   <2009-2015> Forschungszentrum Jülich GmbH. all rights reserved.
 *
 * \section license
 * this file is part of jupedsim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the gnu lesser general public license as published by
 * the free software foundation, either version 3 of the license, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. see the
 * gnu general public license for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with JuPedSim. if not, see <http://www.gnu.org/licenses/>.
 *
 * \section Description

 *
 **/


/********* include files ******************************************************/

#include "TraVisToClient.h"

#include "general/Macros.h"

#include <iostream>

TraVisToClient::TraVisToClient(std::string hostname, unsigned short port)
{
     _hostname=hostname;
     _port = port;
     _isConnected = false;
     createConnection();
}

TraVisToClient::~TraVisToClient()
{
     if (_isConnected) close();
}

/// send datablock to the server

void TraVisToClient::sendData(const char* data)
{
     // first create a new connection, in the case the last one was lost/close
     if (!_isConnected) {
          createConnection();
          //FIXME: queue messsage in a vector
          // msgQueue.push_back(data);
          return;

     }
     char msgSizeStr[10];
     int msgSize = (int)strlen(data);
     sprintf(msgSizeStr, "%d\n", msgSize);

     /* check if parameters are valid */
     if (NULL == data) {
          fprintf(stderr, "invalid message buffer!");
          fprintf(stderr, "leaving sendMessage()");
          _isConnected = false;
          return;
     }


     //  do until queue empty for()


     /*send the length of the message*/
     int msgsize = strlen(msgSizeStr);
     if (msgsize != send(_tcpSocket, (const char *) msgSizeStr, strlen(msgSizeStr), 0)) {
          fprintf(stderr, "sending message Size failed");
          fprintf(stderr, "leaving sendMessage()");
          _isConnected = false;
          return;
     }

     /* now send the message */
     if (msgSize != send(_tcpSocket, (const char *) data, msgSize, 0)) {
          fprintf(stderr, "sending message failed");
          fprintf(stderr, "leaving sendMessage()");
          _isConnected = false;
          return;
     }

     // end do
}

/// close the client (end the connection)

void TraVisToClient::close()
{
     if (_isConnected) {
          /* all things are done, so shutdown the connection */
          if (!shutdownAndCloseSocket(_tcpSocket)) {
               fprintf(stderr, "shutdown and close socket failed!");
               stopSocketSession();
               fprintf(stderr, "leaving main() with error");
               return;
          }

          /* stop the socket session */
          stopSocketSession();
     }

}

void TraVisToClient::createConnection()
{

     /* start the socket session */
     if (!startSocketSession()) {
          fprintf(stderr, "startSocketSession() failed!\n");
          fprintf(stderr, "socket creation failed for host [%s] on port [%d]!\n",_hostname.c_str(),_port);
          exit(EXIT_FAILURE);
     }

     /* create a new socket and connect the socket to the given service */
     if (INVALID_SOCKET == (_tcpSocket = createClientSocket(_hostname.c_str(), _port))) {
          fprintf(stderr, "\nsocket creation failed for host [%s] on port [%d]!\n",_hostname.c_str(),_port);
          stopSocketSession();
          exit(EXIT_FAILURE);
     }
     _isConnected = true;
}

/********* function definitions **************************************/

/**
 *  This function returns an integer (unsigned long) representation
 *  of an IP address, given as a FQN or a dotted IP address.
 *
 *  @param  hostName     name of a host as FQN or dotted IP address
 *
 *  @return  The return value is an integer value of a dotted IP address if
 *           the hostname exists (valid DNS entry) or the hostname is given
 *           as a dotted IP address. Otherwise the function returns
 *           @c INADDR_NONE.
 */
unsigned long
TraVisToClient::lookupHostAddress(const char *hostName)
{
     unsigned long addr; /* inet address of hostname */
     struct hostent *host; /* host structure for DNS request */

     dtrace("entering lookupHostAddress()");

     if (NULL == hostName) {
          derror("invalid parameter");
          dtrace("leaving lookupHostAddress()");
          return (INADDR_NONE);
     }

     dtrace("looking for host %s", hostName);

     addr = inet_addr(hostName);

     if (INADDR_NONE == addr) {
          /* hostName isn't a dotted IP, so resolve it through DNS */
          host = gethostbyname(hostName);
          if (NULL != host) {
               addr = *((unsigned long *) host->h_addr);
          }
     }

     dtrace("leaving lookupHostAddress()");
     return (addr);
}
/******** end of function lookupHostAddress **************************/

/**
 *  This function creates a new client internet domain socket (TCP/IP)
 *  and connects to a service on an internet host.
 *
 *  @param  serverName   hostname of a server
 *  @param  portNumber   port number of service
 *
 *  @return  The function returns a socket handle if the socket can be created
 *           and connected to a service. If an error occurred, the function
 *           returns @c INVALID_SOCKET.
 */
socket_t
TraVisToClient::createClientSocket(const char *serverName, unsigned short portNumber)
{
     unsigned long ipAddress; /* internet address */
     struct sockaddr_in srvAddr; /* server's internet socket address */
     socket_t sock; /* file descriptor for client socket */

     dtrace("entering createClientSocket()");

     /* get the IP address of the server host */
     if (INADDR_NONE == (ipAddress = lookupHostAddress(serverName))) {
          derror("lookupHostAddress() failed");
          dtrace("leaving createClientSocket() with INVALID_SOCKET");
          return (INVALID_SOCKET);
     }

     dtrace("trying to connect %s on port %hu", serverName, portNumber);

     /* create the client socket */
     if (INVALID_SOCKET == (sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP))) {
          derror("socket creation failed");
          dtrace("leaving createClientSocket() with INVALID_SOCKET");
          return (INVALID_SOCKET);
     }

     /* fill the server address structure */
     memset(&srvAddr, 0, sizeof (srvAddr));
     srvAddr.sin_family = AF_INET;
     srvAddr.sin_port = htons(portNumber);
     srvAddr.sin_addr.s_addr = ipAddress;

     /* try to connect to the server socket */
     if (SOCKET_ERROR == connect(sock, (struct sockaddr *) & srvAddr, sizeof (srvAddr))) {
          derror("connect() failed");
          //FIXME
          //closesocket(tcpSocket);
          dtrace("leaving createClientSocket() with INVALID_SOCKET");
          return (INVALID_SOCKET);
     }

     dtrace("leaving createClientSocket()");
     return (sock);
}
/******** end of function createClientSocket *************************/

/**
 *  This function creates a new server internet domain socket (TCP/IP).
 *
 *  @param  portNumber   port number on which to listen for incoming packets
 *
 *  @return  The function returns a socket handle if the socket can be created.
 *           If an error occurs, the function returns @c INVALID_SOCKET.
 */
socket_t
TraVisToClient::createServerSocket(unsigned short portNumber)
{
     struct sockaddr_in srvAddr; /* server's internet socket address */
     socket_t sock; /* file descriptor for server socket */

     dtrace("entering createServerSocket()");

     /* create the server socket */
     if (INVALID_SOCKET == (sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP))) {
          derror("socket creation failed");
          dtrace("leaving createServerSocket() with INVALID_SOCKET");
          return (INVALID_SOCKET);
     }

     /* fill the server address structure */
     /* first of all, zero srvAddr, so that we have a defined status */
     memset(&srvAddr, 0, sizeof (srvAddr));
     srvAddr.sin_family = AF_INET;
     srvAddr.sin_port = htons(portNumber);
     srvAddr.sin_addr.s_addr = htonl(INADDR_ANY);

     /* try to bind socket to the specified server port */
     if (SOCKET_ERROR == bind(sock, (struct sockaddr *) & srvAddr, sizeof (srvAddr))) {
          derror("bind() failed!");
          //FIXME:
          //closesocket(tcpSocket);
          dtrace("leaving createServerSocket() with INVALID_SOCKET");
          return (INVALID_SOCKET);
     }

     if (SOCKET_ERROR == listen(sock, QUEUE_LENGTH)) {
          derror("listen() failed!");
          shutdownAndCloseSocket(sock);
          dtrace("leaving createServerSocket() with INVALID_SOCKET");
          return (INVALID_SOCKET);
     }

     dtrace("server started at port %hu", portNumber);
     dtrace("leaving createServerSocket()");
     return (sock);
}
/******** end of function createServerSocket *************************/

/**
 *  This function sends a message of a given size to a communication
 *  partner via a connected socket.
 *
 *  @param  sock         socket for sending the message
 *  @param  msg          pointer to the message buffer which should be sent
 *  @param  msgSize      size of the message buffer
 *
 *  @return  The function returns @c true if the message was sent to the server,
 *           otherwise the function returns @c false.
 */
bool
TraVisToClient::sendMessage(socket_t sock, const void *msg, int msgSize)
{
     dtrace("entering sendMessage()");

     /* check if parameters are valid */
     if (NULL == msg) {
          derror("invalid message buffer!");
          dtrace("leaving sendMessage()");
          return (false);
     }

     if (0 >= msgSize) {
          derror("invalid message size %d", msgSize);
          dtrace("leaving sendMessage()");
          return (false);
     }

     dtrace("sending message of size %d", msgSize);

     /* now send the message */
     if (msgSize != send(sock, (const char *) msg, msgSize, 0)) {
          derror("sending message failed");
          dtrace("leaving sendMessage()");
          return (false);
     }

     dtrace("leaving sendMessage()");
     return (true);
}
/******** end of function sendMessage ********************************/

/**
 *  This functions receives a message from a communication partner and
 *  stores it into a given message buffer.
 *  The function blocks until all bytes have been successfully received,
 *  or an error occurred.
 *
 *  @param       sock       socket for receiving the message
 *  @param[out]  msg        message buffer for receiving the message
 *  @param       msgSize    size of the buffer to contain the message
 *
 *  @return  The function returns @c true if all things went well. Otherwise
 *           the function returns @c false.
 */
bool
TraVisToClient::receiveMessage(socket_t sock, void *msg, int msgSize)
{
     char *msgPart; /* pointer to the memory for receiving the message */
     int toReceive; /* number of bytes to receive */
     int received; /* number of bytes totally received */
     int nBytes; /* number of bytes currently received */

     dtrace("entering receiveMessage()");

     /* check if parameters are valid */
     if (NULL == msg) {
          derror("invalid message buffer!");
          dtrace("leaving receiveMessage()");
          return (false);
     }

     if (0 >= msgSize) {
          derror("invalid message size!");
          dtrace("leaving receiveMessage()");
          return (false);
     }

     msgPart = (char *) msg;
     received = 0;

     dtrace("trying to receive a message of size %d", msgSize);

     /* start receiving bytes from server until complete message is received */
     do {
          toReceive = msgSize - received;
          nBytes = recv(sock, msgPart, toReceive, 0);
          switch (nBytes) {
          case SOCKET_ERROR: /* error occurred */
               derror("error during message receipt");
               dtrace("leaving receiveMessage()");
               return (false);
          case 0: /* connection has been closed */
               derror("remote host has closed the connection");
               dtrace("leaving receiveMessage()");
               return (false);
          default: /* some bytes have been received */
               dtrace("received %d bytes of message", nBytes);
               received += nBytes;
               msgPart += nBytes;
               break;
          }
     } while (received != msgSize);

     dtrace("received message of size %d", received);

     dtrace("leaving receiveMessage()");
     return (true);
}
/******** end of function receiveMessage *****************************/

/**
 *  This function shuts down and closes a given socket.
 *
 *  @param  sock         socket to be closed
 *
 *  @return  if all things went ok, this function returns @c true, otherwise
 *           @c false
 */
bool
TraVisToClient::shutdownAndCloseSocket(socket_t sock)
{
     bool status = true;

     dtrace("entering shutdownAndCloseSocket()");

     if (SOCKET_ERROR == shutdown(sock, SHUT_RDWR)) {
          derror("shutdown() failed");
          status = false;
     }

     //FIXME:
     //if (SOCKET_ERROR == closesocket(tcpSocket)) {
     //  derror("closesocket() failed");
     //  status = false;
     //}

     dtrace("leaving shutdownAndCloseSocket()");
     return (status);
}

/******** end of function shutdownAndCloseSocket *********************/

void
TraVisToClient::_printErrorMessage(void)
{
}


#ifdef _WIN32

/**
 *  This function initializes the Win32 Socket API.
 *
 *  @return  if all things went ok, this function returns @c true, otherwise
 *           @c false
 */
bool
TraVisToClient::_startWin32SocketSession(void)
{
     WORD requestedVersion;
     WSADATA wsaData;

     dtrace("entering _startWin32SocketSession()");

     requestedVersion = MAKEWORD(WS_MAJOR_VERSION, WS_MINOR_VERSION);

     if (0 != WSAStartup(requestedVersion, &wsaData)) {
          derror("WSAStartup() failed");
          dtrace("leaving _startWin32SocketSession() with error");
          return (false);
     }

     /* Confirm that the Windows Socket DLL supports 1.1. */
     /* Note that if the DLL supports versions greater    */
     /* than 1.1 in addition to 1.1, it will still return */
     /* 1.1 in wVersion since that is the version we      */
     /* requested.                                        */

     if (WS_MINOR_VERSION != LOBYTE(wsaData.wVersion)
               || WS_MAJOR_VERSION != HIBYTE(wsaData.wVersion)) {
          derror("Windows Socket DLL does not support the requested version");
          _stopWin32SocketSession();
          dtrace("leaving _startWin32SocketSession() with error");
          return (false);
     }

     WSASetLastError(0); /* reset the error code */

     dtrace("leaving _startWin32SocketSession()");
     return (true);
}
/******** end of function _startWin32SocketSession *******************/

/**
 *  This function terminates the Win32 Socket API.
 *  No future API calls are allowed.
 */
void
TraVisToClient::_stopWin32SocketSession(void)
{
     dtrace("entering _stopWin32SocketSession()");

     if (SOCKET_ERROR == WSACleanup()) {
          derror("WSACleanup() failed");
     }

     dtrace("leaving _stopWin32SocketSession()");
     return;
}
/******** end of function _stopWin32SocketSession ********************/

#endif /* _WIN32 */

