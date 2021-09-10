#include "default_response.h"

using namespace http_server;

response_t http_server::ok(std::string body, std::string contentType)
{
	response_t response;
	response._status_code = OK;
	response._headers["Content-Type"] = contentType;
	response._headers["Content-Length"] = std::to_string(body.size());
	response.add_body_text(body);
	response._headers["Access-Control-Allow-Origin"] = "*";
	return response;
}

response_t http_server::ok_json(json body)
{
	response_t response;
	std::string json_as_string = body.dump();
	response._status_code = OK;
	response._headers["Content-Type"] = "application/json";
	response._headers["Content-Length"] = std::to_string(json_as_string.size());
	response.add_body_text(json_as_string);
	response._headers["Access-Control-Allow-Origin"] = "*";
	return response;
}

response_t http_server::bad_request()
{
	response_t response;
	response._status_code = BAD_REQUEST;
	response._headers["Content-Type"] = "text/plain";
	std::string body = status_code_string_t()[response._status_code];
	response._headers["Content-Length"] = std::to_string(body.size());
	response.add_body_text(body);
	response._headers["Access-Control-Allow-Origin"] = "*";
	return response;
}

response_t http_server::not_found()
{
	response_t response;
	response._status_code = NOT_FOUND;
	response._headers["Content-Type"] = "text/plain";
	std::string body = status_code_string_t()[response._status_code];
	response._headers["Content-Length"] = std::to_string(body.size());
	response.add_body_text(body);
	response._headers["Access-Control-Allow-Origin"] = "*";
	return response;
}

response_t http_server::internal_server_error()
{
	response_t response;
	response._status_code = INTERNAL_SERVER_ERROR;
	response._headers["Content-Type"] = "text/plain";
	std::string body = status_code_string_t()[response._status_code];
	response._headers["Content-Length"] = std::to_string(body.size());
	response.add_body_text(body);
	response._headers["Access-Control-Allow-Origin"] = "*";
	return response;
}
