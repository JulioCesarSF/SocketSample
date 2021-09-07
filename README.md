# SocketSample
Socket HTTP server

## Sample server
To add endpoints you need to use RequestController add functions.

# Get
```c++
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
```

# Post
```c++
	RequestController requestController;

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
```

# Run server
```c++
	Server server("127.0.0.1", 1248);
	server.StartMultiClientListener(&requestController);
```
