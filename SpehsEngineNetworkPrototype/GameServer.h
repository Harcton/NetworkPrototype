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
	/**This class represents spehs game object class,
	containing dynamic amount of data (parts mostly).
	This data can be partly local to the server, meaning that it will not be sent to the clients at any time*/
public:
	Object() : ID(0), x(0), y(0){}
	uint32_t ID;
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
	~Client();
	friend class GameServer;
	void startReceiveTCP();
	void receiveHandler(const boost::system::error_code& error, std::size_t bytesTransferred);

	CLIENT_ID_TYPE ID;
	boost::asio::ip::tcp::socket socket;
	boost::asio::ip::udp::endpoint* udpEndpoint;
	boost::array<unsigned char, 128> receiveBuffer;
	std::recursive_mutex socketMutex;

	//Input
	bool isKeyDown(unsigned int keyID);
	glm::vec2 getMouseCoords();

private:
	Client(boost::asio::io_service& ioService, GameServer& server);
	GameServer& server;
	std::mutex heldKeysMutex;
	std::mutex mousePosMutex;
	std::vector<unsigned> heldKeys;
	int16_t mouseX;
	int16_t mouseY;
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
	friend class Client;
	void run();
	void gameLoop();
	void exit();

	void clientExit(CLIENT_ID_TYPE ID);

private:
	void update();

	void startReceiveTCP();
	void startReceiveUDP();
	void handleAcceptClient(boost::shared_ptr<Client> client, const boost::system::error_code& error);
	void handleReceiveUDP(const boost::system::error_code& error, std::size_t bytesTransferred);
	void receiveUpdate();//UDP
	void sendUpdateData();
	

	int16_t state;
	std::vector<Object*> objects;
	std::vector<Object*> newObjects;
	std::vector<Object*> removedObjects;
	std::vector<boost::shared_ptr<Client>> clients;
	
	//Mutex
	std::recursive_mutex udpSocketMutex;
	std::recursive_mutex clientsMutex;
	std::recursive_mutex objectsMutex;
	std::recursive_mutex newObjectsMutex;
	std::recursive_mutex removedObjectsMutex;

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
	boost::array<unsigned char, UDP_DATAGRAM_SIZE> receiveBufferUDP;
	
};
