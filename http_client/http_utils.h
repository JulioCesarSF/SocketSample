#pragma once

#include <string>

namespace http_client
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
};