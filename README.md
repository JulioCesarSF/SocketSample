# SocketSample
Socket HTTP server

## Sample server
* To add endpoints you need to use default_controller_t add functions.
* To start, include files: http_server.h, default_controller.h, default_response.h" and http_utils.h"
* If you want json, include: json.hpp
* Link to: Ws2_32.lib
* Visual Studio 2022 (v143)

# Run the sample
* Build project: http_client_test and http_server_tests_etc
* Server sample contains a log with the number of requests handled and display an average value time to handle requests

# VcPgk (OpenSSL) for websocket implementation
* Follow the instruction here: https://vcpkg.io/en/getting-started.html
* Install OpenSSL Package: .\vcpkg install openssl and .\vcpkg install openssl:x64-windows
* Now add this preprocessor: _WEBSOCKET , if you want to build with websocket support

# Get
```c++
	default_controller_t controller;

	controller.add_get("/",
		[](request_t request) -> std::string
		{
			return ok("Hello");
		});

	controller.add_get("/html",
		[&](request_t request) -> std::string
		{
			return ok(hello_world_page, "text/html; charset=utf-8");
		});

	controller.add_get("/resource/{file}",
		[&](request_t request) -> std::string
		{
			if (request._param_value == "test.js")
				return ok(test_js, "application/javascript; charset=utf-8");
			return not_found();
		});
```

# Post
```c++
	default_controller_t controller;

	controller.add_post("/join",
		[](request_t request) -> std::string
		{
			if (request._headers["Content-Type"].find("json") != std::string::npos)
			{
				json request_json = json::parse(request._body);
				auto user_name = request_json.value("username", "");
				if (user_name.empty()) return not_found("username missing");
			}
			else
			{
				return not_found("username missing");
			}

			return ok();
		});
```

# Run server
```c++
server_t server("127.0.0.1", 1248, controller);
server.run_queue();
```

# Custom RequestController
* You can create your own Controller, implement this class
* File to include: request_handler.h
```c++
	/// <summary>
	/// Abstract class to create a controller
	/// </summary>
	class request_handler_i
	{
	public:

		/// <summary>
		/// Retrieve a list of all endpoint for a given controller
		/// </summary>
		/// <returns></returns>
		virtual std::vector<request_item_t> get_endpoints() = 0;

		/// <summary>
		/// Controller name
		/// </summary>
		/// <returns></returns>
		virtual const char* controller_name() = 0;

		/// <summary>
		/// Process a given payload
		/// </summary>
		/// <param name="payload">Incoming request</param>
		/// <returns>http response as string</returns>
		virtual std::string handle_request(const request_t& request) = 0;
	};
```
# Server log callback
* To add a log callback call set_log_callback function