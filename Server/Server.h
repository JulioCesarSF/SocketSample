#pragma once

#include "Logger.h"
#include "FileHelper.h"
#include "RequestHandler.h"

#define MESSAGE_BUFFER_SIZE 1024 * 5

#include <assert.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <string>
#include <sstream>

/// <summary>
/// Socket server (TCP) listener
/// Usage:
/// 	Server server("127.0.0.1", 1248);
///     server.StartMultiClientListener();
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
			0);				// no protocol (we create our own protocol in this case)

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

		Log("Server initialized : " + std::to_string(initialized));
		Log("> Ip:\t" + std::string(ip));
		Log("> Port:\t" + std::to_string(port));

		assert(initialized == true);
	}

	~Server()
	{
		Clear();
	}

	/// <summary>
	/// Start listener only for one client
	/// </summary>
	void StartSingleClientListener()
	{
		assert(initialized == true);
		Log("Waiting for client...");
		while (!shutdownServer && serverSocket != INVALID_SOCKET)
		{
			SOCKET newClient = GetSocketClient(serverSocket);
			if (newClient == INVALID_SOCKET)
				continue;

			Log("!New client connected");
			char cBuffer[MESSAGE_BUFFER_SIZE] = {};
			int bRcv = 0;
			do
			{
				//cleanup buffer
				ZeroMemory(&cBuffer, MESSAGE_BUFFER_SIZE);

				//https://docs.microsoft.com/pt-br/windows/win32/api/winsock2/nf-winsock2-recv
				bRcv = recv(newClient, cBuffer, MESSAGE_BUFFER_SIZE, 0);

				//process message with a command
				if (bRcv > 0)
				{
					//reply received message to client as sample
					SendToClient(newClient, cBuffer);
				}

				//remote shutdown server
				if (_strcmpi(cBuffer, "/quit") == 0)
				{
					Log("Command /quit received from client.");
					shutdownServer = true;
					break;
				}
			} while (bRcv > 0);

			closesocket(newClient);//close connection from client
			closesocket(serverSocket); //close server listener
			serverSocket = SetupSocket(); //restart
		}
		Log("Server stopped.");
	}

	/// <summary>
	/// Multiple client server
	/// </summary>
	void StartMultiClientListener(IRequestHandler* requestHandler = nullptr)
	{
		assert(initialized == true);
		assert(serverSocket != INVALID_SOCKET);
		Log("Waiting for clients...");

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

		Log("Server stopped.");
	}

	/// <summary>
	/// Sends a message back to a given socket client
	/// </summary>
	/// <param name="socketClient">Socket client</param>
	/// <param name="packet">Payload to send back to the client socket</param>
	void SendToClient(int socketClient, const char* packet)
	{
		assert(socketClient > 0);
		assert(packet != nullptr);
		send(socketClient, packet, strlen(packet), 0);
	}
};