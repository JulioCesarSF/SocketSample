#pragma once

#include "request_handler.h"

#define MESSAGE_BUFFER_SIZE 1024 * 5
#define NOMINMAX
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#include <vector>
#include <functional>

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include <iostream>
#include <chrono>

namespace http_server
{
	class Timer
	{
	public:
		Timer() : beg_(clock_::now()) {}
		void reset() { beg_ = clock_::now(); }
		double elapsed() const {
			return std::chrono::duration_cast<second_>
				(clock_::now() - beg_).count();
		}

	private:
		typedef std::chrono::high_resolution_clock clock_;
		typedef std::chrono::duration<double, std::ratio<1> > second_;
		std::chrono::time_point<clock_> beg_;
	};

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
		SOCKET accept_new_client(SOCKET server_socket);

		struct recv_struct_t
		{
			bool process_response = false;
			std::string request = "";
		};

		struct socket_client_t
		{
			SOCKET socket;
			recv_struct_t recv;
			bool can_read = true;
			bool can_write = false;
		};

		/// <summary>
		/// Receives all data from client socket
		/// </summary>
		/// <param name="client"></param>
		/// <returns></returns>
		recv_struct_t recv_all(SOCKET client);

		/// <summary>
		/// Sends data back to a given socket client
		/// </summary>
		/// <param name="socketClient">Socket client</param>
		/// <param name="payload">Payload to send back to the client socket</param>
		void send_all(int socket_client, const char* payload);

		/// <summary>
		/// Clear
		/// </summary>
		void clear();

		/// <summary>
		/// run_new
		/// </summary>
		struct server_fd_sets_t
		{
			fd_set fd_read; //readable, can accept and recv
			fd_set fd_write; //writability, send
			fd_set fd_error; //writability, send
		};
		server_fd_sets_t _sets;
		std::vector<socket_client_t> _server_clients;

		void acquire_new_client(request_handler_i* controller);
		void start_sets();
		void process_sets(request_handler_i* controller);

		std::function<void(std::string)> _log_callback;
		void log_text(std::string log_message);
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

		server_t(const char* ip, u_short port, std::function<void(std::string)> log_callback);

		virtual ~server_t();

		/// <summary>
		/// Multiple client server
		/// </summary>
		void run(request_handler_i* controller);

		void run_new(request_handler_i* controller);
	};
}
