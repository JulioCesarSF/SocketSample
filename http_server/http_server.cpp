#include "http_server.h"
#include <assert.h>

#include <thread>
#include <mutex>

using namespace http_server;

std::mutex server_mutex;

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
		log_text("Server socket not created: " + std::to_string(wsa_error));
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
		log_text("Could not resolve server address: " + std::to_string(wsa_error));
		::closesocket(server_socket); //cleanup socket
		server_socket = INVALID_SOCKET;
		return server_socket;
	}

	//https://docs.microsoft.com/pt-br/windows/win32/api/winsock2/nf-winsock2-bind
	r_val = ::bind(server_socket, (sockaddr*)&s_addr_in, sizeof(s_addr_in)); //associate address and socket

	if (r_val == SOCKET_ERROR)
	{
		//TODO: log and exit
		auto wsa_error = ::WSAGetLastError();
		log_text("Could associate address and socket: " + std::to_string(wsa_error));
		::closesocket(server_socket); //cleanup socket
		server_socket = INVALID_SOCKET;
		return server_socket;
	}

	r_val = ::listen(server_socket, SOMAXCONN);

	if (r_val == SOCKET_ERROR)
	{
		//TODO: log and exit
		auto wsa_error = ::WSAGetLastError();
		log_text("Could star to listen on server socket: " + std::to_string(wsa_error));
		::closesocket(server_socket); //cleanup socket
		server_socket = INVALID_SOCKET;
		return server_socket;
	}

	u_long mode = 1;  //non-blocking socket
	::ioctlsocket(server_socket, FIONBIO, &mode);
	log_text("Socket setup OK");
	return server_socket;
}

SOCKET server_t::accept_new_client(SOCKET server_socket)
{
	assert(_server_socket != INVALID_SOCKET);
	return ::accept(_server_socket, nullptr, nullptr);
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

server_t::server_t(const char* ip, u_short port, std::function<void(std::string)> log_callback) :
	_server_ip(ip),
	_server_port(port),
	_log_callback(log_callback)
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
		log_text("Server not started: " + std::to_string(wsa_error));
		return;
	}

	_server_socket = setup_socket();
	initialized = _server_socket != INVALID_SOCKET;
	assert(initialized == true);

	log_text("Socket initialized");
}

server_t::~server_t()
{
	log_text("Shutting down server");
	clear();
}

void server_t::run(request_handler_i* controller)
{
	assert(initialized == true);
	assert(_server_socket != INVALID_SOCKET);

	// set of clients (this is an array)
	fd_set temp_set;
	FD_ZERO(&temp_set); //zero it
	FD_SET(_server_socket, &temp_set); //add server to array

	while (!shutdown_server)
	{
		fd_set fd_set_read = temp_set; //copy server set

		const timeval tv = { 1, 0 };
		//retrieve the socket array connections
		int socket_count = select
		(
			0, //ignored by MS docs
			&fd_set_read,
			nullptr,
			nullptr,
			&tv
		);

		//loop sockets
		for (int i = 0; i < socket_count; i++)
		{
			SOCKET currentSocket = fd_set_read.fd_array[i];
			//new client at listener socket
			if (currentSocket == _server_socket)
			{
				SOCKET newClient = accept_new_client(_server_socket); //accepts the new client
				if (newClient != INVALID_SOCKET)
				{
					FD_SET(newClient, &temp_set); //add the accepted client to server array
				}
			}
			//client already in server but it's sending a new message
			else
			{
				auto sRequest = recv_all(currentSocket);

				if (sRequest.process_response && controller != nullptr)
				{
					auto response = controller->handle_request(sRequest.request);
					send_all(int(currentSocket), response.c_str()); //send response
				}

				::closesocket(currentSocket); //shutdown client socket
				FD_CLR(currentSocket, &temp_set); //removes this client from server array
			}
		}
	}
}

void server_t::start_sets()
{
	FD_ZERO(&_sets.fd_read);
	FD_ZERO(&_sets.fd_write);
	FD_ZERO(&_sets.fd_error);

	FD_SET(_server_socket, &_sets.fd_read);
	FD_SET(_server_socket, &_sets.fd_error);

	for (size_t index = 0; index < _server_clients.size(); index++)
	{
		socket_client_t client = _server_clients.at(index);
		if (client.can_read)
		{
			FD_SET(client.socket, &_sets.fd_read);
			client.can_read = false;
		}
		else
		{
			FD_SET(client.socket, &_sets.fd_write);
		}

		FD_SET(client.socket, &_sets.fd_error);
	}

}

void server_t::process_sets(request_handler_i* controller)
{
	for (size_t index = 0; index < _server_clients.size(); index++)
	{
		socket_client_t& client = _server_clients.at(index);

		if (FD_ISSET(client.socket, &_sets.fd_error))
		{
			if (index == 0)
				_server_clients.pop_back();
			else
				_server_clients.erase(_server_clients.begin() + index);
			::closesocket(client.socket);
			FD_CLR(client.socket, &_sets.fd_error);
			continue;
		}

		if (FD_ISSET(client.socket, &_sets.fd_read))
		{
			client.recv = recv_all(client.socket);
			client.can_read = false;
			FD_CLR(client.socket, &_sets.fd_read);
		}

		if (FD_ISSET(client.socket, &_sets.fd_write))
		{
			if (client.recv.process_response)
			{
				log_text("Calling handle_request");
				Timer t;
				const auto resp = controller->handle_request(client.recv.request);
				auto elps = t.elapsed();
				log_text("Returned handle_request [" + std::to_string(elps) + "]");
				send_all(client.socket, resp.c_str());
			}

			if (index == 0)
				_server_clients.pop_back();
			else
				_server_clients.erase(_server_clients.begin() + index);
			::closesocket(client.socket);
			FD_CLR(client.socket, &_sets.fd_write);
		}
	}
}

void server_t::acquire_new_client(request_handler_i* controller)
{
	// client not in server yet
	if (FD_ISSET(_server_socket, &_sets.fd_read))
	{
		SOCKET client_socket = accept_new_client(_server_socket);
		if (client_socket != INVALID_SOCKET)
		{
			u_long nNoBlock = 1;
			ioctlsocket(client_socket, FIONBIO, &nNoBlock); ;
			_server_clients.push_back(socket_client_t{ client_socket });
		}
	}
	else if (FD_ISSET(_server_socket, &_sets.fd_error))
	{
		//server error			
		shutdown_server = true;
		return;
	}
	else
	{
		process_sets(controller);
	}
}

void server_t::run_new(request_handler_i* controller)
{
	const timeval select_timout = { 1, 0 };
	FD_ZERO(&_sets.fd_read);
	FD_ZERO(&_sets.fd_write);
	FD_ZERO(&_sets.fd_error);

	FD_SET(_server_socket, &_sets.fd_read);
	FD_SET(_server_socket, &_sets.fd_error);

	log_text("Server is waiting connections...");
	do
	{
		start_sets();
		int connections = select(0, &_sets.fd_read, &_sets.fd_write, &_sets.fd_error, nullptr);
		switch (connections)
		{
		case 0: //time limit expired
			break;
		case SOCKET_ERROR:
			//int wsa_error = WSAGetLastError();
			shutdown_server = true;
			break;
		default:
			acquire_new_client(controller);
			break;
		}

	} while (!shutdown_server);
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

void server_t::send_all(int socket_client, const char* payload)
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

void server_t::log_text(std::string log_message)
{
	if (_log_callback)
		_log_callback(log_message);
}