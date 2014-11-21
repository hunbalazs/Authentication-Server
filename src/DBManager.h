#ifndef TR_DB_MANAGER_H
#define TR_DB_MANAGER_H

#include <stdio.h>
#include <string>
#include <mutex>
#ifdef _WIN32
	#include <windows.h>
#else
	#include <stdlib.h>
#endif
#include <mysql.h>

#include "ThreadsUtils.h"

class DBManager
{
	public:
		static DBManager* Instance();
		static DBManager* Create(const char* dbHost, uint32_t dbPort, const char* dbUser, const char* dbPass, const char* dbName);
		
		~DBManager();

		bool Query(const char* query);
		MYSQL_RES* StoreResult();
		int NumRows(MYSQL_RES* result);
		MYSQL_ROW FetchRow(MYSQL_RES* result);
		void FreeResult(MYSQL_RES* result);
		int ValidatePlayer(const char* Name, const char* Password, uint64_t* UID);
        void lock();
        void unlock();

	protected:
		DBManager(const char* dbHost, uint32_t dbPort, const char* dbUser, const char* dbPass, const char* dbName);

	private:
		static DBManager* Pointer;

		MYSQL*		dbHandle;
		std::string	dbHost;
		uint32_t	dbPort;
		std::string	dbUser;
		std::string	dbName;
		std::string	dbPass;
		std::string	UsersTable;
        std::mutex  dbMutex;
};
#endif
