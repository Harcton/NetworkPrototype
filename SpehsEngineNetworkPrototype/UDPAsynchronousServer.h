#pragma once
#include <string>
#include <boost/asio/socket_base.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/array.hpp>
#include <boost/shared_ptr.hpp>

namespace boost
{
	namespace asio
	{
		class io_service;
	}
	namespace system
	{
		struct error_code;
	}
}


class UDPAsynchronousServer
{//Starts the server
public:
	UDPAsynchronousServer();
	~UDPAsynchronousServer();

	void run();
};

//The actual server class...
class UDPServer
{
public:
	UDPServer(boost::asio::io_service& io_service);

private:
	inline void startReceive();
	void handleReceive(const boost::system::error_code& error, std::size_t bytesTransferred);
	void handleSend(boost::shared_ptr<std::string> message, const boost::system::error_code& error, std::size_t bytesTransferred);

	boost::asio::ip::udp::socket socket;
	boost::asio::ip::udp::endpoint senderEndpoint;
	boost::array<char, 1> receiveBuffer;
};