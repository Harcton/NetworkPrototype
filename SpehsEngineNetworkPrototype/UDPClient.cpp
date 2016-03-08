#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <iostream>
#include <SpehsEngine/Console.h>
#include "UDPClient.h"

UDPClient::UDPClient(){}
UDPClient::~UDPClient(){}
void UDPClient::run(std::string str)
{
	try
	{
		spehs::console::log("Creating UDP client...");
		boost::asio::io_service ioService;
		boost::asio::ip::udp::resolver resolver(ioService);
		boost::asio::ip::udp::resolver::query query(boost::asio::ip::udp::v4(), str, "daytime");
		boost::asio::ip::udp::endpoint receiverEndpoint = *resolver.resolve(query);

		boost::asio::ip::udp::socket socket(ioService);
		socket.open(boost::asio::ip::udp::v4());
		boost::array<char, 1> sendBuffer = { { 0 } };//Sends 0 to server (?)
		socket.send_to(boost::asio::buffer(sendBuffer), receiverEndpoint);

		boost::array<char, 1024> receiveBuffer;
		boost::asio::ip::udp::endpoint senderEndpoint;
		spehs::console::log("UDP client: waiting for server response...");
		size_t length = socket.receive_from(boost::asio::buffer(receiveBuffer), senderEndpoint);

		//Output received data
		std::cout << "\nUDPClient::receive buffer :";
		std::cout.write(receiveBuffer.data(), length);

		std::string receiveBufferAsString;
		receiveBufferAsString.resize(length);
		for (unsigned i = 0; i < length; i++)
			receiveBufferAsString[i] = receiveBuffer[i];
		spehs::console::log("UDPClient::receive buffer: " + receiveBufferAsString);

	}
	catch (std::exception& e)
	{
		std::cerr << "\n" << e.what();
	}
}