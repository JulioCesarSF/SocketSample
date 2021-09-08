#pragma once

#include "json.hpp"
#include "StatusCode.h"
#include "RequestResponse.h"

using namespace nlohmann;

/// <summary>
/// Abstract class to implement a request controller
/// </summary>
class IRequestHandler
{
public:
	/// <summary>
	/// Handle a request and returns a complete response as string
	/// </summary>
	/// <param name="payload">HTTP payload received by server</param>
	/// <returns>HTTP payload to send back to a client</returns>
	virtual std::string HandleRequest(const std::string& payload) = 0;
};

/// <summary>
/// Support namespace
/// </summary>
namespace DefaultResponse
{
	/// <summary>
	/// Response for status code 200
	/// </summary>
	/// <returns></returns>
	inline Response Ok(std::string body = "", std::string contentType = "text/plain")
	{
		Response response;
		response.statusCode = OK;
		response.headers["Content-Type"] = contentType;
		response.headers["Content-Length"] = std::to_string(body.size());
		response.AddBodyText(body);
		return response;
	}

	/// <summary>
	/// Response for status code 200
	/// </summary>
	/// <returns></returns>
	inline Response OkJson(json body)
	{
		Response response;
		std::string jsonAsString = body.dump();
		response.statusCode = OK;
		response.headers["Content-Type"] = "application/json";
		response.headers["Content-Length"] = std::to_string(jsonAsString.size());
		response.AddBodyText(jsonAsString);
		return response;
	}

	/// <summary>
	/// Response for status code 404
	/// </summary>
	/// <returns></returns>
	inline Response BadRequest()
	{
		Response response;
		response.statusCode = BAD_REQUEST;
		response.headers["Content-Type"] = "text/plain";
		std::string body = StatusCodeString()[response.statusCode];
		response.headers["Content-Length"] = std::to_string(body.size());
		response.AddBodyText(body);
		return response;
	}

	/// <summary>
	/// Response for status code 404
	/// </summary>
	/// <returns></returns>
	inline Response NotFound()
	{
		Response response;
		response.statusCode = NOT_FOUND;
		response.headers["Content-Type"] = "text/plain";
		std::string body = StatusCodeString()[response.statusCode];
		response.headers["Content-Length"] = std::to_string(body.size());
		response.AddBodyText(body);
		return response;
	}

	/// <summary>
	/// Response for status code 500
	/// </summary>
	/// <returns></returns>
	inline Response InternalServerError()
	{
		Response response;
		response.statusCode = INTERNAL_SERVER_ERROR;
		response.headers["Content-Type"] = "text/plain";
		std::string body = StatusCodeString()[response.statusCode];
		response.headers["Content-Length"] = std::to_string(body.size());
		response.AddBodyText(body);
		return response;
	}
};