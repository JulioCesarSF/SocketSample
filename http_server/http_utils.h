#pragma once

#include <string>
#include <map>

//base64 encode table
static const unsigned char base64_table[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

//base64 decode table
static const int B64index[256] = { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 62, 63, 62, 62, 63, 52, 53, 54, 55,
56, 57, 58, 59, 60, 61,  0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  3,  4,  5,  6,
7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,  0,
0,  0,  0, 63,  0, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51 };

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
		SWITCHING_PROTOCOLS = 101,
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

	inline std::string base64_encode(const unsigned char* src, size_t len)
	{
		unsigned char* out, * pos;
		const unsigned char* end, * in;

		size_t olen;

		olen = 4 * ((len + 2) / 3); /* 3-byte blocks to 4-byte */

		if (olen < len)
			return std::string(); /* integer overflow */

		std::string outStr;
		outStr.resize(olen);
		out = (unsigned char*)&outStr[0];

		end = src + len;
		in = src;
		pos = out;
		while (end - in >= 3) {
			*pos++ = base64_table[in[0] >> 2];
			*pos++ = base64_table[((in[0] & 0x03) << 4) | (in[1] >> 4)];
			*pos++ = base64_table[((in[1] & 0x0f) << 2) | (in[2] >> 6)];
			*pos++ = base64_table[in[2] & 0x3f];
			in += 3;
		}

		if (end - in) {
			*pos++ = base64_table[in[0] >> 2];
			if (end - in == 1) {
				*pos++ = base64_table[(in[0] & 0x03) << 4];
				*pos++ = '=';
			}
			else {
				*pos++ = base64_table[((in[0] & 0x03) << 4) |
					(in[1] >> 4)];
				*pos++ = base64_table[(in[1] & 0x0f) << 2];
			}
			*pos++ = '=';
		}

		return outStr;
	}

	inline std::string b64decode(const void* data, const size_t len)
	{
		unsigned char* p = (unsigned char*)data;
		int pad = len > 0 && (len % 4 || p[len - 1] == '=');
		const size_t L = ((len + 3) / 4 - pad) * 4;
		std::string str(L / 4 * 3 + pad, '\0');

		for (size_t i = 0, j = 0; i < L; i += 4)
		{
			int n = B64index[p[i]] << 18 | B64index[p[i + 1]] << 12 | B64index[p[i + 2]] << 6 | B64index[p[i + 3]];
			str[j++] = n >> 16;
			str[j++] = n >> 8 & 0xFF;
			str[j++] = n & 0xFF;
		}
		if (pad)
		{
			int n = B64index[p[L]] << 18 | B64index[p[L + 1]] << 12;
			str[str.size() - 1] = n >> 16;

			if (len > L + 2 && p[L + 2] != '=')
			{
				n |= B64index[p[L + 2]] << 6;
				str.push_back(n >> 8 & 0xFF);
			}
		}
		return str;
	}
};