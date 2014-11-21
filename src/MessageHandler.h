#ifndef TR_MESSAGES_H
#define TR_MESSAGES_H

#include <vector>
#include "ThreadsUtils.h"
#include "NetUtils.h"
#include "NetObjects.h"
#include "DBManager.h"
#include "SessionManager.h"
#include "CryptManager.h"

namespace HandleMessage
{
	void ErrorAccBlocked(Player* player);						// By Server
	void AuthError(Player* player, TR_BYTE ErrorCode);			// By Server
	void AuthHello(Player* player);								// By Server
	bool AuthLogin(Player* player);								// By Client
	void AuthLoginOk(Player* player);							// By Server
	void AuthRequestServerList(Player* player);					// By Client
	void AuthServerListEx(Player* player);						// By Server
	uint8_t AuthSelectServer(Player* player);                   // By Client 
	void LastPacket(Player* player, uint8_t ServerID);          // By Server
}

#endif
