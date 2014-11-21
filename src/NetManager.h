#ifndef TR_NET_MANAGER_H
#define TR_NET_MANAGER_H

#ifdef _WIN32
	#include <winsock2.h>
#else
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <sys/select.h>
	#include <netinet/in.h>
	
	#define SOCKET_ERROR -1
	typedef int32_t SOCKET;
	typedef sockaddr_in SOCKADDR_IN;
	typedef sockaddr SOCKADDR;
	typedef fd_set FD_SET;
#endif
#include <stdio.h>
#include "NetObjects.h"
#include "NetUtils.h"
#include "ThreadsUtils.h"

class NetManager
{
	public:
		NetManager(uint16_t Port);
		~NetManager();

		void ResetFD();
		SOCKET WaitForClient();

	private:
		uint16_t Port;
		SOCKET Socket;
		FD_SET fd;
		timeval Timeout;

		SOCKET CreateSocket(uint16_t Port);

};

#endif
