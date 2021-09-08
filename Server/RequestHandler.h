#pragma once

#include "IRequestHandler.h"
#include <functional>

/// <summary>
/// Default Request controller
/// </summary>
class RequestController : public IRequestHandler
{
private:

	/// <summary>
	/// Endpoints for GET verb
	/// </summary>
	std::unordered_map<std::string, std::function<std::string(Request)>> endPointsGet;

	/// <summary>
	/// Endpoints for POST verb
	/// </summary>
	std::unordered_map<std::string, std::function<std::string(Request)>> endPointsPost;

public:

	/// <summary>
	/// Handle Request and response
	/// </summary>
	/// <param name="payload"></param>
	/// <returns></returns>
	std::string HandleRequest(const std::string& payload)
	{
		if (payload.empty())
			return DefaultResponse::BadRequest();

		Request request(payload);

		if (request.verb == "GET")
		{
			auto findEndPoint = std::find_if(endPointsGet.begin(), endPointsGet.end(),
				[&request](std::pair<std::string, std::function<std::string(Request)>> const& endPoint)
				{
					return endPoint.first == request.endpoint;
				});
			if (findEndPoint != endPointsGet.end())
				return findEndPoint->second(request);
		}
		else if (request.verb == "POST")
		{
			auto findEndPoint = std::find_if(endPointsPost.begin(), endPointsPost.end(),
				[&request](std::pair<std::string, std::function<std::string(Request)>> const& endPoint)
				{
					return endPoint.first == request.endpoint;
				});
			if (findEndPoint != endPointsPost.end())
				return findEndPoint->second(request);
		}

		return DefaultResponse::NotFound();
	}

	/// <summary>
	/// Add a new GET endpoint
	/// </summary>
	/// <param name="endPoint">Endpoint eg. /person</param>
	/// <param name="handler">function to handle the request and response</param>
	void AddGet(const std::string& endPoint, const std::function<std::string(Request)>& handler)
	{
		endPointsGet.emplace(std::pair<std::string, std::function<std::string(Request)>>(endPoint, handler));
	}

	/// <summary>
	/// Add a new POST endpoint
	/// </summary>
	/// <param name="endPoint">Endpoint eg. /person</param>
	/// <param name="handler">function to handle the request and response</param>
	void AddPost(const std::string& endPoint, const std::function<std::string(Request)>& handler)
	{
		endPointsPost.emplace(std::pair<std::string, std::function<std::string(Request)>>(endPoint, handler));
	}

	~RequestController()
	{
		endPointsGet.clear();
		endPointsPost.clear();
	}
};