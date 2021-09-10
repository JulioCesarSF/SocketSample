#pragma once

#include "json.h"
#include "response.h"

#include <string>

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
	response_t not_found();

	/// <summary>
	/// Response for status code 500
	/// </summary>
	/// <returns></returns>
	response_t internal_server_error();
}