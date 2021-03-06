#pragma once

#include <string>
#include <vector>
#include "request.h"

namespace http_server
{
	/// <summary>
	/// Abstract class to create a controller
	/// </summary>
	class request_handler_i
	{
	public:

		/// <summary>
		/// Retrieve a list of all endpoint for a given controller
		/// </summary>
		/// <returns></returns>
		virtual std::vector<request_item_t> get_endpoints() = 0;

		/// <summary>
		/// Controller name
		/// </summary>
		/// <returns></returns>
		virtual const char* controller_name() = 0;

		/// <summary>
		/// Process a given payload
		/// </summary>
		/// <param name="payload">Incoming request</param>
		/// <returns>http response as string</returns>
		virtual std::string handle_request(const request_t& request) = 0;
	};
};