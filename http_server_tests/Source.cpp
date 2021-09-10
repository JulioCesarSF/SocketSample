#include <iostream>
#include "../http_server/http_server.h"
#include "../http_server/default_controller.h"
#include "../http_server/default_response.h"
#include "../http_server/http_utils.h"

using namespace http_server;

int main()
{
	default_controller_t controller;

	controller.add_get("/",
		[](request_t request) -> std::string
		{
			return ok();
		});

	controller.add_post("/json",
		[](request_t request) -> std::string
		{
			json response_json;
			response_json["message"] = "post received";

			if (request._headers["Content-Type"].find("json") != std::string::npos)
			{
				json request_json = json::parse(request._body);
				response_json["message"] = "Received this: " + request_json.value("message", "");
			}

			return ok_json(response_json);
		});

	server_t server("127.0.0.1", 1248);
	server.run(&controller);

	return 0;
}