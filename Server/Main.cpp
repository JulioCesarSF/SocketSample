#include "Server.h"
#include "Logger.h"

using namespace nlohmann;

int main()
{
	RequestController requestController;

	requestController.AddGet("/",
		[](Request request) -> std::string
		{
			return DefaultResponse::Ok("Hello World!");
		});

	requestController.AddGet("/json",
		[](Request request) -> std::string
		{
			json jsonBody;
			jsonBody["message"] = "Leo gay";
			return DefaultResponse::OkJson(jsonBody);
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

			return DefaultResponse::OkJson(jsonBody);
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

			return DefaultResponse::OkJson(jsonBody);
		});

	Server server("127.0.0.1", 1248);
	Log("Server is running at 127.0.0.1:1248");
	server.Run(&requestController);

	return 0;
}