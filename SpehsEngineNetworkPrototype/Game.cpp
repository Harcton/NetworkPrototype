#include <iostream>
#include <thread>
#include <boost/asio/ip/udp.hpp>
#include <boost/bind.hpp>
#include <boost/asio/placeholders.hpp>
#include <SpehsEngine/BitwiseOperations.h>
#include <SpehsEngine/Window.h>
#include <SpehsEngine/Time.h>
#include <SpehsEngine/InputManager.h>
#include <SpehsEngine/Console.h>
#include <SpehsEngine/ApplicationData.h>
#include <SpehsEngine/Camera2D.h>
#include "Game.h"

Game::Game(std::string hostName) :  state(0),
	resolverTCP(ioService),
	socketTCP(ioService),
	queryTCP(hostName, std::to_string(PORT_NUMBER_TCP)),
	resolverUDP(ioService),
	socketUDP(ioService),
	queryUDP(boost::asio::ip::udp::v4(), hostName, std::to_string(PORT_NUMBER_UDP))
{
	spehs::Camera2D cam;//
	cam.initialize();//JUUSOTODO: PrimitiveBatch.setCameraMatrixState(false) requires this to be done otherwise no work
}
Game::~Game()
{
	for (auto it = objectVisuals.begin(); it != objectVisuals.end(); it++)
		delete it->second;
	exitGame();
}
/////////////////
// MAIN THREAD //
void Game::exitGame()
{
	boost::array<unsigned char, 1> exitPacket;
	exitPacket[0] = packet::exit;
	socketTCP.send(boost::asio::buffer(exitPacket));
}
void Game::run()
{

	//Connect TCP
	bool success = false;
	do
	{
		try
		{
			serverEndpointTCP = *resolverTCP.resolve(queryTCP);
			socketTCP.open(boost::asio::ip::tcp::v4());
			socketTCP.connect(serverEndpointTCP);

			//Send enter packet to server
			boost::array<unsigned char, 1> enterPacket = { packet::enter };
			socketTCP.send(boost::asio::buffer(enterPacket));
			socketTCP.send(boost::asio::buffer(enterPacket));
			socketTCP.send(boost::asio::buffer(enterPacket));

			//Wait for receiving return data
			size_t bytes = socketTCP.receive(boost::asio::buffer(receiveBufferTCP));
			boost::system::error_code e;
			receiveHandlerTCP(e, bytes);
			success = true;
		}
		catch (std::exception& e)
		{
			std::cout << "\n" << e.what();
		}
	} while (!success);

	//Connect UDP
	success = false;
	do
	{
		try
		{
			serverEndpointUDP = *resolverUDP.resolve(queryUDP);
			socketUDP.open(boost::asio::ip::udp::v4());
			socketUDP.connect(serverEndpointUDP);
			boost::array<unsigned char, sizeof(CLIENT_ID_TYPE) + sizeof(packet::PacketType)> enterUDP;
			enterUDP[0] = packet::enterUdpEndpoint;
			memcpy(&enterUDP[sizeof(packet::PacketType)], &ID, sizeof(ID));
			socketUDP.send(boost::asio::buffer(enterUDP));
			socketUDP.receive(boost::asio::buffer(enterUDP));//Wait for server response
			success = true;
		}
		catch (std::exception& e)
		{
			std::cout << "\n" << e.what();
		}
	} while (!success);
	

	std::thread ioServiceThread(boost::bind(&boost::asio::io_service::run, &ioService));
	while (!checkBit(state, GAME_EXIT_BIT))
	{
		mainWindow->clearBuffer();
		spehs::beginFPS();

		//Update
		spehs::console::update();
		inputManager->update();
		update();

		//Render
		render();
		spehs::console::render();

		spehs::endFPS();
		spehs::drawFPS();
		mainWindow->swapBuffers();

		if (inputManager->isKeyDown(KEYBOARD_Q))
			enableBit(state, GAME_EXIT_BIT);
	}

	//Notify server
	exitGame();
	ioServiceThread.join();
}
void Game::update()
{
	
	//New objects
	objectMutex.lock();
	for (unsigned i = 0; i < newObjects.size(); i++)
	{
		std::pair<std::unordered_map<uint32_t, ObjectVisual*>::iterator, bool> insert = objectVisuals.insert({ newObjects[i].ID, new ObjectVisual() });
		if (insert.second)
			insert.first->second->polygon.setPosition(newObjects[i].x - applicationData->getWindowWidthHalf(), newObjects[i].y - applicationData->getWindowWidthHalf());
	}
	newObjects.clear();
	objectMutex.unlock();
	
	//Gather state contents
	size_t offset = 0;
	std::vector<unsigned char> inputPacket(sizeof(packet::PacketType) + sizeof(CLIENT_ID_TYPE) + sizeof(int16_t) * 2 + sizeof(unsigned)/*Reserve elements for atleast id + mouse pos + held buttons count*/);
	//Packet type
	inputPacket[offset] = packet::updateInput;
	offset += sizeof(packet::PacketType);
	//ID
	idMutex.lock();
	memcpy(&inputPacket[offset], &ID, sizeof(ID));
	offset += sizeof(ID);
	idMutex.unlock();
	//Mouse x/y
	int16_t mousePosVal(inputManager->getMouseX());
	memcpy(&inputPacket[offset], &mousePosVal, sizeof(mousePosVal));
	offset += sizeof(mousePosVal);
	mousePosVal = inputManager->getMouseY();
	memcpy(&inputPacket[offset], &mousePosVal, sizeof(mousePosVal));
	offset += sizeof(mousePosVal);
	////Pressed buttons
	//Check every held button
	std::vector<unsigned> held;
	for (auto it = inputManager->keyMap.begin(); it != inputManager->keyMap.end(); it++)
	{
		if (it->second)
			held.push_back(it->first);
	}
	unsigned size = held.size();
	memcpy(&inputPacket[offset], &size, sizeof(unsigned));
	offset += sizeof(size);
	if (size > 0)
	{
		inputPacket.resize(inputPacket.size() + sizeof(unsigned) * size);
		memcpy(&inputPacket[offset], &held[0], sizeof(unsigned) * size);
		offset += sizeof(unsigned) * size;
	}

	//Synchronous update send/receive state data
	socketUDP.send(boost::asio::buffer(inputPacket));
	socketUDP.receive(boost::asio::buffer(receiveBufferUDP));
	receiveUpdate();//Wait untill a server update arrives

}
void Game::render()
{
	std::lock_guard<std::recursive_mutex> objectLockGuardMutex(objectMutex);
	for (auto it = objectVisuals.begin(); it != objectVisuals.end(); it++)
		it->second->polygon.draw();
}
void Game::receiveUpdate()
{
	size_t offset = sizeof(packet::PacketType);
	std::lock_guard<std::recursive_mutex> objectLockGuardMutex(objectMutex);
	switch (receiveBufferUDP[offset - sizeof(packet::PacketType)])
	{
	default:
	case packet::invalid:
		break;
	case packet::updateObj:
		unsigned objectCount;
		memcpy(&objectCount, &receiveBufferUDP[offset], sizeof(unsigned));
		offset += sizeof(objectCount);
		ObjectData objectData;//Temp location
		for (unsigned c = 0; c < objectCount; c++)
		{
			memcpy(&objectData, &receiveBufferUDP[offset], sizeof(ObjectData));
			offset += sizeof(ObjectData);
			auto it = objectVisuals.find(objectData.ID);
			if (it != objectVisuals.end())
				it->second->polygon.setPosition(objectData.x - applicationData->getWindowWidthHalf(), objectData.y - applicationData->getWindowHeightHalf());
		}
		break;
	}
}
/////////////////
/////////////////






///////////////////////
// IO SERVICE THREAD //
void Game::receiveHandlerTCP(const boost::system::error_code& error, std::size_t bytes)
{
	//Handle data sent by the server
	size_t offset = 0;//Byte offset from buffer begin
	do
	{
		offset += 1;
		switch (packet::PacketType(receiveBufferTCP[offset - 1]))
		{
		default:
		case packet::invalid:
			spehs::console::error(__FUNCTION__" invalid packet type!");
			break;
		case packet::enterID:
		{
			std::lock_guard<std::recursive_mutex> IDLockGuardMutex(idMutex);
			memcpy(&ID, &receiveBufferTCP[offset], sizeof(sizeof(ID)));
			offset += sizeof(CLIENT_ID_TYPE);
		}
		break;
		case packet::createObj:
		{
			std::lock_guard<std::recursive_mutex> objectLockGuardMutex(objectMutex);
			unsigned count;
			memcpy(&count, &receiveBufferTCP[offset], sizeof(unsigned));
			offset += sizeof(unsigned);
			for (unsigned i = 0; i < count; i++)
			{
				newObjects.push_back(ObjectData());
				memcpy(&newObjects.back(), &receiveBufferTCP[offset], sizeof(ObjectData));
				offset += sizeof(ObjectData);
			}
		}
			break;
		}
	} while (offset < bytes);


	//Start receiving again
	if (checkBit(state, GAME_EXIT_BIT))
		socketTCP.async_receive(
			boost::asio::buffer(receiveBufferTCP),
			boost::bind(&Game::receiveHandlerTCP,
			this, boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
}
///////////////////////
///////////////////////