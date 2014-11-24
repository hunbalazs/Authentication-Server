#include <stdio.h>
#include <stdint.h>
#include <thread>
#include <atomic>
#include "ThreadsUtils.h"
#include "INIParser.h"
#include "NetManager.h"
#include "NetUtils.h"
#include "MessageHandler.h"
#include "DBManager.h"
#include "SessionManager.h"
#include "CryptManager.h"

uint32_t Port;
uint32_t MaxClients;
uint32_t AcceptedLogins;
std::atomic<uint32_t> ActiveConns{0};
std::atomic<bool> ServerRunning{true};

void MaintenanceThread();
void PlayersThread();
void PlayerHandling(SOCKET Parameter);

int main(int argc, char *argv[])
{
	printf(" ==============================================================================\r\n");
	printf("                                  Tabula Rasa                                  \r\n");
	printf("                             Authentication Server                             \r\n\r\n");
	printf("  Version 0.4.1                                                     Salsa Crew \r\n");
	printf(" ==============================================================================\r\n");
	printf("Loading...\r\n");

#ifdef _WIN32
	WSADATA Winsock;
	WSAStartup(MAKEWORD(2,2), &Winsock);
#endif

	printf("Reading configuration file...\r\n\r\n");
	INIParser* Parser = new INIParser("config.ini");

	Port = Parser->GetInt("Auth Server", "port", 2106);
	printf("Authentication port: %i\r\n", Port);

	MaxClients = Parser->GetInt("Auth Server", "maxclients", 25);
	printf("Allowed simultaneous clients: %i\r\n", MaxClients);

	uint32_t sessiontimeout = Parser->GetInt("Auth Server",	"sessiontimeout", 50);
	printf("Time before sessions expire: %i\r\n", sessiontimeout);
	uint32_t servertimeout = Parser->GetInt("Auth Server",	"servertimeout", 120);
	printf("Time before servers expire: %i\r\n\r\n", servertimeout);

	std::string dbhost  = Parser->GetString("Auth Server Database",	"host",		"localhost");
	uint32_t    dbport  = Parser->GetInt   ("Auth Server Database",	"port",		10061);
	std::string dbname  = Parser->GetString("Auth Server Database",	"name",		"ir_authentication");
	std::string dbuser  = Parser->GetString("Auth Server Database",	"user",		"root");
	std::string dbpass  = Parser->GetString("Auth Server Database",	"password", "usbw");

	printf("Database Host: %s\r\n", dbhost.c_str());
	printf("Database Port: %u\r\n", dbport);
	printf("Database Name: %s\r\n\r\n", dbname.c_str());
	printf("Database User: %s\r\n", dbuser.c_str());
	printf("Database Pass: %s\r\n", dbpass.c_str());

	printf("Loading Database Manager...");
	DBManager::Create(dbhost.c_str(), dbport, dbuser.c_str(), dbpass.c_str(), dbname.c_str());
	printf("Done\r\n");

	printf("Loading Sessions Manager...");
	SessionManager::Create(sessiontimeout, servertimeout);
	printf("Done\r\n");

	printf("Loading Cryptography Manager...");
	CryptManager::Create();
	printf("Done\r\n\r\n");

	printf("Deleting old sessions...");
	SessionManager::Instance()->WipeSessions();
	printf("Done\r\n");
	printf("Deleting old servers...");
	SessionManager::Instance()->WipeServers();
	printf("Done\r\n\r\n");

	AcceptedLogins = 0;

	printf("Creating Threads...");

	std::thread playersThread{PlayersThread};   // start player handling thread
	std::thread maintThread{MaintenanceThread}; // start maintenance thread
	// TODO: handle error with try catch

	printf("Done\r\n\r\n");
	printf("Authentication Server running\r\n");
	
	if (argc == 2 && !strcmp(argv[1], "-hidden"))
	{
#ifdef _WIN32
		printf("Hidden mode enabled, going underground\r\n");
		HWND hWnd = GetConsoleWindow();
		ShowWindow(hWnd, SW_HIDE);
#else
		// TODO: daemonize
		printf("Hidden mode not supported on this OS\r\n");
#endif
	}
	printf("Type q to close the server\r\n");
	while(ServerRunning)
	{
		char c = getchar();
		if (c == 'q')
			ServerRunning = false;
	}
	printf("Finishing threads...");
	maintThread.join();
	playersThread.join();
	printf("Done\r\n");
	printf("Exiting Server...");
	return 0;
}

void PlayersThread()
{
	NetManager* NetMgr = new NetManager(Port);
	while(ServerRunning)
	{
		NetMgr->ResetFD();
		SOCKET NewClient = NetMgr->WaitForClient();
		if (NewClient != -1)
		{
			AcceptedLogins++;
			printf("Login %u: ", AcceptedLogins);
			Net::PrintIP(Net::NumericIP(NewClient));
			printf("\r\n");
			if (ActiveConns < MaxClients)
			{
				++ActiveConns;
				try
				{
					std::thread clientThread{PlayerHandling, NewClient};
					clientThread.detach(); // we should put it in a list or something
				}
				catch(...)
				{
					printf("Error creating Thread for player handling\r\n");
					Net::Close(NewClient);
				}
			}
			else
			{
				printf("Clients limit reached: %u vs %u\r\n", ActiveConns.load(), MaxClients);
				Net::Close(NewClient);
			}
		}
	}
}

void PlayerHandling(SOCKET Parameter)
{
	Player player;
	player.socket = Parameter;
	Net::SetTimeout(player.socket, 3); // 3 seconds of timeout for each recv

	HandleMessage::AuthHello(&player);
	if (Net::Receive(player.socket, (char*)player.RecvBuffer, 0x32, true) < 1)
	{ 
		printf("Error receiving AuthLogin packet\r\n");
#ifdef _WIN32
		int ret = WSAGetLastError();
		printf("WSA Last Error: %i\r\n", ret);
#endif
		Net::Close(player.socket);
        --ActiveConns;
        return; 
	}
	if(!HandleMessage::AuthLogin(&player))
    {
        printf("Failed to log in player\n\r");
        Net::Close(player.socket);
        --ActiveConns;
        return;
    }
	HandleMessage::AuthLoginOk(&player);
	// This part is the <different packets allowed>
	uint64_t start = Thread::GetTicks();
	uint8_t ServerID;
	while(true)
	{ 
		if ((Thread::GetTicks() - start) > (10*1000))
		{ 
			HandleMessage::AuthError(&player, AuthError::KICKED);
			printf("Time limit reached, kicking\r\n");
			Net::Close(player.socket);
            --ActiveConns;
            return; 
		}
		if (Net::Receive(player.socket, (char*)player.RecvBuffer, 0x1A, true) > 0)
		{
			if (*(uint16_t*)player.RecvBuffer != 0x1A)
			{ 
				printf("Error receiving Server request\r\n");
				Net::Close(player.socket);
                --ActiveConns;
                return; 
			}
			for(int i = 0; i < (0x1A - 2) / 8; i++)
			{
				CryptManager::Instance()->BFDecrypt((uint64_t*)&player.RecvBuffer[2+i*8], 
								 (uint64_t*)&player.RecvBuffer[2+i*8]);
			}
			if (player.RecvBuffer[2] == OPCode::AuthRSL)
			{
					start = Thread::GetTicks(); // Reset timeout
					HandleMessage::AuthRequestServerList(&player);
					HandleMessage::AuthServerListEx(&player);
			}
			if (player.RecvBuffer[2] == OPCode::AuthSS)
			{
				ServerID = HandleMessage::AuthSelectServer(&player);
				if (ServerID > 0)
				{
					HandleMessage::LastPacket(&player, ServerID);
					break;
				}
			}
		}
	}
	printf("Connection Closed: ");
    Net::PrintIP(Net::NumericIP(player.socket));
    printf("\r\n");
    --ActiveConns;
}

void MaintenanceThread()
{
	uint32_t SessionTimeout = SessionManager::Instance()->SessionTimeout;
	uint32_t ServerTimeout  = SessionManager::Instance()->ServerTimeout;

	// This loop makes a lot of not useful calls to the DB
	while(ServerRunning)
	{		
		SessionManager::Instance()->RemoveExpiredSessions();
		SessionManager::Instance()->RemoveExpiredServers();
		Thread::Wait(SessionTimeout * 500);
	}
}
