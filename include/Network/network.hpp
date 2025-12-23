/*
 Copyright (C) 2024 BeamMP Ltd., BeamMP team and contributors.
 Licensed under AGPL-3.0 (or later), see <https://www.gnu.org/licenses/>.
 SPDX-License-Identifier: AGPL-3.0-or-later
*/


#pragma once
#include <filesystem>
#include <string>
#include <thread>

#ifdef __linux__
#include "linuxfixes.h"
#include <bits/types/siginfo_t.h>
#include <cstdint>
#include <sys/ucontext.h>
#include <arpa/inet.h>
#endif

class TCPGameClient;
class UDPGameClient;

class TCPGameClient {
private:
	SOCKET TCPSock = -1;
	const std::string IP;
	int Port;
	std::thread Thread;

	bool CheckBytes(int32_t Bytes);
	void start();
public:
	TCPGameClient(const std::string IP, int Port);
	~TCPGameClient();
	void Run();
	void Stop();
	void TCPSend(const std::string& Data);
	std::string TCPRcv();
	SOCKET Sock();
};

class GameServer {
private:
	const std::string IP;
	int Port;
	std::thread Thread;
	TCPGameClient tgc;

	void start();
public:
	GameServer(const std::string IP, int Port);
	~GameServer();
	void Run();
	void Stop();
	void NetReset();
};

//void NetReset();
extern bool Dev;
extern int ping;

[[noreturn]] void CoreNetwork();
extern int ProxyPort;
extern int ClientID;
extern int LastPort;
extern bool ModLoaded;
extern volatile bool Terminate;
extern uint64_t UDPSock;
//extern uint64_t TCPSock;
extern std::string Branch;
extern std::filesystem::path CachingDirectory;
extern bool deleteDuplicateMods;
extern volatile bool TCPTerminate;
extern std::string LastIP;
extern std::string MStatus;
extern std::string UlStatus;
extern std::string PublicKey;
extern std::string PrivateKey;
extern std::string magic;
int KillSocket(uint64_t Dead);
void UUl(const std::string& R);
void UDPSend(std::string Data);
//bool CheckBytes(int32_t Bytes);
void GameSend(std::string_view Data);
void SendLarge(TCPGameClient &tgc, std::string Data);
//std::string TCPRcv(uint64_t Sock);
//void SyncResources(uint64_t TCPSock);
void SyncResources(TCPGameClient &tgc);
std::string GetAddr(const std::string& IP);
void ServerParser(std::string_view Data);
std::string Login(const std::string& fields);
//void TCPSend(const std::string& Data, uint64_t Sock);
//void TCPClientMain(const std::string& IP, int Port);
void UDPClientMain(TCPGameClient *tgc, const std::string& IP, int Port);
//void TCPGameServer(const std::string& IP, int Port);
bool SecurityWarning();
void CoreSend(std::string data);
