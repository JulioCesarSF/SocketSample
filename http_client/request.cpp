#include "request.h"

using namespace http_client;

request_t::request_t() :
	_body(""),
	_host(""),
	_port(0),
	_method(http_method_e::GET)
{
	_method = http_method_e::GET;
}

request_t::request_t(const std::string& payload, http_method_e method, const std::string& host, int port) :
	_body(payload),
	_host(host),
	_port(port),
	_method(method)
{
	build_request();
}

void request_t::build_request()
{
	_final_request.clear();
	std::ostringstream ss;

	ss << to_string(_method) << " " << _endpoint << " " << "HTTP/1.1" << "\r\n";
	ss << "Host:" << " " << _host << "\r\n";

	for (const auto keyValue : _headers)
	{
		ss << keyValue.first << ": " << keyValue.second << "\r\n";
	}

	if (!_body.empty())
	{
		const auto size = std::to_string(_body.size());
		ss << "Content-Length" << ": " << size << "\r\n";
		ss << "\r\n";
		ss << _body;
	}

	_final_request.append(ss.str());
}