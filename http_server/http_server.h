#pragma once

#include "request_handler.h"

#define MESSAGE_BUFFER_SIZE 1024 * 5
#define NOMINMAX
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

namespace http_server
{
	/// <summary>
	/// Socket server (TCP) listener
	/// </summary>
	class server_t
	{
	private:

		/// <summary>
		/// Server ip
		/// </summary>
		const char* _server_ip = nullptr;

		/// <summary>
		/// Server port
		/// </summary>
		u_short _server_port = 0;

		/// <summary>
		/// Server socket
		/// </summary>
		SOCKET _server_socket = INVALID_SOCKET;

		/// <summary>
		/// Creates a socket TPC
		/// </summary>
		/// <returns>Not INVALID_SOCKET if created</returns>
		SOCKET setup_socket();

		/// <summary>
		/// Accepts a new client connection
		/// </summary>
		/// <param name="serverSocket">Server socket</param>
		/// <returns>Client socket connected to this server</returns>
		SOCKET get_socket_client(SOCKET server_socket);
		struct recv_struct_t
		{
			bool process_response = false;
			std::string request = "";
		};

		recv_struct_t recv_all(SOCKET client);

		/// <summary>
		/// Clear
		/// </summary>
		void clear();

	public:

		/// <summary>
		/// Is this listener initialized?
		/// </summary>
		bool initialized = false;

		/// <summary>
		/// Signal to shutdown server
		/// Stop to listen for new clients
		/// </summary>
		bool shutdown_server = false;

		server_t(const char* ip, u_short port);

		virtual ~server_t();

		/// <summary>
		/// Multiple client server
		/// </summary>
		void run(request_handler_i* controller);

		/// <summary>
		/// Sends a message back to a given socket client
		/// </summary>
		/// <param name="socketClient">Socket client</param>
		/// <param name="payload">Payload to send back to the client socket</param>
		void send_to_client(int socket_client, const char* payload);
	};
}
