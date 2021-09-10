#pragma once

#include <string>
#include <map>

namespace http_server
{
	/// <summary>
	/// HTTP methods
	/// </summary>
	enum class http_method_e : uint16_t
	{
		NONE,
		GET,
		POST
	};

	/// <summary>
	/// Convert a given HttpMethod to string
	/// </summary>
	/// <param name="method"></param>
	/// <returns></returns>
	inline std::string to_string(http_method_e method)
	{
		switch (method)
		{
		case http_method_e::GET:
			return "GET";
		case http_method_e::POST:
			return "POST";
		}

		return "GET";
	}

	/// <summary>
	/// Convert a given method as string to enum
	/// </summary>
	/// <param name="method"></param>
	/// <returns></returns>
	inline http_method_e to_string(const std::string& method)
	{
		http_method_e eMethod = http_method_e::GET;
		if (method == "GET") eMethod = http_method_e::GET;
		else if (method == "POST") eMethod = http_method_e::POST;
		return eMethod;
	}

	/// <summary>
	/// Http status codes
	/// </summary>
	enum http_status_code_t
	{
		OK = 200,
		BAD_REQUEST = 400,
		NOT_FOUND = 404,
		METHOD_NOT_ALLOWED = 405,
		INTERNAL_SERVER_ERROR = 500,
		NOT_IMPLEMENTED = 501
	};

	/// <summary>
	/// Convert a given string to status code
	/// </summary>
	struct status_code_t : public std::map<std::string, http_status_code_t>
	{
		status_code_t();
		virtual ~status_code_t();
	};

	/// <summary>
	/// Convert a given status code to string
	/// </summary>
	struct status_code_string_t : public std::map<http_status_code_t, std::string>
	{
		status_code_string_t();
		virtual ~status_code_string_t();
	};
};