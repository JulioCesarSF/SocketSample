#pragma once

#include "http_utils.h"
#include "response.h"

#include <string>
#include <sstream>
#include <map>

namespace http_server
{
	/// <summary>
	/// <summary>
	/// Request struct
	/// </summary>
	struct request_t
	{
		http_method_e _http_method;
		std::string s_http_method;
		std::string _endpoint;
		std::map<std::string, std::string> _query_string;
		std::map<std::string, std::string> _headers;
		std::string _body;
		std::string _payload; //request full text

		response_t _response;

		request_t(std::string payload);
	};
};