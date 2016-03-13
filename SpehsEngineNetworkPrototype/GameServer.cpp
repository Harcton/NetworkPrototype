#include <boost/bind.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/socket_base.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/shared_ptr.hpp>
#include <SpehsEngine/Console.h>
#include <SpehsEngine/Time.h>
#include "GameServer.h"
#include "MakeDaytimeString.h"


GameServer::GameServer() :
socket(ioService, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), PORT_NUMBER)),
objectData{}
{
}
GameServer::~GameServer()
{
	for (unsigned i = 0; i < objects.size(); i++)
		delete objects[i];
}
void GameServer::run()
{
	startReceive();
	ioService.run();
}
void GameServer::startReceive()
{
	socket.async_receive_from(
		boost::asio::buffer(receiveBuffer), playerEndpoint,
		boost::bind(&GameServer::handleReceive, this,
		boost::asio::placeholders::error,
		boost::asio::placeholders::bytes_transferred));
}
void GameServer::handleReceive(const boost::system::error_code& error, std::size_t bytesTransferred)
{
	if (!error || error == boost::asio::error::message_size)
	{
		if (LOG_NETWORK)
			spehs::console::log("Game server: data received");
		
		unsigned char type;
		memcpy(&type, &receiveBuffer[0], sizeof(unsigned char));
		switch (type)
		{
		default:
		case packet::invalid:
			spehs::console::error("Server: packet with invalid packet type received!");
		case packet::enter:
			receiveEnter();
			break;
		case packet::exit:
			receiveExit();
			break;
		case packet::update:
			receiveUpdate();
			break;
		}
	}

	//Start receiving again
	startReceive();
}
void GameServer::receiveEnter()
{
	clients.push_back(Client());
	if (clients.size() > 1)
		clients.back().ID = clients[clients.size() - 2].ID + 1;//Take next ID
	else
		clients.back().ID = 1;
	spehs::console::log("Client " + std::to_string(clients.back().ID) + " [" + playerEndpoint.address().to_string() + "] entered the game");
	
	//Create object for player
	objects.push_back(new Object());
	objects.back()->ID = clients.back().ID;

	//Send ID back to client
	boost::array<int16_t, 1> answerID = { clients.back().ID };
	socket.send_to(boost::asio::buffer(answerID), playerEndpoint);
}
void GameServer::receiveExit()
{
	boost::array<unsigned char, 1> answer = { 1 };
	int16_t exitID;
	memcpy(&exitID, &receiveBuffer[1], sizeof(int16_t));
	socket.send_to(boost::asio::buffer(answer), playerEndpoint);

	for (unsigned i = 0; i < clients.size(); i++)
	{
		if (clients[i].ID == exitID)
		{
			spehs::console::log("Client " + std::to_string(clients[i].ID) + "[" + playerEndpoint.address().to_string() + "] exited the game");
			clients.erase(clients.begin() + i);
			break;
		}
	}
	for (unsigned i = 0; i < objects.size(); i++)
	{
		if (objects[i]->ID == exitID)
		{
			delete objects[i];
			objects.erase(objects.begin() + i);
			break;
		}
	}
}
void GameServer::receiveUpdate()
{
	//Mem copy so that packet becomes more readable...
	memcpy(&playerStateDataBuffer[0], &receiveBuffer[0], sizeof(PlayerStateData));

	//Do something to objects with player input
	for (unsigned i = 0; i < objects.size(); i++)
	{
		if (objects[i]->ID == playerStateDataBuffer[0].ID)
		{
			objects[i]->x = playerStateDataBuffer[0].mouseX;
			objects[i]->y = playerStateDataBuffer[0].mouseY;
			spehs::console::log(
				"Object " + std::to_string(objects[i]->ID) + " position: " +
				std::to_string(objects[i]->x) + ", " + std::to_string(objects[i]->y));
			break;
		}
	}

	//Send return packet (object data) back to player
	//Write object count
	unsigned objectCount = objects.size();
	memcpy(&objectData[0], &objectCount, sizeof(unsigned));
	//Write objects
	size_t offset = sizeof(unsigned);
	for (unsigned i = 0; i < objects.size(); i++)
	{
		memcpy(&objectData[0] + offset, objects[i], sizeof(ObjectData));
		offset += sizeof(ObjectData);
	}
	socket.send_to(boost::asio::buffer(objectData), playerEndpoint);//Sends to 192.168.1.<localnumberthing> instead of 192.168.1.1 -> game never receives response

}