#include <iostream>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include "UDPSynchronousServer.h"
#include "MakeDaytimeString.h"


UDPSynchronousServer::UDPSynchronousServer(){}
UDPSynchronousServer::~UDPSynchronousServer(){}
void UDPSynchronousServer::run(std::string str)
{
	try
	{
		boost::asio::io_service ioService;
		boost::asio::ip::udp::socket socket(ioService, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 13));

		/*Wait for a client to initiate contact with us.
		The remote_endpoint object will be populated by ip::udp::socket::receive_from(). */
		while (true)
		{
			boost::array<char, 1> receiveBuffer;
			boost::asio::ip::udp::endpoint remoteEndpoint;
			boost::system::error_code error;
			socket.receive_from(boost::asio::buffer(receiveBuffer), remoteEndpoint, 0, error);

			if (error && error != boost::asio::error::message_size)
				throw boost::system::system_error(error);

			/*Determine what we are going to send back to the client. */
			std::string message = makeDaytimeString();

			/*Send the response to the remote_endpoint. */
			boost::system::error_code ignoredError;
			socket.send_to(boost::asio::buffer(message), remoteEndpoint, 0, ignoredError);
		}
	}
	catch (std::exception& e)
	{
		std::cerr << "\n" << e.what();
	}
}
