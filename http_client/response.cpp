#include "request.h"
#include "response.h"

using namespace http_client;

http_client::response_t::response_t() :
	_payload("")
{
}

http_client::response_t::response_t(std::string payload) :
	_payload(payload)
{
	if (payload.empty())
		return;

	build_response();
}

void http_client::response_t::build_response()
{
	bool firstLineParsed = false;
	bool headersParsed = false;

	auto cPayload = const_cast<char*>(_payload.c_str());
	char* nextToken = nullptr;
	char* pToken = strtok_s(cPayload, "\r", &nextToken);

	while (pToken != nullptr)
	{
		//body delimiter
		if (strlen(pToken) == 1 && pToken[0] == '\n')
		{
			headersParsed = true;
			pToken = strtok_s(nullptr, "\r", &nextToken);
			continue;
		}

		//parse first line
		if (!firstLineParsed)
		{
			char* nextSpace = nullptr;
			char* pTokenWhiteSpace = strtok_s(pToken, " ", &nextSpace);
			while (pTokenWhiteSpace != nullptr)
			{
				std::string value = std::string(pTokenWhiteSpace);
				if (value.find('\n') != std::string::npos)
					value.erase(value.find('\n'), 1);
				if (_protocol.empty())
				{
					_protocol = value;
				}
				else if (_status_code.empty())
				{
					_status_code = value;
				}
				else if (_phrase.empty())
				{
					_phrase = value;
					firstLineParsed = true;
					break;
				}
				pTokenWhiteSpace = strtok_s(nullptr, " ", &nextSpace);
			}
			firstLineParsed = true;
		}
		//headers
		else if (!headersParsed && std::string(pToken).find(":") != std::string::npos)
		{
			char* nextDelimiter = nullptr;
			char* delimiter = strtok_s(pToken, ":", &nextDelimiter);
			std::string key = delimiter;
			if (key.find('\n') != std::string::npos)
				key.erase(key.find('\n'), 1);
			delimiter = strtok_s(nullptr, ":", &nextDelimiter);
			delimiter = delimiter == nullptr ? (char*)"\0" : delimiter;
			_headers.emplace(std::pair< std::string, std::string>(key, delimiter));
		}
		//body
		else if (headersParsed)
		{
			if (pToken != nullptr)
			{
				std::string value = std::string(pToken);
				if (value.find('\n') != std::string::npos)
					value.erase(value.find('\n'), 1);
				if (!value.empty())
					_body.append(value);
			}
		}
		pToken = strtok_s(nullptr, "\r", &nextToken);
	}
}
