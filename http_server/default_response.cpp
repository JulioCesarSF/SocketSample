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
	response._headers["Connection"] = "Closed";
	response._headers["Cache-control"] = "no-cache";

	auto date_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	char c_date_time[256] = {};
 	auto now = ctime_s(c_date_time, sizeof(c_date_time), &date_time);
	std::string s_date_time(c_date_time);
	s_date_time.replace(s_date_time.find('\n'), 1, "");
	response._headers["Date"] = s_date_time;
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
	response._headers["Connection"] = "Closed";
	response._headers["Cache-control"] = "no-cache";

	auto date_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	char c_date_time[256] = {};
	auto now = ctime_s(c_date_time, sizeof(c_date_time), &date_time);
	std::string s_date_time(c_date_time);
	s_date_time.replace(s_date_time.find('\n'), 1, "");
	response._headers["Date"] = s_date_time;
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
	response._headers["Connection"] = "Closed";
	response._headers["Cache-control"] = "no-cache";

	auto date_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	char c_date_time[256] = {};
	auto now = ctime_s(c_date_time, sizeof(c_date_time), &date_time);
	std::string s_date_time(c_date_time);
	s_date_time.replace(s_date_time.find('\n'), 1, "");
	response._headers["Date"] = s_date_time;
	return response;
}

response_t http_server::not_found(std::string body)
{
	response_t response;
	response._status_code = NOT_FOUND;
	response._headers["Content-Type"] = "text/plain";
	if (body.empty())
	{
		body = status_code_string_t()[response._status_code];
	}
	response._headers["Content-Length"] = std::to_string(body.size());
	response.add_body_text(body);
	response._headers["Access-Control-Allow-Origin"] = "*";
	response._headers["Connection"] = "Closed";
	response._headers["Cache-control"] = "no-cache";

	auto date_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	char c_date_time[256] = {};
	auto now = ctime_s(c_date_time, sizeof(c_date_time), &date_time);
	std::string s_date_time(c_date_time);
	s_date_time.replace(s_date_time.find('\n'), 1, "");
	response._headers["Date"] = s_date_time;
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
	response._headers["Connection"] = "Closed";
	response._headers["Cache-control"] = "no-cache";

	auto date_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	char c_date_time[256] = {};
	auto now = ctime_s(c_date_time, sizeof(c_date_time), &date_time);
	std::string s_date_time(c_date_time);
	s_date_time.replace(s_date_time.find('\n'), 1, "");
	response._headers["Date"] = s_date_time;
	return response;
}

response_t http_server::switching_protocols(std::map<std::string, std::string> additiona_headers)
{
	response_t response;
	response._status_code = SWITCHING_PROTOCOLS;
	//response._headers["Content-Type"] = "text/plain";
	//std::string body = status_code_string_t()[response._status_code];
	//response._headers["Content-Length"] = std::to_string(body.size());
	//response.add_body_text(body);
	response._headers["Upgrade"] = "websocket";
	response._headers["Access-Control-Allow-Origin"] = "*";
	response._headers["Connection"] = "Upgrade";
	response._headers["Cache-control"] = "no-cache";

	for (const auto& header : additiona_headers)
		response._headers.emplace(header);

	auto date_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	char c_date_time[256] = {};
	auto now = ctime_s(c_date_time, sizeof(c_date_time), &date_time);
	std::string s_date_time(c_date_time);
	s_date_time.replace(s_date_time.find('\n'), 1, "");
	response._headers["Date"] = s_date_time;
	return response;
}