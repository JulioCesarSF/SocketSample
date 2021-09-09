# SocketSample
Socket HTTP server

## Sample server
* To add endpoints you need to use RequestController add functions.
* To start, include files: Server.h and RequestHandler.h
* If you want json, include: json.hpp
* Link to: Ws2_32.lib

# Get
```c++
	RequestController controller;

	controller.Add<HttpMethod::GET>("/",
		[](Request request) -> std::string
		{
			return DefaultResponse::Ok("Hello World!");
		});
```

# Post
```c++
	RequestController controller;

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

			return DefaultResponse::OkJson(jsonBody);
		});
```

# Run server
```c++
	Server server("127.0.0.1", 1248);
	server.Run(&requestController);
```

# Custom RequestController
You can create your own Controller, implement this class
File to include: IRequestHandler.h
```c++
class IRequestHandler
{
public:
	virtual std::string HandleRequest(const std::string& payload) = 0;
};
```
