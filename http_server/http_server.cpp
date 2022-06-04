#include "http_server.h"
#include <assert.h>

#include <thread>
#include <mutex>
#include "default_response.h"
#include "request.h"

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
		0);

	if (server_socket == INVALID_SOCKET)
	{
		auto wsa_error = ::WSAGetLastError();
		throw std::runtime_error("Server socket not created: " + std::to_string(wsa_error));
	}

	u_long mode = 1;  //non-blocking socket
	::ioctlsocket(server_socket, FIONBIO, &mode);

	//https://docs.microsoft.com/pt-br/windows/win32/api/winsock2/nf-winsock2-htons
	s_addr_in.sin_port = ::htons(_server_port); //resolve port
	s_addr_in.sin_family = AF_INET; //IPV4

	//https://docs.microsoft.com/pt-br/windows/win32/api/ws2tcpip/nf-ws2tcpip-inet_pton
	auto r_val = ::inet_pton(AF_INET, _server_ip, &s_addr_in.sin_addr); //resolve address
	if (r_val != 1)
	{
		auto wsa_error = ::WSAGetLastError();
		::closesocket(server_socket); //cleanup socket
		server_socket = INVALID_SOCKET;
		throw std::runtime_error("Could not resolve server address: " + std::to_string(wsa_error));
	}

	//https://docs.microsoft.com/pt-br/windows/win32/api/winsock2/nf-winsock2-bind
	r_val = ::bind(server_socket, (sockaddr*)&s_addr_in, sizeof(s_addr_in)); //associate address and socket

	if (r_val == SOCKET_ERROR)
	{
		auto wsa_error = ::WSAGetLastError();
		::closesocket(server_socket); //cleanup socket
		server_socket = INVALID_SOCKET;
		throw std::runtime_error("Could not to associate address and socket: " + std::to_string(wsa_error));
	}

	r_val = ::listen(server_socket, SOMAXCONN);

	if (r_val == SOCKET_ERROR)
	{
		auto wsa_error = ::WSAGetLastError();
		::closesocket(server_socket); //cleanup socket
		server_socket = INVALID_SOCKET;
		throw std::runtime_error("Could not to start to listen on server socket: " + std::to_string(wsa_error));
	}

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

	while (socket_queue.size() > 0)
	{
		auto client = socket_queue.pop();
		if (!client.has_value()) continue;
		closesocket(client.value().socket);
	}

	if (_server_socket != INVALID_SOCKET)
		::closesocket(_server_socket);
	//https://docs.microsoft.com/pt-br/windows/win32/api/winsock/nf-winsock-wsacleanup
	::WSACleanup();
	initialized = false;
}

server_t::server_t(const char* ip, u_short port, request_handler_i& controller) :
	_server_ip(ip),
	_server_port(port),
	_controller(controller)
{
	assert(ip != nullptr);
	assert(port > 0 && port <= 65535);

	server_endpoints = _controller.get_endpoints();

	WORD socketVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	ZeroMemory(&wsaData, sizeof(WSADATA));

	//https://docs.microsoft.com/pt-br/windows/win32/api/winsock/nf-winsock-wsastartup
	auto wsa_startup = ::WSAStartup(socketVersion, &wsaData);
	if (wsa_startup != 0)
	{
		auto wsa_error = ::WSAGetLastError();
		throw std::runtime_error("Could not initialize windows socket: " + std::to_string(wsa_error));
	}

	_server_socket = setup_socket();
	initialized = _server_socket != INVALID_SOCKET;
	assert(initialized == true);
}

server_t::~server_t()
{
	log_text("Shutting down server");
	clear();
}

void server_t::set_log_callback(std::function<void(std::string)> callback)
{
	_log_callback = callback;
}

void server_t::run()
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

				if (sRequest.process_response)
				{
					std::string s_response;
					if (sRequest.request.empty())
					{
						s_response = bad_request();
					}
					else
					{
						request_t request(sRequest.request);
						s_response = _controller.handle_request(request);
					}
					send_all(int(currentSocket), s_response.c_str()); //send response
				}

				::closesocket(currentSocket); //shutdown client socket
				FD_CLR(currentSocket, &temp_set); //removes this client from server array
			}
		}
	}
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

	size_t total_to_send = ::strlen(payload) + 1;
	int sent = 0;
	int offset = 0;
	do
	{
		int bytes_sent = ::send(socket_client, payload + offset, static_cast<int>(total_to_send), 0);
		if (bytes_sent <= 0) break;
		sent += bytes_sent;
		total_to_send -= bytes_sent;
		offset += bytes_sent;
	} while (sent < total_to_send);
}

void server_t::log_text(std::string log_message)
{
	std::lock_guard<std::mutex> lck(server_mutex);
	if (_log_callback)
		_log_callback(log_message);
}

void server_t::consume_queue()
{
	auto socket = socket_queue.pop();
	if (!socket.has_value()) return;
	auto client = socket.value();

	benchmark_t benchmark;

	recv_struct_t rcv_struct = recv_all(client.socket);
	if (rcv_struct.process_response)
	{
		std::string s_response;
		if (rcv_struct.request.empty())
		{
			s_response = bad_request();
		}
		else
		{
			request_t request(rcv_struct.request);
			s_response = is_endpoint_available(request) ?
				_controller.handle_request(request) :
				bad_request();
		}
		send_all(static_cast<int>(client.socket), s_response.c_str()); //send response					
	}
	::closesocket(client.socket); //shutdown client socket

	clients_consumed++;
	time_per_client += benchmark.elapsed() + client.time_to_accept;
	auto avrg = time_per_client / clients_consumed;
	log_text("Clients consumed: " + std::to_string(clients_consumed) + ", avg per request (accept/recv/send/closesocket): " + std::to_string(avrg));
	return;
}

bool http_server::server_t::is_endpoint_available(request_t& request)
{
	auto endpoint_exists = std::find_if(server_endpoints.begin(), server_endpoints.end(),
		[&](const request_item_t& item)
		{
			if (request._endpoint.size() == item._endpoint.size())
				return request._http_method == item._http_method && request._endpoint == item._endpoint;

			const auto contains_param = item._endpoint.find("{");
			if (contains_param == std::string::npos)
				return request._http_method == item._http_method && request._endpoint == item._endpoint; // :<

			std::string copy_end_point = item._endpoint.substr(0, item._endpoint.find_last_of("/"));
			return request._http_method == item._http_method && contains_param && (request._endpoint.find(copy_end_point) != std::string::npos);
		});

	auto found = endpoint_exists != server_endpoints.end();

	if (found)
	{
		// check for url params
		auto param_pos_start = endpoint_exists->_endpoint.find("{");
		auto param_pos_end = endpoint_exists->_endpoint.find("}");
		if ((param_pos_start != std::string::npos) && (param_pos_end != std::string::npos))
		{
			auto param_start_request = request._endpoint.find_last_of('/');
			if (request._endpoint.size() > request._endpoint.size() - param_pos_start)
			{
				request._param_value = request._endpoint.substr(param_start_request + 1);
				request._endpoint = endpoint_exists->_endpoint;
			}
			else
			{
				request._param_value = "";
			}
		}
	}

	return found;
}

void server_t::run_queue(bool run_on_thread)
{
	assert(initialized == true);
	assert(_server_socket != INVALID_SOCKET);

	if (!run_on_thread)
	{
		while (!shutdown_server)
		{
			std::thread(
				[&] {
					benchmark_t benchmark;
					SOCKET new_client = accept_new_client(_server_socket);
					if (new_client != INVALID_SOCKET)
					{
						u_long mode = 1;  //non-blocking socket
						::ioctlsocket(new_client, FIONBIO, &mode);
						socket_queue.push(n_socket_client_t{ new_client, "", "", benchmark.elapsed() });
					}
				}
			).detach();

			consume_queue();
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}
	else
	{
		std::thread(
			[&]() {
				while (!shutdown_server)
				{
					std::thread(
						[&] {
							benchmark_t benchmark;
							SOCKET new_client = accept_new_client(_server_socket);
							if (new_client != INVALID_SOCKET)
							{
								u_long mode = 1;  //non-blocking socket
								::ioctlsocket(new_client, FIONBIO, &mode);
								socket_queue.push(n_socket_client_t{ new_client, "", "", benchmark.elapsed() });
							}
						}
					).detach();

					consume_queue();
					std::this_thread::sleep_for(std::chrono::microseconds(500));
				}
			}).detach();
	}

}
