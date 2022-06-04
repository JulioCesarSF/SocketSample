#pragma once

#include "request_handler.h"
#include "request.h"

#include <unordered_map>
#include <map>
#include <vector>
#include <functional>
#include <thread>
#include <mutex>

namespace http_server
{
	/// <summary>
	/// Default implementation for request_handler_i
	/// </summary>
	class default_controller_t : public request_handler_i
	{
	private:

		/// <summary>
		/// Endpoints for GET verb
		/// </summary>
		std::unordered_map<std::string, std::function<std::string(request_t)>> end_points_get;

		/// <summary>
		/// Endpoints for POST verb
		/// </summary>
		std::unordered_map<std::string, std::function<std::string(request_t)>> end_points_post;

		std::mutex controller_mutex;

	public:

		std::string handle_request(const std::string& payload);

		std::vector<request_item_t> get_endpoints();

		std::string handle_request(const request_t& request);

		const char* controller_name();

		/// <summary>
		/// Add a new GET endpoint
		/// </summary>
		/// <param name="endPoint">Endpoint eg. /person</param>
		/// <param name="handler">function to handle the request and response</param>
		void add_get(const std::string& endpoint, std::function<std::string(request_t)> handler);

		/// <summary>
		/// Add a new POST endpoint
		/// </summary>
		/// <param name="endPoint">Endpoint eg. /person</param>
		/// <param name="handler">function to handle the request and response</param>
		void add_post(const std::string& endpoint, std::function<std::string(request_t)> handler);

		/// <summary>
		/// Add a given endpoint to this controller
		/// </summary>
		/// <param name="endPoint">EndPoint eg. /person</param>
		/// <param name="process">function to handle the request and response</param>
		template<http_method_e method>
		void add_endpoint(const std::string& endpoint, std::function<std::string(request_t)> handler);

		virtual ~default_controller_t();
	};
}