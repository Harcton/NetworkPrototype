#pragma once
#include <stdint.h>
#include <string>
#include <vector>
#include <SpehsEngine/PolygonBatch.h>
#include <boost/asio/io_service.hpp>
#include <boost/asio/socket_base.hpp>
#include <boost/array.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/ip/tcp.hpp>
#include "Network.h"
#define GAME_EXIT_BIT 0x0001

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

class ObjectVisual
{
public:
	ObjectVisual() : polygon(4, 10, 10)
	{
		polygon.setCameraMatrixState(false);
		polygon.setColor(180, 90, 0);
	}
	spehs::PolygonBatch polygon;
	int16_t ID;
};

class Game
{
public:
	Game(std::string hostName);
	~Game();
	void run();
	void exitGame();

private:
	void update();
	void receiveUpdate(const boost::system::error_code& error, std::size_t bytesTransferred);

	//Misc
	int16_t state;
	int16_t ID;//Client id

	//Asio
	boost::asio::io_service ioService;
	//TCP
	boost::asio::ip::tcp::socket socketTCP;
	boost::asio::ip::tcp::resolver resolverTCP;
	boost::asio::ip::tcp::resolver::query queryTCP;
	boost::asio::ip::tcp::endpoint serverEndpointTCP;
	//UDP
	boost::asio::ip::udp::socket socketUDP;
	boost::asio::ip::udp::endpoint myEndpointUDP;
	boost::asio::ip::udp::endpoint serverEndpointUDP;
	boost::asio::ip::udp::resolver resolverUDP;
	boost::asio::ip::udp::resolver::query queryUDP;

	//Data
	boost::array<unsigned char, UDP_DATAGRAM_MAX> receiveBuffer;
	std::vector<ObjectVisual> objectVisuals;
};
