#include "RequestHelper.h"

//#define SERVER_IP "127.0.0.1"
//#define SERVER_PORT 1248


int main()
{
	RequestHelper rHelper;

	auto response = rHelper.Get("127.0.0.1", "", 1248);

	Log(response);

	std::cin.get();

	return 0;
}

/*
Client client(SERVER_IP, SERVER_PORT);
	if (!client.initialized) return -1;

	std::string message;
	std::atomic_bool stop = false;

	//get all messages from server
	std::thread getResponses([&]
		{
			while (!stop)
			{
				auto response = client.ReadServerResponse();
				if (!response.empty())
					Log(response);

				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		});

	getResponses.detach();

	while (!stop)
	{
		Log("Type a message:");
		std::getline(std::cin, message);
		if (message.empty()) continue;
		client.SendMessageToServer(message.c_str());

		message.clear();

		if (GetAsyncKeyState(VK_END))
			stop = true;
	}

*/