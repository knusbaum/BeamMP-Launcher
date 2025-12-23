/*
 Copyright (C) 2024 BeamMP Ltd., BeamMP team and contributors.
 Licensed under AGPL-3.0 (or later), see <https://www.gnu.org/licenses/>.
 SPDX-License-Identifier: AGPL-3.0-or-later
*/


#include "Logger.h"
#include <Zlib/Compressor.h>
#include <chrono>
#include <iostream>
#include <vector>

#if defined(_WIN32)
#include <ws2tcpip.h>
#elif defined(__linux__)
#include <arpa/inet.h>
#include <cstring>
#include <errno.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#endif

#include "Network/network.hpp"

int LastPort;
std::string LastIP;
//SOCKET TCPSock = -1;

bool TCPGameClient::CheckBytes(int32_t Bytes) {
    if (Bytes == 0) {
        debug("(TCP) Connection closing... CheckBytes(16)");
        Terminate = true;
        return false;
    } else if (Bytes < 0) {
        debug("(TCP CB) recv failed with error: " + std::to_string(WSAGetLastError()));
        KillSocket(TCPSock);
        Terminate = true;
        return false;
    }
    return true;
}
void UUl(const std::string& R) {
    UlStatus = "UlDisconnected: " + R;
}

void TCPGameClient::TCPSend(const std::string& Data) {
    if (TCPSock == -1) {
        Terminate = true;
        UUl("Invalid Socket");
        return;
    }

    int32_t Size, Sent, Temp;
    std::string Send(4, 0);
    Size = int32_t(Data.size());
    memcpy(&Send[0], &Size, sizeof(Size));
    Send += Data;
    // Do not use Size before this point for anything but the header
    Sent = 0;
    Size += 4;
    do {
        if (size_t(Sent) >= Send.size()) {
            error("string OOB in " + std::string(__func__));
            UUl("TCP Send OOB");
            return;
        }
        Temp = send(TCPSock, &Send[Sent], Size - Sent, 0);
        if (!CheckBytes(Temp)) {
            UUl("Socket Closed Code 2");
            return;
        }
        Sent += Temp;
    } while (Sent < Size);
}

std::string TCPGameClient::TCPRcv() {
    if (TCPSock == -1) {
        Terminate = true;
        UUl("Invalid Socket");
        return "";
    }
    int32_t Header;
    int Temp;
    std::vector<char> Data(sizeof(Header));
    Temp = recv(TCPSock, Data.data(), sizeof(Header), MSG_WAITALL);
    if (!CheckBytes(Temp)) {
        UUl("Socket Closed Code 3");
        return "";
    }
    memcpy(&Header, Data.data(), sizeof(Header));

    if (!CheckBytes(Temp)) {
        UUl("Socket Closed Code 4");
        return "";
    }

    Data.resize(Header, 0);
    Temp = recv(TCPSock, Data.data(), Header, MSG_WAITALL);
    if (!CheckBytes(Temp)) {
        UUl("Socket Closed Code 5");
        return "";
    }

    std::string Ret(Data.data(), Header);

    if (Ret.substr(0, 4) == "ABG:") {
        auto substr = Ret.substr(4);
        try {
            auto res = DeComp(std::span<char>(substr.data(), substr.size()));
            Ret = std::string(res.data(), res.size());
        } catch (const std::runtime_error& err) {
            // this happens e.g. when we're out of memory, or when we get incomplete data
            error("Decompression failed");
            return "";
        }
    }

#ifdef DEBUG
    // debug("Parsing from server -> " + std::to_string(Ret.size()));
#endif
    if (Ret[0] == 'E' || Ret[0] == 'K')
        UUl(Ret.substr(1));
    return Ret;
}

TCPGameClient::TCPGameClient(const std::string IP, int Port) :IP(IP) {
	debug("My IP IS: " + this->IP);
    this->Port = Port;
}

TCPGameClient::~TCPGameClient() {
	debug("TCP GAME CLIENT DESTROYED");
	Stop();
}

SOCKET TCPGameClient::Sock() {
	return TCPSock;
}

void TCPGameClient::Stop() {
	if(TCPSock != -1) {
		KillSocket(TCPSock);
	}
	if (Thread.joinable()) {
		debug("TCPGameClient JOINING THREAD!");
		Thread.join();
		debug("TCPGameClient DONE JOINING THREAD!!!!!");                
	}
}

void TCPGameClient::Run() {
    debug("Running Game Client!");
	debug("My IP IS: " + IP);  
	Thread = std::thread(&TCPGameClient::start, this);	
}

void TCPGameClient::start() {
	debug("TCPGameClient Start! IP: (");
	debug(IP);
	debug(" ) IP DONE!");        
    LastIP = IP;
    LastPort = Port;
    SOCKADDR_IN ServerAddr;
    int RetCode;
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(514, &wsaData); // 2.2
#endif
    TCPSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (TCPSock == -1) {
        debug("Client: socket failed! Error code: %d\n" + std::to_string(WSAGetLastError()));
        WSACleanup();
		debug("TCPGameClient EXITING!");
        return;
    }

    ServerAddr.sin_family = AF_INET;
    ServerAddr.sin_port = htons(Port);
    inet_pton(AF_INET, IP.c_str(), &ServerAddr.sin_addr);
    RetCode = connect(TCPSock, (SOCKADDR*)&ServerAddr, sizeof(ServerAddr));
    if (RetCode != 0) {
        UlStatus = "UlConnection Failed!";
        error("Client: connect failed! Error code: " + std::to_string(WSAGetLastError()));
        KillSocket(TCPSock);
        WSACleanup();
        Terminate = true;
        CoreSend("L");
		debug("TCPGameClient EXITING!");        
        return;
    }
    info("Connected!");

    char Code = 'C';
    send(TCPSock, &Code, 1, 0);
    SyncResources(*this);
    while (!Terminate) {
        ServerParser(TCPRcv());
    }
    GameSend("T");
    ////Game Send Terminate
    if (KillSocket(TCPSock) != 0)
        debug("(TCP) Cannot close socket. Error code: " + std::to_string(WSAGetLastError()));

#ifdef _WIN32
    if (WSACleanup() != 0)
        debug("(TCP) Client: WSACleanup() failed!...");
#endif
	debug("TCPGameClient EXITING!");    
}
