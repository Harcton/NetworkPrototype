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
	for (unsigned i = 0; i < objectVisuals.size(); i++)
		delete objectVisuals[i];
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

			//Wait for receiving return data
			socketTCP.async_receive(boost::asio::buffer(receiveBufferTCP),
				boost::bind(
					&Game::receiveHandlerTCP,
					this,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
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
	ioServiceThread.join();

	//Notify server
	exitGame();
}
void Game::update()
{
	
	//New objects
	objectMutex.lock();
	for (unsigned i = 0; i < newObjects.size(); i++)
	{
		objectVisuals.push_back(new ObjectVisual());
		objectVisuals.back()->ID = newObjects[i].ID;
		objectVisuals.back()->polygon.setPosition(newObjects[i].x - applicationData->getWindowWidthHalf(), newObjects[i].y - applicationData->getWindowWidthHalf());
	}
	newObjects.clear();
	objectMutex.unlock();
	
	//Gather state contents
	std::array<PlayerStateData, 1> playerStateData;
	idMutex.lock();
	playerStateData[0].ID = ID;
	idMutex.unlock();
	playerStateData[0].mouseX = inputManager->getMouseX();
	playerStateData[0].mouseY = inputManager->getMouseY();

	//Synchronous update send/receive state data
	socketUDP.send(boost::asio::buffer(playerStateData));
	socketUDP.receive(boost::asio::buffer(receiveBufferUDP));
	receiveUpdate();//Wait untill a server update arrives

}
void Game::render()
{
	std::lock_guard<std::recursive_mutex> objectLockGuardMutex(objectMutex);
	for (unsigned i = 0; i < objectVisuals.size(); i++)
		objectVisuals[i]->polygon.draw();
}
void Game::receiveUpdate()
{
	std::lock_guard<std::recursive_mutex> objectLockGuardMutex(objectMutex);
	unsigned objectCount;
	memcpy(&objectCount, &receiveBufferUDP[0], sizeof(unsigned));
	size_t offset = sizeof(objectCount);
	ObjectData objectData;//Temp location
	for (unsigned c = 0; c < objectCount; c++)
	{
		memcpy(&objectData, &receiveBufferUDP[offset], sizeof(ObjectData));
		offset += sizeof(ObjectData);
		for (unsigned i = 0; i < objectVisuals.size(); i++)
		{
			if (objectVisuals[i]->ID == objectData.ID)
			{
				objectVisuals[i]->polygon.setPosition(objectData.x - applicationData->getWindowWidthHalf(), objectData.y - applicationData->getWindowHeightHalf());
				break;
			}
		}
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
			offset += sizeof(uint32_t);
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
	socketTCP.async_receive(
		boost::asio::buffer(receiveBufferTCP),
		boost::bind(&Game::receiveHandlerTCP,
		this, boost::asio::placeholders::error,
		boost::asio::placeholders::bytes_transferred));
}
///////////////////////
///////////////////////