#pragma once

#include <string>

namespace http_server
{
	/// <summary>
	/// Abstract class to create a controller
	/// </summary>
	class request_handler_i
	{
	public:

		/// <summary>
		/// Process a given payload
		/// </summary>
		/// <param name="payload">Incoming request</param>
		/// <returns>http response as string</returns>
		virtual std::string handle_request(const std::string& payload) = 0;
	};
};