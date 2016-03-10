#pragma once
#include <stdint.h>
#include <string>
#include <vector>
#include <boost/asio/io_service.hpp>
#include <boost/asio/socket_base.hpp>
#include <boost/array.hpp>
#include <boost/asio/ip/udp.hpp>
#define GAME_EXIT_BIT 0x0001;

namespace boost
{
	namespace asio
	{

	}
	namespace system
	{
		struct error_code;
	}
}

class Game
{
public:
	Game(std::string hostName);
	~Game();
	void run();

private:
	void connectHandler(const boost::system::error_code& error);
	void update();
	void receiveUpdate(const boost::system::error_code& error);
	int16_t state;
	boost::asio::io_service ioService;
	boost::asio::ip::udp::socket socket;
	boost::asio::ip::udp::endpoint myEndpoint;
	boost::asio::ip::udp::endpoint serverEndpoint;
	boost::asio::ip::udp::resolver resolver;
	boost::asio::ip::udp::resolver::query query;
	boost::array<unsigned char, 1024> receiveBuffer;
	boost::array<unsigned char, 1024> sendBuffer;
};

