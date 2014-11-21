#include "SessionManager.h"

SessionManager* SessionManager::Pointer = 0;

SessionManager* SessionManager::Instance()
{
	return SessionManager::Pointer;
}

SessionManager* SessionManager::Create(uint32_t SessionTimeout, uint32_t ServerTimeout)
{
	if (SessionManager::Pointer == 0)
		SessionManager::Pointer = new SessionManager(SessionTimeout, ServerTimeout);
	return SessionManager::Pointer;
}

SessionManager::SessionManager(uint32_t SessionTimeout, uint32_t ServerTimeout)
{
	this->SessionTimeout = SessionTimeout;
	this->ServerTimeout = ServerTimeout;
}

SessionManager::~SessionManager() {}

uint64_t SessionManager::GenerateSession(const char* AccountName, uint64_t UID)
{
    DBManager::Instance()->lock();
	uint64_t SessionID;
	uint32_t SessionID1;
	uint32_t SessionID2;
	SessionID = GenerateUniqueKey();
	SessionID1 = (uint32_t)(SessionID & 0xFFFFFFFF);
	SessionID2 = (uint32_t)(SessionID >> 32);
	std::stringstream converter;
	std::stringstream query;
	std::string sSessionID1;
	std::string sSessionID2;
	std::string sUID;
	converter << SessionID1; converter >> sSessionID1; converter.clear();
	converter << SessionID2; converter >> sSessionID2; converter.clear();
	converter << UID; converter >> sUID; converter.clear();
	query << "INSERT INTO sessions (session_id1, session_id2, uid, account, checktime) VALUES(";
	query << sSessionID1 << ",";
	query << sSessionID2 << ",";
	query << sUID << ",";
	query << "'" << std::string(AccountName) << "',";
	query << "NULL" << ")";
	DBManager::Instance()->Query(query.str().c_str());
    DBManager::Instance()->unlock();
	// ToDo: Error Checking
	return SessionID;
}

uint64_t SessionManager::GenerateUniqueKey()
{
	uint64_t Key;
	
	uint32_t GenTime = Thread::GetTicks();
	Key = rand() + GenTime;
	for (int i = 0; i < 30; i++)
	{
		Key = (Key<<2) | (Key>>62);
		Key += rand();
		Key += GenTime / (i + 1);
	}

	return Key;
}

void SessionManager::WipeSessions()
{
	DBManager::Instance()->lock();
	DBManager::Instance()->Query("DELETE FROM sessions");
	DBManager::Instance()->unlock();
}

void SessionManager::WipeServers()
{
	DBManager::Instance()->lock();
	DBManager::Instance()->Query("DELETE FROM game_servers WHERE static='0'");
	DBManager::Instance()->unlock();
}

void SessionManager::RemoveExpiredSessions()
{
	std::stringstream converter;
	std::string Timeout;
	converter << SessionManager::SessionTimeout; converter >> Timeout; converter.clear();

	std::string Query = std::string("");
	Query += "DELETE FROM sessions WHERE (TIMESTAMPDIFF(SECOND, checktime,CURRENT_TIMESTAMP())) > ";
	Query += Timeout;
	DBManager::Instance()->lock();
	DBManager::Instance()->Query(Query.c_str());
	DBManager::Instance()->unlock();
}

void SessionManager::RemoveExpiredServers()
{
	std::stringstream converter;
	std::string Timeout;
	converter << SessionManager::ServerTimeout; converter >> Timeout; converter.clear();

	std::string Query = std::string("");
	Query += "DELETE FROM game_servers WHERE (TIMESTAMPDIFF(SECOND, check_time, CURRENT_TIMESTAMP())) > ";
	Query += Timeout;
	Query += " AND static='0'";
	DBManager::Instance()->lock();
	DBManager::Instance()->Query(Query.c_str());
	DBManager::Instance()->unlock();
}

// DELETE FROM sessions WHERE checktime > (CURRENT_TIMESTAMP() - 50)
