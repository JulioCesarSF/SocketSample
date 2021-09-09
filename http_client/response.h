#pragma once

namespace http_client
{
	/// <summary>
	/// Response struct
	/// </summary>
	struct response_t
	{
		std::string _protocol;
		std::string _status_code;
		std::string _phrase;
		std::map<std::string, std::string> _headers;
		std::string _body;
		std::string _payload;

		/// <summary>
		/// Default
		/// </summary>
		response_t();

		/// <summary>
		/// Initialize a response object
		/// </summary>
		/// <param name="payload">Response received as string</param>
		response_t(std::string payload);

		/// <summary>
		/// Parse payload received
		/// </summary>
		void build_response();
	};
};