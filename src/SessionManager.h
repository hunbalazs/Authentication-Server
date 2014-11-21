// ToDo
// Add IP to sessions for checking
// Maybe change struct for class

#ifndef TR_SESSION_MANAGER_H
#define TR_SESSION_MANAGER_H

#ifdef _WIN32
	#include <windows.h>
#endif
#include <math.h>
#include <stdio.h>
#include <sstream>
#include <string>
#include "DBManager.h"
#include "ThreadsUtils.h"

class SessionManager
{
	public:
		static SessionManager* Instance();
		static SessionManager* Create(uint32_t SessionTimeout, uint32_t ServerTimeout);
		
		~SessionManager();

		uint64_t GenerateSession(const char* AccountName, uint64_t UID);
		void WipeSessions();
		void WipeServers();
		void RemoveExpiredSessions();
		void RemoveExpiredServers();

		uint32_t SessionTimeout;
		uint32_t ServerTimeout;

	protected:
		SessionManager(uint32_t SessionTimeout, uint32_t ServerTimeout);

	private:
		static SessionManager* Pointer;

		uint64_t GenerateUniqueKey();
};

#endif
