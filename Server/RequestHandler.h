#pragma once

#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <functional>

enum HttpStatusCode
{
	OK = 200,
	BAD_REQUEST = 400,
	NOT_FOUND = 404,
	METHOD_NOT_ALLOWED = 405,
	INTERNAL_SERVER_ERROR = 500,
	NOT_IMPLEMENTED = 501
};

struct StatusCode : public std::map<std::string, HttpStatusCode>
{
	StatusCode()
	{
		this->operator[]("200") = OK;
		this->operator[]("400") = BAD_REQUEST;
		this->operator[]("404") = NOT_FOUND;
		this->operator[]("405") = METHOD_NOT_ALLOWED;
		this->operator[]("500") = INTERNAL_SERVER_ERROR;
		this->operator[]("501") = NOT_IMPLEMENTED;
	};

	~StatusCode() {}
};

struct StatusCodeString : public std::map<HttpStatusCode, std::string>
{
	StatusCodeString()
	{
		this->operator[](OK) = "OK";
		this->operator[](BAD_REQUEST) = "Bad Request";
		this->operator[](NOT_FOUND) = "Not Found";
		this->operator[](METHOD_NOT_ALLOWED) = "405";
		this->operator[](INTERNAL_SERVER_ERROR) = "500";
		this->operator[](NOT_IMPLEMENTED) = "501";
	};

	~StatusCodeString() {}
};

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
		///Parse();

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

	void Parse()
	{
		if (payload.empty())
		{
			response.statusCode = StatusCode()["501"];
			std::ostringstream ss;
			ss << "HTTP/1.1 " << std::to_string(response.statusCode) << " " << StatusCodeString()[response.statusCode];
			response.AddBodyText(ss.str());
		}
		else
		{
			response.statusCode = StatusCode()["200"];
			auto cPayload = const_cast<char*>(payload.c_str());
			char* nextToken = nullptr;
			char* pToken = strtok_s(cPayload, "\r", &nextToken);
			bool firstLineParsed = false;
			bool headersParsed = false;
			do
			{
				//body delimiter
				if (strlen(pToken) == 1 && pToken[0] == '\n')
				{
					headersParsed = true;
				}

				//parse headers
				if (!headersParsed && std::string(pToken).find(":") != std::string::npos)
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
				//parse first line
				else if (!firstLineParsed)
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
						}
						pTokenWhiteSpace = strtok_s(nullptr, " ", &nextSpace);
					}
					firstLineParsed = true;
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
			} while (pToken != nullptr);
		}
	}
};


class IRequestHandler
{
public:
	virtual std::string HandleRequest(std::string payload) = 0;
};

class RequestController : public IRequestHandler
{
private:

	std::map<std::string, std::function<std::string(Request)>> endPointsGet;
	std::map<std::string, std::function<std::string(Request)>> endPointsPost;

public:

	std::string HandleRequest(std::string payload)
	{
		Request request(payload);

		if (request.verb == "GET")
		{
			auto findEndPoint = std::find_if(endPointsGet.begin(), endPointsGet.end(),
				[&request](std::pair<std::string, std::function<std::string(Request)>> const& endPoint)
				{
					return endPoint.first == request.endpoint;
				});
			if (findEndPoint != endPointsGet.end())
				return findEndPoint->second(request);
		}
		else if (request.verb == "POST")
		{
			auto findEndPoint = std::find_if(endPointsPost.begin(), endPointsPost.end(),
				[&request](std::pair<std::string, std::function<std::string(Request)>> const& endPoint)
				{
					return endPoint.first == request.endpoint;
				});
			if (findEndPoint != endPointsPost.end())
				return findEndPoint->second(request);
		}

		request.response.statusCode = StatusCode()["404"];
		return request.response.ToString();
	}

	void AddGet(std::string endPoint, std::function<std::string(Request)> handler)
	{
		endPointsGet.emplace(std::pair<std::string, std::function<std::string(Request)>>(endPoint, handler));
	}

	void AddPost(std::string endPoint, std::function<std::string(Request)> handler)
	{
		endPointsPost.emplace(std::pair<std::string, std::function<std::string(Request)>>(endPoint, handler));
	}
};