#pragma once
#include <string>
#include <vector>
#include <stdint.h>
#include <mutex>
#include <glm/vec2.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/socket_acceptor_service.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/array.hpp>
#include "Network.h"
#define SERVER_EXIT_BIT		0x0001

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

class Object
{
public:
	Object() : ID(0), x(0), y(0){}
	int32_t ID;
	int16_t x;
	int16_t y;
};


class GameServer;
class Client : public boost::enable_shared_from_this<Client>
{
public:
	static boost::shared_ptr<Client> create(boost::asio::io_service& ioService, GameServer& server)
	{
		return  boost::shared_ptr<Client>(new Client(ioService, server));
	}
	void startReceiveTCP();
	void receiveHandler(const boost::system::error_code& error, std::size_t bytesTransferred);

	GameServer& server;
	int32_t ID;
	boost::asio::ip::tcp::socket socket;
	boost::array<unsigned char, 128> receiveBuffer;
private:
	Client(boost::asio::io_service& ioService, GameServer& server);
};


class GameServer
{
/**What goes through...
TCP?
	IN:
		-Enter/Exit marks
		-Chat input
		-Player fleet build data (enter)
	OUT:
		-Object create/destroy
		-Chat output
		-Player fleet build data (save)
UDP?
	IN:
		-Player input state
	OUT:
		-Object data
*/
public:
	GameServer();
	~GameServer();
	void run();
	void gameLoop();

	void clientExit(uint32_t ID);

private:
	void startReceiveTCP();
	void startReceiveUDP();
	void handleAcceptClient(boost::shared_ptr<Client> client, const boost::system::error_code& error);
	void handleReceiveUDP(const boost::system::error_code& error, std::size_t bytesTransferred);
	void receiveUpdate();//UDP
	void sendUpdateData();
	

	int16_t state;

	////Asio
	boost::asio::io_service ioService;
	//TCP
	boost::asio::ip::tcp::acceptor acceptorTCP;
	//UDP
	boost::asio::ip::udp::socket socketUDP;
	boost::asio::ip::udp::endpoint senderEndpointUDP;

	////Network data
	//TCP
	//UDP
	boost::array<unsigned char, UDP_DATAGRAM_MAX> receiveBufferUDP;
	boost::array<unsigned char, UDP_DATAGRAM_MAX> objectDataUDP;//Outgoing object data packet
	std::array<PlayerStateData, 1> playerStateDataBufferUDP;//Buffer for memcopying data from receive buffer into readable format
	
	std::recursive_mutex objectMutex;
	std::vector<Object*> objects;
	std::vector<Object*> newObjects;
	std::vector<Object*> removedObjects;
	
	std::recursive_mutex clientMutex;
	std::vector<boost::shared_ptr<Client>> clients;
};
