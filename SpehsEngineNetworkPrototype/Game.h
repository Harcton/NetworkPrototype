#pragma once
#include <stdint.h>
#include <string>
#include <vector>
#include <SpehsEngine/PolygonBatch.h>
#include <boost/asio/io_service.hpp>
#include <boost/asio/socket_base.hpp>
#include <boost/array.hpp>
#include <boost/asio/ip/udp.hpp>
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
	void connectHandler(const boost::system::error_code& error);
	void update();
	void receiveUpdate(const boost::system::error_code& error, std::size_t bytesTransferred);

	//Misc
	int16_t state;
	int16_t ID;//Client id

	//Asio
	boost::asio::io_service ioService;
	boost::asio::ip::udp::socket socket;
	boost::asio::ip::udp::endpoint myEndpoint;
	boost::asio::ip::udp::endpoint serverEndpoint;
	boost::asio::ip::udp::resolver resolver;
	boost::asio::ip::udp::resolver::query query;

	//Data
	boost::array<unsigned char, UDP_DATAGRAM_MAX> receiveBuffer;
	std::vector<ObjectVisual> objectVisuals;
};
