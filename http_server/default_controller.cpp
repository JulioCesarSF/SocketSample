#include "default_controller.h"
#include "http_utils.h"
#include "default_response.h"

#include <future>

using namespace http_server;

std::string default_controller_t::handle_request(const std::string& payload)
{
	auto task = std::async(
		[&]() -> std::string {
			controller_mutex.lock();
			if (payload.empty())
			{
				controller_mutex.unlock();
				return bad_request();
			}

			request_t request(payload);

			switch (request._http_method)
			{
			case http_method_e::GET:
			{
				auto controller_endpoint = std::find_if(end_points_get.begin(), end_points_get.end(),
					[&request](std::pair<std::string, std::function<std::string(request_t)>> const& endPoint)
					{
						return endPoint.first == request._endpoint;
					});
				if (controller_endpoint != end_points_get.end())
				{
					controller_mutex.unlock();
					return controller_endpoint->second(request);
				}
			}
			break;
			case http_method_e::POST:
			{
				auto controller_endpoint = std::find_if(end_points_post.begin(), end_points_post.end(),
					[&request](std::pair<std::string, std::function<std::string(request_t)>> const& endPoint)
					{
						return endPoint.first == request._endpoint;
					});
				if (controller_endpoint != end_points_post.end())
				{
					controller_mutex.unlock();
					return controller_endpoint->second(request);
				}
			}
			}
			controller_mutex.unlock();
			return not_found();
		});
	return task.get();
}

void default_controller_t::add_get(const std::string& endpoint, std::function<std::string(request_t)> handler)
{
	end_points_get.emplace(std::pair<std::string, std::function<std::string(request_t)>>(endpoint, handler));
}

void default_controller_t::add_post(const std::string& endpoint, std::function<std::string(request_t)> handler)
{
	end_points_post.emplace(std::pair<std::string, std::function<std::string(request_t)>>(endpoint, handler));
}

template<http_method_e method>
void default_controller_t::add_endpoint(const std::string& endpoint, std::function<std::string(request_t)> handler)
{
	switch (method)
	{
	case http_method_e::GET:
		add_get(endpoint, handler);
		break;
	case http_method_e::POST:
		add_post(endpoint, handler);
		break;
	}
}

default_controller_t::~default_controller_t()
{
	end_points_get.clear();
	end_points_post.clear();
}