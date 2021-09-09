
#include <iostream>
#include "../http_client/http_client.h"

using namespace http_client;

int main()
{
	request_sender_t sender;

	//sends a GET request
	request_t request;
	request._host = "127.0.0.1";
	request._port = 1248;
	request._endpoint = "/";
	request._method = http_method_e::GET;
	auto resp = sender.send_request(request);

	std::cout << "GET Response" << std::endl;
	std::cout << resp._payload << std::endl;

	//sends a POST request
	request._method = http_method_e::POST;
	resp = sender.send_request(request);

	std::cout << "POST Response" << std::endl;
	std::cout << resp._payload << std::endl;

	std::cin.get();

	return 0;
}