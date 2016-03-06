#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <iostream>
#include "UDPClient.h"

UDPClient::UDPClient(){}
UDPClient::~UDPClient(){}
void UDPClient::run(std::string str)
{
	try
	{
		boost::asio::io_service ioService;
		boost::asio::ip::udp::resolver resolver(ioService);
		boost::asio::ip::udp::resolver::query query(boost::asio::ip::udp::v4(), str, "daytime");
		boost::asio::ip::udp::endpoint receiverEndpoint = *resolver.resolve(query);

		boost::asio::ip::udp::socket socket(ioService);
		socket.open(boost::asio::ip::udp::v4());
		boost::array<char, 1> sendBuffer = { { 0 } };
		socket.send_to(boost::asio::buffer(sendBuffer), receiverEndpoint);

		boost::array<char, 128> receiveBuffer;
		boost::asio::ip::udp::endpoint senderEndpoint;
		size_t length = socket.receive_from(
			boost::asio::buffer(receiveBuffer), senderEndpoint);
		std::cout.write(receiveBuffer.data(), length);
	}
	catch (std::exception& e)
	{
		std::cerr << "\n" << e.what();
	}
}