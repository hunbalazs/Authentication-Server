#include "MessageHandler.h"

void HandleMessage::ErrorAccBlocked(Player* player)
{
	Packet::ErrorAccBlocked packet;
	packet.PacketLength = 0x07;
	packet.OPCode		= OPCode::AuthEAB;
	packet.ErrorID		= 0x00;

	Net::SendEncrypted(player->socket, (char*)&packet, 0x07);
	return;
}

void HandleMessage::AuthError(Player* player, TR_BYTE ErrorCode)
{
	Packet::AuthError packet;
	packet.PacketLength = 0x07;
	packet.OPCode		= OPCode::AuthE;
	packet.ErrorID		= ErrorCode;

	Net::SendEncrypted(player->socket, (char*)&packet, 0x07);
	return;
}

void HandleMessage::AuthHello(Player* player)
{
	Packet::AuthHello packet;
	packet.PacketLength = 0x0B;
	packet.OPCode		= OPCode::AuthH;
	packet.Unknown1		= 0xDEAD0E01;
	packet.Unknown2		= 0x00;

	Net::SendPlain(player->socket, (char*)&packet, 0x0B);
	return;
}

bool HandleMessage::AuthLogin(Player* player)
{
	CryptManager::Instance()->lock();
	for(int i = 0; i < (0x32 - 2) / 8; i++)
	{
		CryptManager::Instance()->BFDecrypt((uint64_t*)&player->RecvBuffer[2+i*8], 
						 (uint64_t*)&player->RecvBuffer[2+i*8+4]);
	}

	Packet::AuthLogin packet;
	memcpy(&packet, player->RecvBuffer, sizeof(Packet::AuthLogin));

	if (packet.OPCode != OPCode::AuthL)
    {
        printf("Wrong opcode: not AuthL\n");
        printf("%02x != %02x\n", packet.OPCode, OPCode::AuthL);
        return false;
    }

	// Check the GameID and CDKey, both are hardcoded
	if (packet.GameID != 0x08 || packet.CDKey != 0x01)
    {
        printf("Wrong GameID or CDKey\n");
        return false;
    }

	CryptManager::Instance()->TRDecrypt((uint8_t*)&packet.UserData, 30);
    CryptManager::Instance()->unlock();
	memcpy(player->Account,	(char*)packet.UserData, 14);
	memcpy(player->Password, ((char*)packet.UserData + 14), 16);

    DBManager::Instance()->lock();
	int LoginOK = DBManager::Instance()->ValidatePlayer(
                            (char*)player->Account,
                            CryptManager::Instance()->GenMD5(
                                        (char*)player->Password,
                                        strlen(player->Password)).c_str(),
                            &player->AccUID);
    DBManager::Instance()->unlock();

	switch (LoginOK)
	{
		case -1: // Account not found
			printf("Account not found: %s\n", player->Account);
			HandleMessage::AuthError(player, AuthError::INVALID_PASSWORD);
			return false;
			break;
		case 0: // OK
			break;
		case 1: // Account blocked
			printf("Account blocked: %s\n", player->Account);
			HandleMessage::ErrorAccBlocked(player);
			return false;
			break;
		case 2: // Account already logged
			printf("Account already logged: %s\n", player->Account);
			HandleMessage::AuthError(player, AuthError::ALREADY_LOGGED_IN);
			return false;
			break;
	}
	// Logged
	return true;
}

void HandleMessage::AuthLoginOk(Player* player)
{
	// related to subscription, need info
	Packet::AuthLoginOk packet;
	packet.PacketLength = 0x28;
	packet.OPCode		= OPCode::AuthLO;
	packet.Unknown1		= 0x00;
	packet.Unknown2		= 0x00;
	packet.Unknown3		= 0x00;
	packet.Unknown4		= 0x00;
	packet.Unknown5		= 0x00;
	packet.Unknown6		= 0x00;
	packet.Unknown7		= 0x00;
	packet.Unknown8		= 0x00;
	packet.Unknown9		= 0x00;
	packet.Unknown10	= 0x00;

	Net::SendEncrypted(player->socket, (char*)&packet, 0x28);
	return;
}

void HandleMessage::AuthRequestServerList(Player* player)
{
		Packet::AuthRequestServerList packet;
		memcpy(&packet, player->RecvBuffer, sizeof(Packet::AuthRequestServerList));
		// Add something here? 
		return;
}

void HandleMessage::AuthServerListEx(Player* player)
{
	MYSQL_ROW row;
    DBManager::Instance()->lock();
	DBManager::Instance()->Query("SELECT * FROM `game_servers`");
	MYSQL_RES* result = DBManager::Instance()->StoreResult();
    DBManager::Instance()->unlock();

	uint8_t NumberOfServers = (uint8_t)DBManager::Instance()->NumRows(result);
	char* buffer;
	buffer = (char*)malloc(sizeof(Packet::AuthServerListEx) + (sizeof(Packet::SLServer) * NumberOfServers));
	Packet::AuthServerListEx* packet = (Packet::AuthServerListEx*)buffer;
	packet->PacketLength				= 0xFF;
	packet->OPCode						= OPCode::AuthSLE;
	packet->ServerCount					= NumberOfServers;
	packet->LastServerID				= 0x35;
	
	if (NumberOfServers > 0)
	{
		for (int i=0; i<packet->ServerCount; i++)
		{
			Packet::SLServer* server    = (Packet::SLServer*)(buffer + sizeof(Packet::AuthServerListEx) + (sizeof(Packet::SLServer) * i));
			row = DBManager::Instance()->FetchRow(result);
			server->ServerID			= (uint8_t) atoi(row[0]);
			server->Host				= Net::IPtoHex(row[1]);
			server->Port				= atoi(row[2]);
			server->AgeLimit			= (uint8_t) atoi(row[3]);
			server->PKFlag				= (uint8_t) atoi(row[4]);
			server->CurrentUserCount	= (uint16_t)atoi(row[5]);
			server->MaxUserCount		= (uint16_t)atoi(row[6]);
			server->ServerStatus		= (uint8_t) atoi(row[7]);
		}
	}
	Net::SendEncrypted(player->socket, buffer, sizeof(Packet::AuthServerListEx) + (sizeof(Packet::SLServer) * NumberOfServers));
	free(buffer);
    // shouldn't we free the MySQL result too?
	return;
}

uint8_t HandleMessage::AuthSelectServer(Player* player)
{
	Packet::AuthSelectServer packet;
	memcpy(&packet, player->RecvBuffer, sizeof(Packet::AuthSelectServer));
	return (uint8_t)packet.ServerID;
}

void HandleMessage::LastPacket(Player* player, uint8_t ServerID)
{
	uint64_t sessionID = SessionManager::Instance()->GenerateSession(player->Account, player->AccUID);
	Packet::LastPacket packet;
	packet.PacketLength = 0x0C;
	packet.OPCode = OPCode::AuthLP;
	packet.SessionID1 = (uint32_t)(sessionID & 0xFFFFFFFF);
	packet.SessionID2 = (uint32_t)(sessionID >> 32);
	packet.ServerID = ServerID;	
	Net::SendEncrypted(player->socket, (char*)&packet, 0xC);
}
