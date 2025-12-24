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
class GameServer;

class TCPGameClient {
private:
    SOCKET TCPSock = -1;
    const std::string IP;
    int Port;
    std::thread Thread;
    GameServer &gs;

    bool CheckBytes(int32_t Bytes);
    void start();
public:
    TCPGameClient(GameServer &gs, const std::string IP, int Port);
    ~TCPGameClient();
    void Run();
    void Stop();
    void TCPSend(const std::string& Data);
    std::string TCPRcv();
    SOCKET Sock();
    bool Running();
};

class GameServer {
private:
    const std::string IP;
    int Port;
    std::thread Thread;
    TCPGameClient tgc;
    bool CServer = true;

    void start();
public:
    GameServer(const std::string IP, int Port);
    ~GameServer();
    void Run();
    void Stop();
    void NetReset();
};

class SyncError{};

extern bool Dev;
extern int ping;

[[noreturn]] void CoreNetwork();
extern int ProxyPort;
extern int ClientID;
extern int LastPort;
extern bool ModLoaded;
extern volatile bool Terminate;
extern uint64_t UDPSock;
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
extern std::atomic<signed char> confirmed;
int KillSocket(uint64_t Dead);
void UUl(const std::string& R);
void UDPSend(std::string Data);
void GameSend(std::string_view Data);
void SendLarge(TCPGameClient &tgc, std::string Data);
bool SyncResources(TCPGameClient &tgc);
std::string GetAddr(const std::string& IP);
void ServerParser(std::string_view Data);
std::string Login(const std::string& fields);
void UDPClientMain(GameServer *gs, TCPGameClient *tgc, const std::string& IP, int Port);
void CoreSend(std::string data);
