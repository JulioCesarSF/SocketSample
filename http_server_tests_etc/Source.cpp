#include <iostream>

#include "../http_server/http_server.h"
#include "../http_server/default_controller.h"
#include "../http_server/default_response.h"
#include "../http_server/http_utils.h"

#include "resource.h"
#include "resource_manager.h"

using namespace http_server;

void log(std::string msg)
{
	std::cout << msg << std::endl;
}

int main()
{
	resource_manager rm;
	rm.load();

	default_controller_t controller;

	controller.add_get("/",
		[&](request_t request) -> std::string
		{
			return ok(rm.pre_loaded_resources[IDR_HTML1], "text/html; charset=utf-8");
		});

	controller.add_get("/resource/{file}",
		[&](request_t request) -> std::string
		{
			if (request._param_value == "test.js")
				return ok(rm.pre_loaded_resources[IDR_JAVASCRIPT1], "application/javascript; charset=utf-8");
			return not_found();
		});

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

	server_t server("127.0.0.1", 80, controller);
	server.set_log_callback(
		[](std::string msg) {
			std::cout << "\r" << msg;
		});
	server.run_queue(true);

	bool exit = false;
	while (!exit)
	{
		if (GetAsyncKeyState(VK_F2) & 0x8000)
		{
			server.shutdown_server = true;
			exit = true;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	return 0;
}