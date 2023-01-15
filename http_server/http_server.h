#pragma once

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
#include <queue>
#include <optional>
#include <bitset>

#include "request_handler.h"
#include "benchmark.h"

#ifdef _WEBSOCKET
#include "openssl/sha.h"
#endif

namespace http_server
{
	struct n_socket_client_t
	{
		SOCKET socket;
		std::string request;
		std::string response;
		double time_to_accept;
	};

	class socket_queue_t
	{
		std::vector<n_socket_client_t> _socket_queue;
		mutable std::mutex mutex_;

		bool empty() const
		{
			return _socket_queue.empty();
		}

	public:
		socket_queue_t() = default;
		socket_queue_t(const socket_queue_t&) = delete;
		socket_queue_t& operator=(const socket_queue_t&) = delete;

		socket_queue_t(socket_queue_t&& other) noexcept
		{
			std::lock_guard<std::mutex> lock(mutex_);
			_socket_queue = std::move(other._socket_queue);
		}

		virtual ~socket_queue_t() { }

		unsigned long size() const
		{
			std::lock_guard<std::mutex> lock(mutex_);
			return static_cast<unsigned long>(_socket_queue.size());
		}

		std::optional<n_socket_client_t> pop()
		{
			std::lock_guard<std::mutex> lock(mutex_);
			if (_socket_queue.empty())
			{
				return {};
			}
			n_socket_client_t tmp = _socket_queue.front();
			_socket_queue.pop_back();
			return tmp;
		}

		void push(const n_socket_client_t& item)
		{
			std::lock_guard<std::mutex> lock(mutex_);
			_socket_queue.push_back(item);
		}
	};

	/// <summary>
	/// This should output a python http client from mapped endpoints
	/// </summary>
	class python_client_creator
	{
	private:

	public:

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

		//aux log
		std::function<void(std::string)> _log_callback;
		void log_text(std::string log_message);

		//queue stuff
		std::atomic<int> clients_consumed;
		double time_per_client;
		void consume_queue();
		socket_queue_t socket_queue;
		std::thread socket_consumer;
		std::mutex server_mutex;
		std::mutex consumer_lock;
		request_handler_i& _controller;
		fd_set queue_set;

		//websocket clients
		std::vector<n_socket_client_t> websocket_clients;

		std::vector<request_item_t> server_endpoints;
		bool is_endpoint_available(request_t& request);

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

		server_t(const char* ip, u_short port, request_handler_i& controller);

		virtual ~server_t();

		void set_log_callback(std::function<void(std::string)> callback);

		/// <summary>
		/// Multiple client server
		/// </summary>		
		void run();

		/// <summary>
		/// Multiclient and multirequest server
		/// </summary>
		void run_queue(bool run_on_thread = false);

		/// <summary>
		/// Send a given text to all websocket clients (websocket)
		/// </summary>
		/// <param name="message">Message to send</param>
		void send_message_to_all_client(std::string message);

		/// <summary>
		/// Read all incomming values from connected sockets (websocket)
		/// </summary>
		void receive_messages_from_all_clients();
	};
}
