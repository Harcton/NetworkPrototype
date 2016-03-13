#include <iostream>
#include <SpehsEngine/Console.h>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include "UDPClient.h"
#include "Network.h"

UDPClient::UDPClient(){}
UDPClient::~UDPClient(){}
void UDPClient::run(std::string str)
{
	try
	{
		spehs::console::log("Creating UDP client...");
		boost::asio::io_service ioService;
		boost::asio::ip::udp::resolver resolver(ioService);
		boost::asio::ip::udp::resolver::query query(boost::asio::ip::udp::v4(), str, std::to_string(PORT_NUMBER_UDP));

		//Getting server endpoint
		boost::asio::ip::udp::endpoint serverEndpoint = *resolver.resolve(query);

		//Opening and connecting socket
		boost::asio::ip::udp::socket socket(ioService);
		socket.open(boost::asio::ip::udp::v4());
		socket.async_connect(serverEndpoint, boost::bind(&UDPClient::connectHandler, this, boost::asio::placeholders::error));

		boost::array<char, 1> sendBuffer = { { 0 } };//Sends 0 to server (?)
		boost::array<char, 1024> receiveBuffer;
		std::string receiveBufferAsString;
		int sameCount = 0;
		boost::asio::ip::udp::endpoint senderEndpoint;

		while (true)
		{
			//socket.send_to(boost::asio::buffer(sendBuffer), serverEndpoint);
			socket.send(boost::asio::buffer(sendBuffer));

			if (LOG_NETWORK)
				spehs::console::log("UDPClient: waiting for server response...");
			//size_t length = socket.receive_from(boost::asio::buffer(receiveBuffer), senderEndpoint);
			size_t length = socket.receive(boost::asio::buffer(receiveBuffer));

			//Output received data
			if (LOG_NETWORK)
			{
				std::cout << "\nUDPClient::receive buffer :";
				std::cout.write(receiveBuffer.data(), length);
			}

			bool same = true;
			receiveBufferAsString.resize(length);
			for (unsigned i = 0; i < length; i++)
			{
				if (receiveBufferAsString[i] != receiveBuffer[i])
				{
					receiveBufferAsString[i] = receiveBuffer[i];
					same = false;
				}
			}

			if (same)
				++sameCount;
			else
			{
				spehs::console::log("UDPClient::receive buffer: x" + std::to_string(sameCount) + ": " + receiveBufferAsString);
				sameCount = 0;
			}

		}
	}
	catch (std::exception& e)
	{
		std::cerr << "\n" << e.what();
	}
}
void UDPClient::connectHandler(const boost::system::error_code& error)
{
	if (LOG_NETWORK)
		spehs::console::log("UDPClient: connect handler");
}