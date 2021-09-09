#pragma once
#include "http_utils.h"

#include <string>
#include <sstream>
#include <map>
#include <vector>

namespace http_client
{
	/// <summary>
	/// Request struct
	/// </summary>
	struct request_t
	{
		http_method_e _method;
		std::string _host;
		int _port;
		std::string _endpoint;
		std::map<std::string, std::string> _headers;
		std::string _body;

		/// <summary>
		/// Payload to send
		/// </summary>
		std::string _final_request;

		/// <summary>
		/// Empty request
		/// </summary>
		request_t();

		/// <summary>
		/// Initialize request
		/// </summary>
		/// <param name="sPayload"></param>
		request_t(const std::string& payload, http_method_e method, const std::string& host, int port);

		/// <summary>
		/// Build a payload to send
		/// </summary>
		void build_request();
	};
};