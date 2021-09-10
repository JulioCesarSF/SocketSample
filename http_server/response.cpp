#include "response.h"
using namespace http_server;

void response_t::add_body_text(std::string text)
{
	_body.push_back(text);
}

std::string response_t::to_string()
{
	std::string finalResponse;
	std::ostringstream ss;
	ss << "HTTP/1.1 " << _status_code << " " << status_code_string_t()[_status_code] << "\r\n";

	for (const auto keyValue : _headers)
	{
		ss << keyValue.first << ": " << keyValue.second << "\r\n";
	}

	std::string bodyBuffer;
	for (const std::string& s : _body)
	{
		bodyBuffer.append(s);
	}

	if (!_body.empty())
		ss << "\r\n" << bodyBuffer << "\r\n";

	finalResponse.append(ss.str());
	return finalResponse;
}
