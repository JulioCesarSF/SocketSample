#include "Server.h"
#include "Logger.h"

int main()
{
	RequestController controller;

	controller.Add<HttpMethod::GET>("/",
		[](Request request) -> std::string
		{
			return default_response::Ok("Hello World!");
		});

	controller.Add<HttpMethod::GET>("/json",
		[](Request request) -> std::string
		{
			json jsonBody;
			jsonBody["message"] = "Leo gay";
			return default_response::OkJson(jsonBody);
		});

	controller.Add<HttpMethod::POST>("/process",
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

			return default_response::OkJson(jsonBody);
		});

	controller.Add<HttpMethod::POST>("/",
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
			else
			{
				jsonBody["message"] = "Post received";
			}

			return default_response::OkJson(jsonBody);
		});

	Server server("127.0.0.1", 1248);
	Log("Server is running at 127.0.0.1:1248");
	server.Run(&controller);

	return 0;
}