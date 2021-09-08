#pragma once

#include "StatusCode.h"

#include <string>
#include <sstream>
#include <vector>
#include <map>

struct Response
{
	std::vector<std::string> body;
	HttpStatusCode statusCode = OK;
	std::map<std::string, std::string> headers;

	void AddBodyText(std::string text)
	{
		body.push_back(text);
	}

	std::string ToString()
	{
		std::string finalResponse;
		std::ostringstream ss;
		ss << "HTTP/1.1 " << statusCode << " " << StatusCodeString()[statusCode] << "\r\n";

		for (const auto keyValue : headers)
		{
			ss << keyValue.first << ": " << keyValue.second << "\r\n";
		}

		std::string bodyBuffer;
		for (const std::string& s : body)
		{
			bodyBuffer.append(s);
		}

		if (!body.empty())
			ss << "\r\n" << bodyBuffer << "\r\n";

		finalResponse.append(ss.str());
		return finalResponse;
	}

	operator std::string()
	{
		return this->ToString();
	}
};

struct Request
{
	std::string verb;
	std::string endpoint;

	std::map<std::string, std::string> headers;
	std::string body;

	std::string payload; //request full text

	Response response;

	Request(std::string sPayload)
	{
		payload = sPayload;
		if (payload.empty())
		{
			response.statusCode = BAD_REQUEST;
			std::ostringstream ss;
			ss << "HTTP/1.1 " << std::to_string(response.statusCode) << " " << StatusCodeString()[response.statusCode];
			response.AddBodyText(ss.str());
		}
		else
		{
			response.statusCode = OK;

			bool firstLineParsed = false;
			bool headersParsed = false;

			auto cPayload = const_cast<char*>(payload.c_str());
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
						if (verb.empty())
						{
							verb = value;
						}
						else if (endpoint.empty())
						{
							endpoint = value;
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
					headers.emplace(std::pair< std::string, std::string>(key, delimiter));
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
							body.append(value);
					}
				}
				pToken = strtok_s(nullptr, "\r", &nextToken);
			}
		}
	}
};
