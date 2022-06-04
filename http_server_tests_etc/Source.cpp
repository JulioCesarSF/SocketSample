#include <iostream>
#include <mutex>

std::mutex log_mutex;

#include "../http_server/http_server.h"
#include "../http_server/default_controller.h"
#include "../http_server/default_response.h"
#include "../http_server/http_utils.h"

#include "resource.h"

using namespace http_server;

void log(std::string msg)
{
	std::cout << msg << std::endl;
}

std::string load_resources()
{
	HRSRC p_src = FindResource(NULL, MAKEINTRESOURCE(IDR_HTML1), RT_HTML);
	if (p_src == nullptr)
	{
		DWORD error = GetLastError();
		return "";
	}

	HGLOBAL p_handle = LoadResource(NULL, p_src);
	if (p_handle == nullptr)
	{
		CloseHandle(p_src);
		return "";		
	}

	LPVOID p_resource = LockResource(p_handle);
	if (p_resource == nullptr)
	{
		FreeResource(p_handle);
		return "";		
	}

	DWORD size = SizeofResource(NULL, p_src);
	if (size == 0)
	{
		UnlockResource(p_resource);
		FreeResource(p_handle);
		return "";
	}

	std::vector<char> buffer(size + 1);
	strcpy_s(buffer.data(), buffer.size(), static_cast<char*>(p_resource));

	UnlockResource(p_resource);
	FreeResource(p_handle);

	std::string html = std::string(buffer.begin(), buffer.end());
	return html;
}

int main()
{
	default_controller_t controller;

	std::string hello_world_page = load_resources();

	controller.add_get("/",
		[&](request_t request) -> std::string
		{
			return ok(hello_world_page, "text/html");
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

	server_t server("127.0.0.1", 1248, controller);
	server.set_log_callback(
		[](std::string msg) {
			std::cout << "\r" << msg;
		});
	server.run_queue();

	return 0;
}