#pragma once
#include <string>
#include <vector>
#include <stdint.h>
#include <glm/vec2.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/array.hpp>
#include "Network.h"

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

class Client
{
public:
	int16_t ID;
};
class Object
{
public:
	Object() : ID(0), x(0), y(0){}
	int16_t ID;
	int16_t x;
	int16_t y;
};

class GameServer
{
public:
	GameServer();
	~GameServer();
	void run();

private:
	void startReceive();
	void handleReceive(const boost::system::error_code& error, std::size_t bytesTransferred);
	void receiveEnter();
	void receiveExit();
	void receiveUpdate();

	//Asio
	boost::asio::io_service ioService;
	boost::asio::ip::udp::socket socket;
	boost::asio::ip::udp::endpoint senderEndpoint;

	//Data
	boost::array<unsigned char, UDP_DATAGRAM_MAX> receiveBuffer;
	boost::array<unsigned char, UDP_DATAGRAM_MAX> objectData;//Outgoing object data packet
	std::array<PlayerStateData, 1> playerStateDataBuffer;//Buffer for memcopying data from receive buffer into readable format
	std::vector<Object*> objects;
	std::vector<Client> clients;
};
