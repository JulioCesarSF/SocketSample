#include "Server.h"
#include "json.hpp"
using namespace nlohmann;

int main()
{
	RequestController requestController;

	requestController.AddGet("/",
		[](Request request) -> std::string
		{
			std::string messageToSend = "Hello World!";

			request.response.headers["Content-Type"] = "text/plain";
			request.response.headers["Content-Length"] = std::to_string(messageToSend.size());

			request.response.AddBodyText(messageToSend);

			return request.response.ToString();
		});

	requestController.AddGet("/json",
		[](Request request) -> std::string
		{
			json jsonBody;
			jsonBody["message"] = "Leo gay";
			std::string messageToSend = jsonBody.dump();

			request.response.headers["Content-Type"] = "application/json";
			request.response.headers["Content-Length"] = std::to_string(messageToSend.size());

			request.response.AddBodyText(messageToSend);
			return request.response.ToString();
		});

	requestController.AddPost("/process",
		[](Request request) -> std::string
		{
			json jsonBody;

			auto contentTypeReceived = request.headers["Content-Type"];
			if (contentTypeReceived.find("json") != std::string::npos)
			{
				json jToken = json::parse(request.body);
				std::string name = jToken.value<std::string>("name", "");
				jsonBody["message"] = "You sent the name: " + name;
			}

			std::string messageToSend = jsonBody.dump();
			request.response.headers["Content-Type"] = "application/json";
			request.response.headers["Content-Length"] = std::to_string(messageToSend.size());
			request.response.AddBodyText(messageToSend);
			return request.response.ToString();
		});

	requestController.AddPost("/",
		[](Request request) -> std::string
		{
			json jsonBody;

			auto contentTypeReceived = request.headers["Content-Type"];
			if (contentTypeReceived.find("json") != std::string::npos)
			{
				json jToken = json::parse(request.body);
				std::string name = jToken.value<std::string>("name", "");
				jsonBody["message"] = "You sent the name: " + name;
			}

			std::string messageToSend = jsonBody.dump();
			request.response.headers["Content-Type"] = "application/json";
			request.response.headers["Content-Length"] = std::to_string(messageToSend.size());
			request.response.AddBodyText(messageToSend);
			return request.response.ToString();
		});


	Server server("127.0.0.1", 1248);
	server.StartMultiClientListener(&requestController);

	return 0;
}