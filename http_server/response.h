#pragma once

#include "http_utils.h"

#include <string>
#include <sstream>
#include <map>
#include <vector>

namespace http_server
{
	/// <summary>
	/// Response struct
	/// </summary>
	struct response_t
	{
		std::vector<std::string> _body;
		http_status_code_t _status_code = OK;
		std::map<std::string, std::string> _headers;

		void add_body_text(std::string text);
		std::string to_string();

		operator std::string()
		{
			return this->to_string();
		}
	};
};