#include "DBManager.h"

DBManager* DBManager::Pointer = 0;

DBManager* DBManager::Instance()
{
	return DBManager::Pointer;
}

DBManager* DBManager::Create(const char* dbHost, uint32_t dbPort, const char* dbUser, const char* dbPass, const char* dbName)
{
	if (DBManager::Pointer == 0)
		DBManager::Pointer = new DBManager(dbHost, dbPort, dbUser, dbPass, dbName);
	return DBManager::Pointer;
}

DBManager::DBManager(const char* dbHost, uint32_t dbPort, const char* dbUser, const char* dbPass, const char* dbName)
{
	this->dbHost = std::string(dbHost);
	this->dbPort = dbPort;
	this->dbUser = std::string(dbUser);
	this->dbPass = std::string(dbPass);
	this->dbName = std::string(dbName);

	this->dbHandle = mysql_init(0);
	MYSQL *dbHandleErr = dbHandle;
	this->dbHandle = mysql_real_connect(dbHandle, this->dbHost.c_str(), this->dbUser.c_str(),
										this->dbPass.c_str(), this->dbName.c_str(), this->dbPort, NULL, 0);
	if (this->dbHandle == NULL)
	{
		printf("MySQL: %s", mysql_error(dbHandleErr));
        throw("MySQL: mysql_real_connect(...) failed!");
	}
}

DBManager::~DBManager()
{
	if (this->dbHandle != NULL)
		mysql_close(this->dbHandle);
}

void DBManager::lock()
{
    dbMutex.lock();
}

void DBManager::unlock()
{
    dbMutex.unlock();
}

bool DBManager::Query(const char* query)
{
	if (mysql_query(this->dbHandle, query))
	{
		printf("MySQL: Error with query\n");
        printf("MySQL: %s", mysql_error(this->dbHandle));
		return false;
	}

	return true;
}

MYSQL_RES* DBManager::StoreResult()
{
	return mysql_store_result(this->dbHandle);
}

int DBManager::NumRows(MYSQL_RES* result)
{
	int rows = (int)mysql_num_rows(result);
	return rows;
}

MYSQL_ROW DBManager::FetchRow(MYSQL_RES* result)
{
	MYSQL_ROW row = mysql_fetch_row(result);
	return row;
}

void DBManager::FreeResult(MYSQL_RES* result)
{
	mysql_free_result(result);
}

int DBManager::ValidatePlayer(const char* Name, const char* Password, uint64_t* UID)
{
	std::string Query;
	Query = "SELECT * FROM `users` WHERE username='";
	Query += Name;
	Query += "' AND password='";
	Query += Password;
	Query += "'";

	if (mysql_query(this->dbHandle, Query.c_str()))
	{
		printf("MySQL: Error with query\n");
		return -1;
	}

	MYSQL_RES* res = mysql_store_result(this->dbHandle);
	uint64_t rows = mysql_num_rows(res);
	if (rows > 0)
	{
		MYSQL_ROW row = mysql_fetch_row(res);
		if (atoi(row[4]) == 1) // Account Blocked
		{
            mysql_free_result(res);
            return 1;
        }
		if (atoi(row[5]) == 1) // Already logged
		{
            mysql_free_result(res);
            return 2;
        }
		*UID = atoi(row[0]);
		mysql_free_result(res);
		return 0;
	}
	mysql_free_result(res); 
	return -1;
}
