#include <iostream>
#include <mutex>

std::mutex log_mutex;

#include "../http_server/http_server.h"
#include "../http_server/default_controller.h"
#include "../http_server/default_response.h"
#include "../http_server/http_utils.h"

using namespace http_server;

void log(std::string msg)
{
	std::cout << msg << std::endl;
}

int main()
{
	default_controller_t controller;

	controller.add_get("/",
		[](request_t request) -> std::string
		{
			return ok("Hello");
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

	server_t server("127.0.0.1", 1248, 
		[] (std::string msg) {
			std::cout << msg << std::endl;
		});
	server.run_new(&controller);

	return 0;
}