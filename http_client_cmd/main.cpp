#include <iostream>
#include "../http_client/http_client.h"

using namespace http_client;

request_t parse_args(int argc, char** argv);

int main(int argc, char** argv)
{
	try
	{
		//read args
		auto request = parse_args(argc, argv);

		//send request
		request_sender_t sender;
		auto response = sender.send_request(request);

		//log response
		if (response._status_code.empty())
			std::cout << "could not connect to server" << std::endl;
		else
		{
			std::cout << response._status_code << std::endl;
			std::cout << response._body << std::endl;
		}		
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}

	return 0;
}

request_t parse_args(int argc, char** argv)
{
	request_t request;

	if(argc < 5)
		throw std::runtime_error("no args to parse, eg: method=get ip=127.0.0.1 port=80 endpoint=/etc");

	for (int i = 1; i < argc; i++)
	{		
		char* context = nullptr;
		char* token = strtok_s(argv[i], "=", &context);
		if(token == nullptr || context == nullptr)
			throw std::runtime_error("invalid args to parse,, eg: method=get ip=127.0.0.1 port=80 endpoint=/etc");

		if (_strcmpi(token, "method") == 0)
		{
			if(_strcmpi(context, "get") == 0)
				request._method = http_method_e::GET;
			else if (_strcmpi(context, "post") == 0)
				request._method = http_method_e::POST;
		}
		else if (_strcmpi(token, "ip") == 0)
		{			
			request._host = context;
		}
		else if (_strcmpi(token, "port") == 0)
		{
			request._port = std::stoi(context);
		}
		else if (_strcmpi(token, "endpoint") == 0)
		{
			request._endpoint = context;
		}
	}

	if (argc == 6 && request._method == http_method_e::POST)
	{
		char* context = nullptr;
		char* token = strtok_s(argv[5], "=", &context);
		if (token == nullptr || context == nullptr)
			throw std::runtime_error("invalid args to parse,, eg: method=get ip=127.0.0.1 port=80 endpoint=/etc body=text");

		auto is_post_body = _strcmpi(token, "body") == 0;
		if(!is_post_body)
			throw std::runtime_error("invalid POST args to parse,, eg: method=get ip=127.0.0.1 port=80 endpoint=/etc body=text");

		if(context == nullptr)
			throw std::runtime_error("invalid POST args to parse,, eg: method=get ip=127.0.0.1 port=80 endpoint=/etc body=text");

		request._body = context;
	}

	return request;
}