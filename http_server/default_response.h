#pragma once

#include "json.h"
#include "response.h"

#include <string>
#include <chrono>
#include <ctime>
#include <time.h>

using namespace nlohmann;

namespace http_server
{
	/// <summary>
	/// Response for status code 200
	/// </summary>
	/// <returns></returns>
	response_t ok(std::string body = "", std::string contentType = "text/plain");

	/// <summary>
	/// Response for status code 200 with json
	/// </summary>
	/// <returns></returns>
	response_t ok_json(json body);

	/// <summary>
	/// Response for status code 404
	/// </summary>
	/// <returns></returns>
	response_t bad_request();

	/// <summary>
	/// Response for status code 404
	/// </summary>
	/// <returns></returns>
	response_t not_found(std::string body = "");

	/// <summary>
	/// Response for status code 500
	/// </summary>
	/// <returns></returns>
	response_t internal_server_error();

	/// <summary>
	/// Response for status code 101
	/// </summary>
	/// <returns></returns>
	response_t switching_protocols(std::map<std::string, std::string> additiona_headers);
}