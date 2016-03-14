#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/shared_ptr.hpp>
#include <SpehsEngine/Console.h>
#include <SpehsEngine/BitwiseOperations.h>
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
		//Enter practically useless packet type, enter server handled by acceptor
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
	objectDataUDP{}, state (0){}
GameServer::~GameServer()
{
	for (unsigned i = 0; i < objects.size(); i++)
		delete objects[i];
	for (unsigned i = 0; i < newObjects.size(); i++)
		delete newObjects[i];
	for (unsigned i = 0; i < removedObjects.size(); i++)
		delete removedObjects[i];
}
void GameServer::run()
{
	startReceiveTCP();
	startReceiveUDP();
	ioService.run();//TODO, blocks simulation loop below

	while (!checkBit(state, SERVER_EXIT_BIT))
	{
		//Object pre update
		//Object update
		//Object post update
		//Chunk update
		//Physics update
		sendUpdateData();
	}
}
void GameServer::sendUpdateData()
{
	//TCP packet
	std::vector<unsigned char> packetTCP;
	size_t offset = 0;
	if (newObjects.size() > 0)
	{
		packetTCP.reserve(sizeof(unsigned char/*packet type*/) + sizeof(unsigned/*object count*/) + newObjects.size() * sizeof(ObjectData));
		packetTCP[0] = packet::createObj;
		unsigned size = newObjects.size();
		memcpy(&packetTCP[1], &size, sizeof(unsigned));
		offset += sizeof(unsigned) + sizeof(unsigned char);
		for (unsigned i = 0; i < size; i++)
		{
			packetTCP.push_back(packet::createObj);
			ObjectData objectData;
			objectData.ID = newObjects[i]->ID;
			objectData.x = newObjects[i]->x;
			objectData.y = newObjects[i]->y;
			memcpy(&packetTCP[offset], &objectData, sizeof(ObjectData));
			offset += sizeof(ObjectData);
		}
	}

	for (unsigned i = 0; i < clients.size(); i++)
		clients[i]->socket.send(boost::asio::buffer(packetTCP));

	//UDP packet
	//Create object data packet for each client

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
	newObjects.push_back(new Object());
	newObjects.back()->ID = clients.back()->ID;

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
			removedObjects.push_back(objects[i]);
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
	try
	{
		/* Plain socket.send() doesn's work - the socket isn't connected to the client in this case
		socket.send_to() sends to 192.168.1.<localnumberthing> instead of 192.168.1.1 -> game never receives response
		*/
		socketUDP.send_to(boost::asio::buffer(objectDataUDP), senderEndpointUDP);		
	}
	catch (std::exception& e)
	{
		std::cerr << "\n" << e.what();
	}
}