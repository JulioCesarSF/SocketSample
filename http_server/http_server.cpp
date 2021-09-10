#include "http_server.h"
#include <assert.h>
using namespace http_server;

SOCKET server_t::setup_socket()
{
	//prepare to bind server socket (address struct)
	sockaddr_in s_addr_in;
	ZeroMemory(&s_addr_in, sizeof(sockaddr_in));

	//https://docs.microsoft.com/pt-br/windows/win32/api/winsock2/nf-winsock2-socket
	SOCKET server_socket = ::socket(
		AF_INET,		//IPV4
		SOCK_STREAM,	//TCP
		0);				// no protocol (we create our own protocol in this case)

	if (server_socket == INVALID_SOCKET)
	{
		//TODO: log and exit
		auto wsa_error = ::WSAGetLastError();
		return server_socket;
	}


	//https://docs.microsoft.com/pt-br/windows/win32/api/winsock2/nf-winsock2-htons
	s_addr_in.sin_port = ::htons(_server_port); //resolve port
	s_addr_in.sin_family = AF_INET; //IPV4

	//https://docs.microsoft.com/pt-br/windows/win32/api/ws2tcpip/nf-ws2tcpip-inet_pton
	auto r_val = ::inet_pton(AF_INET, _server_ip, &s_addr_in.sin_addr); //resolve address
	if (r_val != 1)
	{
		//TODO: log and exit
		auto wsa_error = ::WSAGetLastError();

		::closesocket(server_socket); //cleanup socket
		server_socket = INVALID_SOCKET;
		return server_socket;
	}

	u_long mode = 1;  //non-blocking socket
	::ioctlsocket(server_socket, FIONBIO, &mode);

	//https://docs.microsoft.com/pt-br/windows/win32/api/winsock2/nf-winsock2-bind
	r_val = ::bind(server_socket, (sockaddr*)&s_addr_in, sizeof(s_addr_in)); //associate address and socket

	if (r_val == SOCKET_ERROR)
	{
		//TODO: log and exit
		auto wsa_error = ::WSAGetLastError();

		::closesocket(server_socket); //cleanup socket
		server_socket = INVALID_SOCKET;
		return server_socket;
	}

	r_val = ::listen(server_socket, SOMAXCONN);

	if (r_val == SOCKET_ERROR)
	{
		//TODO: log and exit
		auto wsa_error = ::WSAGetLastError();

		::closesocket(server_socket); //cleanup socket
		server_socket = INVALID_SOCKET;
		return server_socket;
	}

	return server_socket;
}

SOCKET server_t::get_socket_client(SOCKET server_socket)
{
	assert(_server_socket != INVALID_SOCKET);

	SOCKET newClient = ::accept(_server_socket, nullptr, nullptr);

	return newClient;
}

server_t::recv_struct_t server_t::recv_all(SOCKET client)
{
	assert(client != INVALID_SOCKET);
	recv_struct_t recvStruct;
	char cBuffer[MESSAGE_BUFFER_SIZE] = {};
	int bRcv = 0;
	int totalSize = 0;

	while (true)
	{
		bRcv = ::recv(client, cBuffer, MESSAGE_BUFFER_SIZE, 0);
		if (bRcv <= 0) break;
		recvStruct.request.append(cBuffer);
		totalSize += bRcv;
		ZeroMemory(&cBuffer, MESSAGE_BUFFER_SIZE);
	}
	recvStruct.process_response = totalSize > 0;
	return recvStruct;
}

void server_t::clear()
{
	shutdown_server = true;
	if (_server_socket != INVALID_SOCKET)
		::closesocket(_server_socket);
	//https://docs.microsoft.com/pt-br/windows/win32/api/winsock/nf-winsock-wsacleanup
	::WSACleanup();
	initialized = false;
}

server_t::server_t(const char* ip, u_short port) : _server_ip(ip), _server_port(port)
{
	assert(ip != nullptr);
	assert(port > 0 && port <= 65535);	

	WORD socketVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	ZeroMemory(&wsaData, sizeof(WSADATA));

	//https://docs.microsoft.com/pt-br/windows/win32/api/winsock/nf-winsock-wsastartup
	auto wsa_startup = ::WSAStartup(socketVersion, &wsaData);
	if (wsa_startup != 0)
	{
		//log and exit
		auto wsa_error = ::WSAGetLastError();
		return;
	}

	_server_socket = setup_socket();
	initialized = _server_socket != INVALID_SOCKET;
	assert(initialized == true);
}

server_t::~server_t()
{
	clear();
}

void server_t::run(request_handler_i* controller)
{
	assert(initialized == true);
	assert(_server_socket != INVALID_SOCKET);

	// set of clients (this is an array)
	fd_set master;
	FD_ZERO(&master); //zero it
	FD_SET(_server_socket, &master); //add server to array

	while (!shutdown_server)
	{
		fd_set masterCopy = master; //copy server set

		const timeval tv = { 1, 0 };
		//retrieve the socket array connections
		int socket_count = select
		(
			0, //ignored by MS docs
			&masterCopy,
			nullptr,
			nullptr,
			&tv
		);

		//loop sockets
		for (int i = 0; i < socket_count; i++)
		{
			SOCKET currentSocket = masterCopy.fd_array[i];
			//new client at listener socket
			if (currentSocket == _server_socket)
			{
				SOCKET newClient = get_socket_client(_server_socket); //accepts the new client
				if (newClient != INVALID_SOCKET)
				{
					FD_SET(newClient, &master); //add the accepted client to server array
				}
			}
			//client already in server but it's sending a new message
			else
			{
				u_long mode = 1;  //non-blocking socket
				::ioctlsocket(currentSocket, FIONBIO, &mode);

				auto sRequest = recv_all(currentSocket);

				if (sRequest.process_response && controller != nullptr)
				{
					auto response = controller->handle_request(sRequest.request);
					send_to_client(int(currentSocket), response.c_str()); //send response
				}

				::closesocket(currentSocket); //shutdown client socket
				FD_CLR(currentSocket, &master); //removes this client from server array
			}
		}
	}
}

void server_t::send_to_client(int socket_client, const char* payload)
{	
	assert(socket_client > 0);
	assert(payload != nullptr);

	int total_to_send = ::strlen(payload);
	int sent = 0;
	int offset = 0;
	do
	{
		int bytes_sent = ::send(socket_client, payload + offset, total_to_send, 0);
		if (bytes_sent <= 0) break;
		sent += bytes_sent;
		total_to_send -= bytes_sent;
		offset += bytes_sent;
	} while (sent < total_to_send);
}