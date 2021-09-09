#pragma once

#include "RequestHandler.h"

#define MESSAGE_BUFFER_SIZE 1024 * 5

#include <assert.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <string>
#include <sstream>

/// <summary>
/// Socket server (TCP) listener
/// </summary>
class Server
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
	/// Server socket
	/// </summary>
	SOCKET serverSocket = INVALID_SOCKET;

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
		SOCKET serverSocket = socket(
			AF_INET,		//IPV4
			SOCK_STREAM,	//TCP
			0);

		if (serverSocket == INVALID_SOCKET)
		{
			//TODO: log and exit
			auto wsaError = WSAGetLastError();
			return serverSocket;
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

			closesocket(serverSocket); //cleanup socket
			serverSocket = INVALID_SOCKET;
			return serverSocket;
		}

		u_long mode = 1;  //non-blocking socket
		ioctlsocket(serverSocket, FIONBIO, &mode);

		//https://docs.microsoft.com/pt-br/windows/win32/api/winsock2/nf-winsock2-bind
		rVal = bind(serverSocket, (sockaddr*)&sAddrIn, sizeof(sAddrIn)); //associate address and socket

		if (rVal == SOCKET_ERROR)
		{
			//TODO: log and exit
			auto wsaError = WSAGetLastError();

			closesocket(serverSocket); //cleanup socket
			serverSocket = INVALID_SOCKET;
			return serverSocket;
		}

		rVal = listen(serverSocket, SOMAXCONN);

		if (rVal == SOCKET_ERROR)
		{
			//TODO: log and exit
			auto wsaError = WSAGetLastError();

			closesocket(serverSocket); //cleanup socket
			serverSocket = INVALID_SOCKET;
			return serverSocket;
		}

		return serverSocket;
	}

	/// <summary>
	/// Accepts a new client connection
	/// </summary>
	/// <param name="serverSocket">Server socket</param>
	/// <returns>Client socket connected to this server</returns>
	SOCKET GetSocketClient(SOCKET serverSocket)
	{
		assert(serverSocket != INVALID_SOCKET);

		SOCKET newClient = accept(serverSocket, nullptr, nullptr);

		return newClient;
	}

	struct RecvStruct
	{
		bool processResponse = false;
		std::string request = "";
	};

	RecvStruct RecvAll(SOCKET client)
	{
		assert(client != INVALID_SOCKET);
		RecvStruct recvStruct;
		char cBuffer[MESSAGE_BUFFER_SIZE] = {};
		int bRcv = 0;
		int totalSize = 0;

		while (true)
		{
			bRcv = recv(client, cBuffer, MESSAGE_BUFFER_SIZE, 0);
			if (bRcv <= 0) break;
			recvStruct.request.append(cBuffer);
			totalSize += bRcv;
			ZeroMemory(&cBuffer, MESSAGE_BUFFER_SIZE);
		}
		recvStruct.processResponse = totalSize > 0;
		return recvStruct;
	}

	/// <summary>
	/// Clear
	/// </summary>
	void Clear()
	{
		shutdownServer = true;
		if (serverSocket != INVALID_SOCKET)
			closesocket(serverSocket);
		//https://docs.microsoft.com/pt-br/windows/win32/api/winsock/nf-winsock-wsacleanup
		WSACleanup();
		initialized = false;
	}

public:

	/// <summary>
	/// Is this listener initialized?
	/// </summary>
	bool initialized = false;

	/// <summary>
	/// Signal to shutdown server
	/// Stop to listen for new clients
	/// </summary>
	bool shutdownServer = false;

	Server(const char* ip, u_short port) : serverIp(ip), serverPort(port)
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

		serverSocket = SetupSocket();
		initialized = serverSocket != INVALID_SOCKET;
		assert(initialized == true);
	}

	~Server()
	{
		Clear();
	}

	/// <summary>
	/// Run server
	/// </summary>
	void Run(IRequestHandler* requestHandler)
	{
		assert(initialized == true);
		assert(serverSocket != INVALID_SOCKET);
		assert(requestHandler != nullptr);

		// set of clients (this is an array)
		fd_set master;
		FD_ZERO(&master); //zero it
		FD_SET(serverSocket, &master); //add server to array

		while (!shutdownServer)
		{
			fd_set masterCopy = master; //copy server set

			//retrieve the socket array connections
			int socketCount = select
			(
				0, //ignored by MS docs
				&masterCopy,
				nullptr,
				nullptr,
				nullptr //no timeout, wait foreever
			);

			//loop sockets
			for (int i = 0; i < socketCount; i++)
			{
				SOCKET currentSocket = masterCopy.fd_array[i];
				//new client at listener socket
				if (currentSocket == serverSocket)
				{
					SOCKET newClient = GetSocketClient(serverSocket); //accepts the new client
					if (newClient != INVALID_SOCKET)
					{
						FD_SET(newClient, &master); //add the accepted client to server array						
					}
				}
				//client already in server but it's sending a new message
				else
				{
					u_long mode = 1;  //non-blocking socket
					ioctlsocket(currentSocket, FIONBIO, &mode);

					auto sRequest = RecvAll(currentSocket);

					if (sRequest.processResponse && requestHandler != nullptr)
					{
						auto response = requestHandler->HandleRequest(sRequest.request);
						SendToClient(currentSocket, response.c_str()); //send response												
					}

					closesocket(currentSocket); //shutdown client socket
					FD_CLR(currentSocket, &master); //removes this client from server array				
				}
			}
		}
	}

	/// <summary>
	/// Sends a message back to a given socket client
	/// </summary>
	/// <param name="socketClient">Socket client</param>
	/// <param name="payload">Payload to send back to the client socket</param>
	void SendToClient(int socketClient, const char* payload)
	{
		assert(socketClient != INVALID_SOCKET);
		assert(payload != nullptr);

		int totalToSend = strlen(payload);
		int sent = 0;
		int offset = 0;
		do
		{
			int bytesSent = send(socketClient, payload + offset, totalToSend, 0);
			if (bytesSent <= 0) break;
			sent += bytesSent;
			totalToSend -= bytesSent;
			offset += bytesSent;
		} while (sent < totalToSend);
	}
};