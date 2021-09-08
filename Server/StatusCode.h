#pragma once

#include <string>
#include <map>

/// <summary>
/// Status codes
/// </summary>
enum HttpStatusCode
{
	OK = 200,
	BAD_REQUEST = 400,
	NOT_FOUND = 404,
	METHOD_NOT_ALLOWED = 405,
	INTERNAL_SERVER_ERROR = 500,
	NOT_IMPLEMENTED = 501
};

/// <summary>
/// Convert string to Enum
/// </summary>
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

/// <summary>
/// Convert Status code to readable string
/// </summary>
struct StatusCodeString : public std::map<HttpStatusCode, std::string>
{
	StatusCodeString()
	{
		this->operator[](OK) = "OK";
		this->operator[](BAD_REQUEST) = "Bad Request";
		this->operator[](NOT_FOUND) = "Not Found";
		this->operator[](METHOD_NOT_ALLOWED) = "405";
		this->operator[](INTERNAL_SERVER_ERROR) = "Internal Server Error";
		this->operator[](NOT_IMPLEMENTED) = "501";
	};

	~StatusCodeString() {}
};