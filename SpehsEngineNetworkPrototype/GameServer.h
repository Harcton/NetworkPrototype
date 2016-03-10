#pragma once
#include <string>
#include <vector>
#include <stdint.h>
#include <glm/vec2.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/array.hpp>

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

struct Player
{
	boost::asio::ip::address address;
	int16_t position[2];
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
	void handleSend(const boost::system::error_code& error, std::size_t bytesTransferred);

	boost::asio::io_service ioService;
	boost::array<unsigned char, 1024> receiveBuffer;
	boost::asio::ip::udp::socket socket;
	boost::asio::ip::udp::endpoint myEndpoint;
	boost::asio::ip::udp::endpoint playerEndpoint;

	std::vector<Player> players;
};
