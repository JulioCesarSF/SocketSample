#include <iostream>

#include "../http_server/http_server.h"
#include "../http_server/default_controller.h"
#include "../http_server/default_response.h"
#include "../http_server/http_utils.h"

#include "resource.h"
#include "resource_manager.h"

#define TO_PYTHON_GET(python_function, endpoint, callback) controller.add_get(endpoint, callback)
#define TO_PYTHON_POST(python_function, endpoint, callback) controller.add_post(endpoint, callback)
#define HOST_CONFIG(ip, port)

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

	HOST_CONFIG("127.0.0.1", 80);

	TO_PYTHON_GET("default_get", "/", 
		[&](request_t request) -> std::string
		{
			return ok(rm.pre_loaded_resources[IDR_HTML1], "text/html; charset=utf-8");
		});

	TO_PYTHON_POST("dummy_post", "/dummy", 
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

	/*controller.add_get("/",
		[&](request_t request) -> std::string
		{
			return ok(rm.pre_loaded_resources[IDR_HTML1], "text/html; charset=utf-8");
		});*/

	controller.add_get("/resource/{file}",
		[&](request_t request) -> std::string
		{
			if (request._param_value == "test.js")
				return ok(rm.pre_loaded_resources[IDR_JAVASCRIPT1], "application/javascript; charset=utf-8");
			return not_found();
		});

#ifdef _WEBSOCKET
	controller.add_get("/websocket",
		[&](request_t request) -> std::string
		{
			/*
			* Calculate Sec-WebSocket-Accept 
			* https://www.rfc-editor.org/rfc/rfc6455#page-24
			*/
			auto new_raw_hash = request._headers["Sec-WebSocket-Key"] + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
			auto c_new_raw_hash = (const unsigned char*)new_raw_hash.c_str();			
			auto created = SHA1(c_new_raw_hash, new_raw_hash.size(), NULL);
			std::string server_key = http_server::base64_encode(
				reinterpret_cast<const unsigned char*>(created), strlen((char*)created));
			request._response._headers["Sec-WebSocket-Accept"] = server_key;
			return switching_protocols(request._response._headers);
		});
#endif
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

	static auto now = GetTickCount64();
	while (!exit)
	{
		if (GetAsyncKeyState(VK_F2) & 0x8000)
		{
			server.shutdown_server = true;
			exit = true;
		}
		auto after = GetTickCount64();
		auto dif = after - now;
		if (dif > 100)
		{
			//server.send_message_to_all_client("Hello World websocket");
			server.receive_messages_from_all_clients();
			now = after;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	return 0;
}