#include "request.h"
using namespace http_server;

request_t::request_t(std::string payload)
{
	_payload = payload;
	if (_payload.empty())
	{
		_response._status_code = BAD_REQUEST;
		std::ostringstream ss;
		ss << "HTTP/1.1 " << std::to_string(_response._status_code) << " " << status_code_string_t()[_response._status_code];
		_response.add_body_text(ss.str());
	}
	else
	{
		_response._status_code = OK;

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
					if (s_http_method.empty())
					{
						s_http_method = value;
						_http_method = to_string(value);
					}
					else if (_endpoint.empty())
					{
						//	"/?file=test.txt&key=value"
						//	"/product?file=test.txt&key=value"
						auto query_string_start = value.find("?");
						if (query_string_start == std::string::npos)
						{
							_endpoint = value;
							firstLineParsed = true;
						}
						else // parse query string
						{
							_endpoint = value.substr(0, query_string_start);
							auto raw_query = value.substr(query_string_start + 1);
							auto c_raw_query = const_cast<char*>(raw_query.c_str());

							char* query_token = nullptr;
							char* query_key_token = strtok_s(c_raw_query, "&", &query_token);

							while (query_key_token != nullptr)
							{
								char* key_context = nullptr;
								char* key = strtok_s(query_key_token, "=", &key_context);
								char* value = strtok_s(nullptr, "=", &key_context);
								_query_string.emplace(std::pair< std::string, std::string>{ std::string(key), std::string(value) });

								query_key_token = strtok_s(nullptr, "&", &query_token);
							}

							firstLineParsed = true;
						}

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
				std::string s_value(delimiter);
				if (s_value[0] == ' ')
					s_value.erase(0, 1);
				_headers.emplace(std::pair< std::string, std::string>(key, s_value));
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
}