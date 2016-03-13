#include <boost/bind.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/shared_ptr.hpp>
#include <SpehsEngine/Console.h>
#include <SpehsEngine/Time.h>
#include "GameServer.h"
#include "MakeDaytimeString.h"

Client::Client(boost::asio::io_service& io, GameServer& s) : socket(io), server(s){}
void Client::startReceiveTCP()
{
	socket.async_receive(boost::asio::buffer(receiveBuffer), boost::bind(&Client::receiveHandler, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}
void Client::receiveHandler(const boost::system::error_code& error, std::size_t bytesTransferred)
{
	if (bytesTransferred < 1)
		return;//Must have transferred at least 1 byte (packet type)
	if (error)
		return;//TODO

	bool restartToReceive = true;
	switch (receiveBuffer[0])
	{
	default:
		spehs::console::warning("Client" + std::to_string(ID) + " received invalid packet type (TCP)!");
		break;
	case packet::enter:
		break;
	case packet::exit:
		server.clientExit(ID);
		restartToReceive = false;
		break;
	}

	if (restartToReceive)
		startReceiveTCP();
}

GameServer::GameServer() :
	acceptorTCP(ioService, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), PORT_NUMBER_TCP)),
	socketUDP(ioService, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), PORT_NUMBER_UDP)),
	objectDataUDP{}{}
GameServer::~GameServer()
{
	for (unsigned i = 0; i < objects.size(); i++)
		delete objects[i];
}
void GameServer::run()
{
	startReceiveTCP();
	startReceiveUDP();
	ioService.run();
}


void GameServer::startReceiveTCP()
{
	clients.push_back(Client::create(ioService, *this));
	//Give an ID for the client
	if (clients.size() > 1)
		clients.back()->ID = clients[clients.size() - 2]->ID + 1;//Take next ID
	else
		clients.back()->ID = 1;

	acceptorTCP.async_accept(clients.back()->socket, boost::bind(&GameServer::handleAcceptClient, this, clients.back(), boost::asio::placeholders::error));
}
void GameServer::handleAcceptClient(boost::shared_ptr<Client> client, const boost::system::error_code& error)
{
	spehs::console::log("Client " + std::to_string(clients.back()->ID) + " [" + clients.back()->socket.remote_endpoint().address().to_string() + "] entered the game");
	
	//Create object for player
	objects.push_back(new Object());
	objects.back()->ID = clients.back()->ID;

	//Send ID back to client
	boost::array<uint32_t, 1> answerID = { clients.back()->ID };
	clients.back()->socket.send(boost::asio::buffer(answerID));

	//Start receiving again
	clients.back()->startReceiveTCP();
	startReceiveTCP();
}
void GameServer::clientExit(uint32_t ID)
{
	for (unsigned i = 0; i < clients.size(); i++)
	{
		if (clients[i]->ID == ID)
		{
			spehs::console::log("Client " + std::to_string(clients[i]->ID) + "[" + clients[i]->socket.remote_endpoint().address().to_string() + "] exited the game");
			clients.erase(clients.begin() + i);
			break;
		}
	}

	//Remove the object representing the client...
	for (unsigned i = 0; i < objects.size(); i++)
	{
		if (objects[i]->ID == ID)
		{
			delete objects[i];
			objects.erase(objects.begin() + i);
			break;
		}
	}
}






void GameServer::startReceiveUDP()
{
	socketUDP.async_receive_from(
		boost::asio::buffer(receiveBufferUDP), senderEndpointUDP,
		boost::bind(&GameServer::handleReceiveUDP, this,
		boost::asio::placeholders::error,
		boost::asio::placeholders::bytes_transferred));
}
void GameServer::handleReceiveUDP(const boost::system::error_code& error, std::size_t bytesTransferred)
{
	if (!error || error == boost::asio::error::message_size)
	{
		if (LOG_NETWORK)
			spehs::console::log("Game server: data received");
		
		unsigned char type;
		memcpy(&type, &receiveBufferUDP[0], sizeof(unsigned char));
		switch (type)
		{
		default:
		case packet::invalid:
			spehs::console::error("Server: packet with invalid packet type received!");
			break;
		case packet::update:
			receiveUpdate();
			break;
		}
	}

	//Start receiving again
	startReceiveUDP();
}
void GameServer::receiveUpdate()
{
	//Mem copy so that packet becomes more readable...
	memcpy(&playerStateDataBufferUDP[0], &receiveBufferUDP[0], sizeof(PlayerStateData));

	//Do something to objects with player input...
	for (unsigned i = 0; i < objects.size(); i++)
	{
		if (objects[i]->ID == playerStateDataBufferUDP[0].ID)
		{
			objects[i]->x = playerStateDataBufferUDP[0].mouseX;
			objects[i]->y = playerStateDataBufferUDP[0].mouseY;
			if (LOG_NETWORK)
				spehs::console::log(
				"Object " + std::to_string(objects[i]->ID) + " position: " +
				std::to_string(objects[i]->x) + ", " + std::to_string(objects[i]->y));
			break;
		}
	}

	//Send return packet (object data) back to player
	//Write object count
	unsigned objectCount = objects.size();
	memcpy(&objectDataUDP[0], &objectCount, sizeof(unsigned));
	//Write objects
	size_t offset = sizeof(unsigned);
	for (unsigned i = 0; i < objects.size(); i++)
	{
		memcpy(&objectDataUDP[0] + offset, objects[i], sizeof(ObjectData));
		offset += sizeof(ObjectData);
	}
	socketUDP.send_to(boost::asio::buffer(objectDataUDP), senderEndpointUDP);//Sends to 192.168.1.<localnumberthing> instead of 192.168.1.1 -> game never receives response

}