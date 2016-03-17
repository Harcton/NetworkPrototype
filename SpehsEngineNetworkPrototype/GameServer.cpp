#include <iostream>
#include <thread>
#include <boost/bind.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/shared_ptr.hpp>
#include <SpehsEngine/Console.h>
#include <SpehsEngine/BitwiseOperations.h>
#include <SpehsEngine/Time.h>
#include <SpehsEngine/ApplicationData.h>
#include "GameServer.h"
#include "MakeDaytimeString.h"
#define PI 3.14159265359f
#define PERFORMANCE_TEST_OBJECT_COUNT 500









GameServer::GameServer() :
	acceptorTCP(ioService, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), PORT_NUMBER_TCP)),
	socketUDP(ioService, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), PORT_NUMBER_UDP)),
	state (0)
{
	//Performance test
	objectsMutex.lock();
	for (int i = 0; i < PERFORMANCE_TEST_OBJECT_COUNT; i++)
	{
		float angle = float(i) / float(PERFORMANCE_TEST_OBJECT_COUNT) * 2.0f * PI;
		objects.push_back(new Object);
		objects.back()->ID = i + 1;
		objects.back()->x = cos(angle) * float(applicationData->getWindowHeightHalf()) * float(i) / float(PERFORMANCE_TEST_OBJECT_COUNT) + applicationData->getWindowWidthHalf();
		objects.back()->y = sin(angle) * float(applicationData->getWindowHeightHalf()) * float(i) / float(PERFORMANCE_TEST_OBJECT_COUNT) + applicationData->getWindowHeightHalf();
	}
	objectsMutex.unlock();
}
GameServer::~GameServer()
{
	ioService.stop();

	for (unsigned i = 0; i < objects.size(); i++)
		delete objects[i];
	for (unsigned i = 0; i < newObjects.size(); i++)
		delete newObjects[i];
	for (unsigned i = 0; i < removedObjects.size(); i++)
		delete removedObjects[i];
}
void GameServer::exit()
{
	enableBit(state, SERVER_EXIT_BIT);
}
/////////////////
// MAIN THREAD //
void GameServer::run()
{
	startReceiveTCP();
	startReceiveUDP();

	std::thread ioServiceThread(boost::bind(&boost::asio::io_service::run, &ioService));
	while (!checkBit(state, SERVER_EXIT_BIT))
	{
		//Object pre update
		//Object update
		update();
		//Object post update
		//Chunk update
		//Physics update
		sendUpdateData();
	}
	ioServiceThread.join();
}
void GameServer::update()
{
	static float a = 0.0f;
	a += spehs::deltaTime / 50000.0f;
	if (a > 2 * PI)
		a = 0;
	objectsMutex.lock();
	for (int i = 0; i < PERFORMANCE_TEST_OBJECT_COUNT; i++)
	{
		float angle = float(i) / float(PERFORMANCE_TEST_OBJECT_COUNT) * 2.0f * PI + a;
		objects[i]->x = cos(angle) * float(applicationData->getWindowHeightHalf()) * float(i) / float(PERFORMANCE_TEST_OBJECT_COUNT) + applicationData->getWindowWidthHalf();
		objects[i]->y = sin(angle) * float(applicationData->getWindowHeightHalf()) * float(i) / float(PERFORMANCE_TEST_OBJECT_COUNT) + applicationData->getWindowHeightHalf();
	}
	objectsMutex.unlock();
}
void GameServer::sendUpdateData()
{

	//TCP packet
	std::vector<unsigned char> packetTCP;
	size_t offset = 0;
	//New objects
	newObjectsMutex.lock();
	if (newObjects.size() > 0)
	{
		packetTCP.resize(
			//Create object
			sizeof(unsigned char/*packet type*/) + sizeof(unsigned/*object count*/) + newObjects.size() * sizeof(ObjectData));
		packetTCP[offset] = packet::createObj;
		offset += sizeof(unsigned char);
		unsigned size = newObjects.size();
		memcpy(&packetTCP[offset], &size, sizeof(unsigned));
		offset += sizeof(unsigned);
		for (unsigned i = 0; i < size; i++)
		{
			//Create packet data
			ObjectData objectData;
			objectData.ID = newObjects[i]->ID;
			objectData.x = newObjects[i]->x;
			objectData.y = newObjects[i]->y;
			memcpy(&packetTCP[offset], &objectData, sizeof(ObjectData));
			offset += sizeof(ObjectData);

			//Push to objects
			objects.push_back(newObjects[i]);
		}

		//All new objects have been pushed to objects vector, clear newObjects vector
		newObjects.clear();
	}
	newObjectsMutex.unlock();
	//Removed objects
	//Chat
	////Send packet to every client
	if (packetTCP.size() > 0)
	{
		std::lock_guard<std::recursive_mutex> lockClients(clientsMutex);
		int activeClientCount = int(clients.size()) - 1;
		for (int i = 0; i < activeClientCount; i++)
		{
			clients[i]->socketMutex.lock();
			clients[i]->socket.send(boost::asio::buffer(packetTCP));
			clients[i]->socketMutex.unlock();
		}
	}

	//////UDP packet (sent to each client)
	std::vector<unsigned char> packetUDP;
	offset = 0;
	packetUDP.resize(sizeof(packet::PacketType) + sizeof(unsigned) + sizeof(ObjectData) * objects.size());
	////Update obj
	if (objects.size() > 0)
	{
		//Write packet type
		packetUDP[offset] = packet::updateObj;
		offset += sizeof(packet::PacketType);
		//Write object count
		objectsMutex.lock();
		unsigned objectCount = objects.size();
		memcpy(&packetUDP[offset], &objectCount, sizeof(unsigned));
		offset += sizeof(objectCount);
		ObjectData objectData;
		//Write objects
		for (unsigned i = 0; i < objects.size(); i++)
		{
			objectData.ID = objects[i]->ID;
			objectData.x = objects[i]->x;
			objectData.y = objects[i]->y;
			memcpy(&packetUDP[offset], &objectData, sizeof(ObjectData));
			offset += sizeof(ObjectData);
		}
		objectsMutex.unlock();
	}
	////Visual effects
	////Sound effects
	try
	{
		std::lock_guard<std::recursive_mutex> lockUdpSocket(udpSocketMutex);
		std::lock_guard<std::recursive_mutex> lockClient(clientsMutex);
		for (unsigned i = 0; i < clients.size(); i++)
		{
			if (clients[i]->udpEndpoint)
			{
				clients[i]->socketMutex.lock();
				socketUDP.send_to(boost::asio::buffer(packetUDP), *clients[i]->udpEndpoint);
				clients[i]->socketMutex.unlock();
			}
		}
	}
	catch (std::exception& e)
	{
		std::cerr << "\n" << e.what();
	}

}
/////////////////
/////////////////


/////////////////////////////
// IO SERVICE - RUN THREAD //
void GameServer::startReceiveTCP()
{
	std::lock_guard<std::recursive_mutex> lockClients(clientsMutex);
	clients.push_back(Client::create(ioService, *this));
	//Give an ID for the client
	if (clients.size() > 1)
		clients.back()->ID = clients[clients.size() - 2]->ID + 1;//Take next ID
	else
		clients.back()->ID = 1;
	if (checkBit(state, SERVER_EXIT_BIT))
	{
		clients.back()->socketMutex.lock();
		acceptorTCP.async_accept(clients.back()->socket, boost::bind(&GameServer::handleAcceptClient, this, clients.back(), boost::asio::placeholders::error));
		clients.back()->socketMutex.unlock();
	}
}
void GameServer::handleAcceptClient(boost::shared_ptr<Client> client, const boost::system::error_code& error)
{	
	clientsMutex.lock();
	//Log enter
	clients.back()->socketMutex.lock();
	spehs::console::log("Client " + std::to_string(clients.back()->ID) + " [" + clients.back()->socket.remote_endpoint().address().to_string() + "] entered the game");
	clients.back()->socketMutex.unlock();

	//Data vector
	std::vector<unsigned char> packetTCP;

	//ID
	size_t offset = 0;
	packetTCP.resize(5/*packet type + uint32*/);

	packetTCP[offset] = packet::enterID;
	offset += sizeof(unsigned char);

	memcpy(&packetTCP[offset], &clients.back()->ID, sizeof(clients.back()->ID));
	offset += sizeof(clients.back()->ID);
	clientsMutex.unlock();


	//Send all existing objects as createObj packet type
	objectsMutex.lock();
	packetTCP.resize(packetTCP.size() + sizeof(unsigned char/*packet type*/) + sizeof(unsigned/*object count*/) + objects.size() * sizeof(ObjectData));
	packetTCP[offset] = packet::createObj;
	offset += sizeof(unsigned char);

	unsigned size = objects.size();
	memcpy(&packetTCP[offset], &size, sizeof(unsigned));
	offset += sizeof(unsigned/*object count*/);

	for (unsigned i = 0; i < size; i++)
	{
		//Create packet data
		ObjectData objectData;
		objectData.ID = objects[i]->ID;
		objectData.x = objects[i]->x;
		objectData.y = objects[i]->y;
		memcpy(&packetTCP[offset], &objectData, sizeof(ObjectData));
		offset += sizeof(ObjectData);
	}
	objectsMutex.unlock();

	//Start receiving again
	clientsMutex.lock();
	clients.back()->startReceiveTCP();
	clients.back()->socketMutex.lock();
	clients.back()->socket.send(boost::asio::buffer(packetTCP));
	clients.back()->socketMutex.unlock();
	clientsMutex.unlock();
	startReceiveTCP();
	
}
void GameServer::clientExit(CLIENT_ID_TYPE ID)
{
	//Remove client from clients
	{
		std::lock_guard<std::recursive_mutex> lockClients(clientsMutex);
		for (unsigned i = 0; i < clients.size(); i++)
		{
			if (clients[i]->ID == ID)
			{
				clients[i]->socketMutex.lock();
				spehs::console::log("Client " + std::to_string(clients[i]->ID) + " [" + clients[i]->socket.remote_endpoint().address().to_string() + "] exited the game");
				clients[i]->socketMutex.unlock();
				clients.erase(clients.begin() + i);
				break;
			}
		}
	}
}
void GameServer::startReceiveUDP()
{
	if (checkBit(state, SERVER_EXIT_BIT))
	{
		std::lock_guard<std::recursive_mutex> lockUdpSocket(udpSocketMutex);
		socketUDP.async_receive_from(
			boost::asio::buffer(receiveBufferUDP), senderEndpointUDP,
			boost::bind(&GameServer::handleReceiveUDP, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
	}
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
		case packet::enterUdpEndpoint:
		{
			clientsMutex.lock();
			CLIENT_ID_TYPE id;
			memcpy(&id, &receiveBufferUDP[1], sizeof(id));
			clientsMutex.unlock();
			for (unsigned i = 0; i < clients.size(); i++)
			{
				clientsMutex.lock();
				if (clients[i]->ID == id && !clients[i]->udpEndpoint)
				{
					clients[i]->udpEndpoint = new boost::asio::ip::udp::endpoint;
					*clients[i]->udpEndpoint = senderEndpointUDP;
					clientsMutex.unlock();
					boost::array<unsigned char, 2> answer = { packet::enterUdpEndpointReceived, 1 };
					std::lock_guard<std::recursive_mutex> lockUdpSocket(udpSocketMutex);
					socketUDP.send_to(boost::asio::buffer(answer), senderEndpointUDP);//HACK: send 3 times to client for better chance of packet arrival
					socketUDP.send_to(boost::asio::buffer(answer), senderEndpointUDP);
					socketUDP.send_to(boost::asio::buffer(answer), senderEndpointUDP);
					break;
				}
				else
					clientsMutex.unlock();
			}
		}
			break;
		case packet::updateInput:
			receiveUpdate();
			break;
		}
	}

	//Start receiving again
	startReceiveUDP();
}
void GameServer::receiveUpdate()
{
	std::lock_guard<std::recursive_mutex> lockClient(clientsMutex);
	size_t offset = sizeof(packet::PacketType);
	CLIENT_ID_TYPE id;
	memcpy(&id, &receiveBufferUDP[offset], sizeof(id));
	offset += sizeof(id);
	boost::shared_ptr<Client> client(nullptr);
	for (unsigned i = 0; i < clients.size(); i++)
	{
		if (clients[i]->ID == id)
		{
			client = clients[i];
			break;
		}
	}
	if (!client)
	{
		spehs::console::warning(__FUNCTION__" client ID not found!");
		return;
	}

	//Update client input state
	std::vector<unsigned> prevHeld;
	client->heldKeysMutex.lock();
	prevHeld = client->heldKeys;
	client->heldKeys.clear();
	memcpy(&client->mouseX, &receiveBufferUDP[offset], sizeof(int16_t));
	offset += sizeof(int16_t);
	memcpy(&client->mouseY, &receiveBufferUDP[offset], sizeof(int16_t));
	offset += sizeof(int16_t);
	unsigned heldCount;
	memcpy(&heldCount, &receiveBufferUDP[offset], sizeof(unsigned));
	offset += sizeof(unsigned);
	if (heldCount > 0)
	{
		client->heldKeys.resize(heldCount);
		memcpy(&client->heldKeys[0], &receiveBufferUDP[offset], sizeof(unsigned) * heldCount);
		offset += sizeof(unsigned) * heldCount;
		std::cout << "\n[" + std::to_string(client->ID) + "] Held:  ";
		for (unsigned i = 0; i < client->heldKeys.size(); i++)
		{
			std::cout << char(client->heldKeys[i]) << "  ";
		}
	}
	client->heldKeysMutex.unlock();
}
//CLIENT (run from io service thread)
Client::Client(boost::asio::io_service& io, GameServer& s) : socket(io), server(s), udpEndpoint(nullptr){}
Client::~Client()
{
	if (udpEndpoint)
		delete udpEndpoint;
}
void Client::startReceiveTCP()
{
	if (checkBit(server.state, SERVER_EXIT_BIT))
	{
		socketMutex.lock();
		socket.async_receive(boost::asio::buffer(receiveBuffer), boost::bind(&Client::receiveHandler, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
		socketMutex.unlock();
	}
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
bool Client::isKeyDown(unsigned int keyID)
{
	heldKeysMutex.lock();
	for (unsigned i = 0; i < heldKeys.size(); i++)
	{
		if (heldKeys[i] == keyID)
		{
			return true;
			heldKeysMutex.unlock();
		}
	}
	return false;
	heldKeysMutex.unlock();
}
glm::vec2 Client::getMouseCoords()
{
	mousePosMutex.lock();
	glm::vec2 pos(mouseX, mouseY);
	mousePosMutex.unlock();
	return pos;
}
/////////////////////////////
/////////////////////////////