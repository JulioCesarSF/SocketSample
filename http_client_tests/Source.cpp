#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include "../http_client/http_client.h"
#include "benchmark.h"

#include <thread>
#include <mutex>

using namespace http_client;

int main()
{
	struct t_send_t
	{
		std::thread thread;		
		int id = 0;
		std::thread::id _id;
	};
	std::vector<t_send_t> threads;
	request_sender_t sender;
	std::mutex mutex_;
	auto sendget = [&]()
	{
		std::lock_guard<std::mutex> lck(mutex_);	

		//sends a GET request
		request_t request;
		request._host = "127.0.0.1";
		request._port = 1248;
		request._endpoint = "/";
		request._method = http_method_e::GET;
		auto thread_id = std::this_thread::get_id();
		auto t_o = std::find_if(threads.begin(), threads.end(), [&](const t_send_t& th)
			{
				return th._id == thread_id;
			});

		auto i = 0;
		if (t_o != threads.end())
			i = t_o->id;

		benchmark_t t;
		response_t resp = sender.send_request(request);
		auto elp = t.elapsed();
		std::ostringstream ss;
		
		ss << "[" << i << "] sendget status_code " << resp._status_code << " [" << elp << "]" << thread_id << "\r\n";
		
		std::cout << ss.str();

		//std::remove_if(threads.begin(), threads.end(), [&](const t_send_t& th)
		//	{
		//		return th._id == thread_id;
		//	});
	};

	auto sendpost = [&]()
	{
		std::lock_guard<std::mutex> lck(mutex_);
		

		//sends a GET request
		request_t request;
		request._host = "127.0.0.1";
		request._port = 1248;
		request._endpoint = "/join";
		request._method = http_method_e::POST;

		benchmark_t t;		
		response_t resp = sender.send_request(request);
		auto elp = t.elapsed();
		std::ostringstream ss;
		
		auto thread_id = std::this_thread::get_id();
		auto t_o = std::find_if(threads.begin(), threads.end(), [&](const t_send_t& th)
			{
				return th._id == thread_id;
			});

		auto i = 0;
		if (t_o != threads.end())
			i = t_o->id;

		ss << "[" << i << "] sendpost status_code " << resp._status_code << " [" << elp << "]" << thread_id << "\r\n";
		
		std::cout << ss.str();

		//std::remove_if(threads.begin(), threads.end(), [&](const t_send_t& th)
		//	{
		//		return th._id == thread_id;
		//	});
	};

	auto sendpost_notfound = [&]()
	{
		std::lock_guard<std::mutex> lck(mutex_);
		

		//sends a GET request
		request_t request;
		request._host = "127.0.0.1";
		request._port = 1248;
		request._endpoint = "/";
		request._method = http_method_e::POST;

		benchmark_t t;
		response_t resp = sender.send_request(request);
		auto elp = t.elapsed();
		std::ostringstream ss;

		auto thread_id = std::this_thread::get_id();
		auto t_o = std::find_if(threads.begin(), threads.end(), [&](const t_send_t& th)
			{
				return th._id == thread_id;
			});		

		auto i = 0;
		if (t_o != threads.end())
			i = t_o->id;

		ss << "[" << i << "] sendpost_notfound status_code " << resp._status_code << " [" << elp << "] " << thread_id << "\r\n";
		
		std::cout << ss.str();

		//std::remove_if(threads.begin(), threads.end(), [&](const t_send_t& th)
		//	{
		//		return th._id == thread_id;
		//	});
	};

	
	int to_run = 1000;
	//threads.reserve(to_run);
	for (int i = 0; i < to_run; i++)
	{
		std::lock_guard<std::mutex> lck(mutex_);
		t_send_t t;
		t.thread = std::thread(sendget);
		t.id = i;
		t._id = t.thread.get_id();		
		threads.emplace_back(std::move(t));
		
		
		t_send_t t2;
		t2.thread = std::thread(sendpost);
		t2.id = i;
		t2._id = t2.thread.get_id();
		threads.emplace_back(std::move(t2));

		t_send_t t3;
		t3.thread = std::thread(sendpost_notfound);
		t3.id = i;
		t3._id = t2.thread.get_id();
		threads.emplace_back(std::move(t3));
	}
	
	std::cin.get();

	return 0;
}