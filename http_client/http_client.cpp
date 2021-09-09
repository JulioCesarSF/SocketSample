#include "http_client.h"
using namespace http_client;

request_sender_t::request_sender_t() :
	_socket(INVALID_SOCKET)
{
	WORD socketVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	ZeroMemory(&wsaData, sizeof(WSADATA));

	auto wsaStartup = WSAStartup(socketVersion, &wsaData);
	if (wsaStartup != 0)
	{
		//log and exit
		auto wsaError = WSAGetLastError();
		return;
	}

	initialized = true;
}

request_sender_t::~request_sender_t()
{
	WSACleanup();
	initialized = false;
}

bool request_sender_t::create_connection(const std::string& host, int port)
{
	if (!initialized) return false;

	addrinfo addr_info_hints;
	addrinfo* addr_info_ress;
	ZeroMemory(&addr_info_hints, sizeof(addrinfo));

	_socket = socket(AF_INET, SOCK_STREAM, 0);

	if (_socket == INVALID_SOCKET)
	{
		const int error_code = WSAGetLastError();
		return false;
	}

	addr_info_hints.ai_family = AF_INET;
	addr_info_hints.ai_socktype = SOCK_STREAM;

	const auto resolved_host = getaddrinfo(host.c_str(), std::to_string(port).c_str(), &addr_info_hints, &addr_info_ress);
	if (resolved_host != 0)
	{
		const int error_code = WSAGetLastError();
		freeaddrinfo(addr_info_ress);
		closesocket(_socket);
		return false;
	}

	const auto connected = connect(_socket, addr_info_ress->ai_addr, addr_info_ress->ai_addrlen);
	if (connected != 0)
	{
		const int error_code = WSAGetLastError();
		freeaddrinfo(addr_info_ress);
		closesocket(_socket);
		return false;
	}

	freeaddrinfo(addr_info_ress);

	return true;
}

std::string http_client::request_sender_t::receive_response()
{
	std::string response;

	char cBuffer[MESSAGE_BUFFER_SIZE] = {};
	int bRcv = 0;

	while (true)
	{
		bRcv = recv(_socket, cBuffer, MESSAGE_BUFFER_SIZE, 0);
		if (bRcv <= 0) break;
		response.append(cBuffer);
		ZeroMemory(&cBuffer, MESSAGE_BUFFER_SIZE);
	}

	return response;
}

response_t request_sender_t::send_request(request_t& request)
{
	response_t response;
	if (!initialized) return response;
	if (!create_connection(request._host, request._port)) return response;

	request.build_request();
	const char* request_content = request._final_request.c_str();

	//send
	int totalToSend = strlen(request_content);
	int sent = 0;
	int offset = 0;
	do
	{
		int bytesSent = send(_socket, request_content, totalToSend, 0);
		if (bytesSent <= 0) break;
		sent += bytesSent;
		totalToSend -= bytesSent;
		offset += bytesSent;
	} while (sent < totalToSend);

	//receive and parse
	if (sent > 0)
	{
		response._payload = receive_response();
		response.build_response();
	}

	return response;
}