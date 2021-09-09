#pragma once

#include "request.h"
#include "response.h"

#define MESSAGE_BUFFER_SIZE 1024 * 5
#define NOMINMAX
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

namespace http_client
{
	/// <summary>
	/// Creates a connection
	/// </summary>
	class request_sender_t
	{
	private:

		/// <summary>
		/// Request to send
		/// </summary>
		request_t _request;

		/// <summary>
		/// Socket
		/// </summary>
		SOCKET _socket;

		/// <summary>
		/// Creates a connection to the server
		/// </summary>
		/// <param name="host">Server address</param>
		/// <param name="port">Server port</param>
		/// <returns>True if connected</returns>
		bool create_connection(const std::string& host, int port);

		/// <summary>
		/// Reads socket to the end
		/// </summary>
		/// <returns>Server response as string</returns>
		std::string receive_response();

	public:

		bool initialized = false;

		request_sender_t();

		~request_sender_t();

		/// <summary>
		/// Sends a http request
		/// </summary>
		/// <param name="request">Reques to send</param>
		/// <returns>Server response as string</returns>
		response_t send_request(request_t& request);
	};
};