#include <iostream>
#include <boost/asio/io_service.hpp>
#include <boost/bind.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/asio/buffer.hpp>
#include <SpehsEngine/Console.h>
#include "UDPAsynchronousServer.h"
#include "MakeDaytimeString.h"
#include "Network.h"

UDPAsynchronousServer::UDPAsynchronousServer()
{
}
UDPAsynchronousServer::~UDPAsynchronousServer()
{
}
void UDPAsynchronousServer::run()
{
	spehs::console::log("Creating asynchronous UDP server...");
	try
	{
		boost::asio::io_service ioService;
		UDPServer server(ioService);
		ioService.run();
	}
	catch (std::exception& e)
	{
		std::cerr << "\n" << e.what();
	}
}


UDPServer::UDPServer(boost::asio::io_service& io_service) :
socket(io_service, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), PORT_NUMBER_UDP))
{
	startReceive();
}
void UDPServer::startReceive()
{
	socket.async_receive_from(
		boost::asio::buffer(receiveBuffer), senderEndpoint,
		boost::bind(&UDPServer::handleReceive, this,
		boost::asio::placeholders::error,
		boost::asio::placeholders::bytes_transferred));
}
void UDPServer::handleReceive(const boost::system::error_code& error, std::size_t bytesTransferred)
{
	//Start receiving again
	startReceive();

	if (!error || error == boost::asio::error::message_size)
	{
		if (LOG_NETWORK)
			spehs::console::log("Async server: data received");
		boost::shared_ptr<std::string> message(new std::string("Async server:" + makeDaytimeString()));
		socket.async_send_to(boost::asio::buffer(*message), senderEndpoint,
			boost::bind(&UDPServer::handleSend, this, message,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
	}
}
void UDPServer::handleSend(boost::shared_ptr<std::string> message, const boost::system::error_code& error, std::size_t bytesTransferred)
{
	if (LOG_NETWORK)
		spehs::console::log("Async server: Handle send called");
}