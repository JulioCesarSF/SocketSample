#pragma once

//https://gist.github.com/JulioCesarSF/e3d280413348736284df255b7035d48a

#include "Logger.h"

#define MESSAGE_BUFFER_SIZE 1024 * 5

#include <assert.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <string>
#include <sstream>
#include <thread>
#include <mutex>

std::mutex socketMutex;

/// <summary>
/// TCP client
/// Usage:
/*
	Client client(SERVER_IP, SERVER_PORT);
	if (!client.initialized) return -1;

	std::string message;
	std::atomic_bool stop = false;

	//get all messages from server
	std::thread getResponses([&]
		{
			while (!stop)
			{
				auto response = client.ReadServerResponse();
				if (!response.empty())
					Log(response);

				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		});

	getResponses.detach();

	while (!stop)
	{
		Log("Type a message:");
		std::getline(std::cin, message);
		if (message.empty()) continue;
		client.SendMessageToServer(message.c_str());

		message.clear();

		if (GetAsyncKeyState(VK_END))
			stop = true;
	}
*/
/// </summary>
class Client
{
private:
	/// <summary>
	/// Server ip
	/// </summary>
	const char* serverIp = nullptr;

	/// <summary>
	/// Server port
	/// </summary>
	u_short serverPort = 0;

	/// <summary>
	/// Client socket
	/// </summary>
	SOCKET clientSocket = INVALID_SOCKET;

	/// <summary>
	/// Creates a socket TPC
	/// </summary>
	/// <returns>Not INVALID_SOCKET if created</returns>
	SOCKET SetupSocket()
	{
		//prepare to bind server socket (address struct)
		sockaddr_in sAddrIn;
		ZeroMemory(&sAddrIn, sizeof(sockaddr_in));

		//https://docs.microsoft.com/pt-br/windows/win32/api/winsock2/nf-winsock2-socket
		SOCKET bSocket = socket(
			AF_INET,		//IPV4
			SOCK_STREAM,	//TCP
			0);				// no protocol (we create our own protocol in this case)

		if (bSocket == INVALID_SOCKET)
		{
			//TODO: log and exit
			auto wsaError = WSAGetLastError();
			return bSocket;
		}


		//https://docs.microsoft.com/pt-br/windows/win32/api/winsock2/nf-winsock2-htons
		sAddrIn.sin_port = htons(serverPort); //resolve port
		sAddrIn.sin_family = AF_INET; //IPV4

		//https://docs.microsoft.com/pt-br/windows/win32/api/ws2tcpip/nf-ws2tcpip-inet_pton
		auto rVal = inet_pton(AF_INET, serverIp, &sAddrIn.sin_addr); //resolve address
		if (rVal != 1)
		{
			//TODO: log and exit
			auto wsaError = WSAGetLastError();

			closesocket(bSocket); //cleanup socket
			bSocket = INVALID_SOCKET;
			return bSocket;
		}

		if (connect(bSocket, (sockaddr*)&sAddrIn, sizeof(sAddrIn)) != 0)
		{
			//TODO: log and exit
			auto wsaError = WSAGetLastError();

			closesocket(bSocket); //cleanup socket
			bSocket = INVALID_SOCKET;
			return bSocket;
		}

		u_long mode = 1;  //non-blocking socket
		ioctlsocket(bSocket, FIONBIO, &mode);

		return bSocket;
	}

	void Clear()
	{
		if (clientSocket != INVALID_SOCKET)
			closesocket(clientSocket);
		WSACleanup();
	}
public:

	/// <summary>
	/// Is this client initialized?
	/// </summary>
	bool initialized = false;

	Client(const char* ip, u_short port) : serverIp(ip), serverPort(port)
	{
		assert(ip != nullptr);
		assert(port > 0 && port <= 65535);

		WORD socketVersion = MAKEWORD(2, 2);
		WSADATA wsaData;
		ZeroMemory(&wsaData, sizeof(WSADATA));

		//https://docs.microsoft.com/pt-br/windows/win32/api/winsock/nf-winsock-wsastartup
		auto wsaStartup = WSAStartup(socketVersion, &wsaData);
		if (wsaStartup != 0)
		{
			//log and exit
			auto wsaError = WSAGetLastError();
			return;
		}

		clientSocket = SetupSocket();

		initialized = clientSocket != INVALID_SOCKET;

		Log("Client initialized: " + std::to_string(initialized));
		Log("> Ip:\t" + std::string(ip));
		Log("> Port:\t" + std::to_string(port));

		assert(initialized == true);
	}

	~Client()
	{
		Clear();
	}

	bool SendMessageToServer(const char* payload)
	{
		assert(initialized == true);
		assert(clientSocket != INVALID_SOCKET);

		socketMutex.lock();
		auto sizeToSend = strlen(payload);
		auto totalSent = 0;
		char* bufferToSend = const_cast<char*>(payload);

		auto sent = send(clientSocket, bufferToSend, sizeToSend, 0);
		while (sent != sizeToSend)
		{
			if (sent == -1)
				break;
			sizeToSend -= sent;
			bufferToSend += sent;
			totalSent += sent;
			sent = send(clientSocket, bufferToSend, sizeToSend, 0);
		}
		socketMutex.unlock();

		return sent > 0;
	}

	std::string ReadServerResponse()
	{
		assert(initialized == true);
		assert(clientSocket != INVALID_SOCKET);

		char cBuffer[MESSAGE_BUFFER_SIZE] = {};
		int bRcv = 0;
		socketMutex.lock();
		//https://docs.microsoft.com/pt-br/windows/win32/api/winsock2/nf-winsock2-recv
		bRcv = recv(clientSocket, cBuffer, MESSAGE_BUFFER_SIZE, 0);
		socketMutex.unlock();
		if (bRcv > 0)
		{
			return std::string(cBuffer);
		}

		return "";
	}
};