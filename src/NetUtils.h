#ifndef TR_NET_UTILS_H
#define TR_NET_UTILS_H

#include <stdio.h>
#ifdef _WIN32
	#include <winsock2.h>
#else
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <sys/select.h>
	#include <netinet/in.h>
	#include <unistd.h>
	
	#define SOCKET_ERROR -1
	typedef int32_t SOCKET;
	typedef sockaddr_in SOCKADDR_IN;
#endif
#include "CryptManager.h"

	struct Player
	{
		SOCKET socket;
		unsigned char RecvBuffer[128]; // No packet from client is ever bigger than this
		char Account[14];
		char Password[16];
		uint64_t AccUID;
	};

namespace Net
{
	void SendEncrypted(SOCKET s, const char* buff, int len);
	void SendPlain(SOCKET s, const char* buff, int len);
	int Receive(SOCKET s, char* buff, int len, bool clear=false, int size=128);
	void SetTimeout(SOCKET s, int time);
	void Close(SOCKET s);
	uint32_t NumericIP(SOCKET s);
	void PrintIP(uint32_t IP);
	uint32_t IPtoHex(char* ip);
}

#endif
