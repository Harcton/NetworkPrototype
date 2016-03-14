#include <iostream>
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
void Game::exitGame()
{
	boost::array<unsigned char, 1> exitPacket;
	exitPacket[0] = packet::exit;
	socketTCP.send(boost::asio::buffer(exitPacket));
}
void Game::run()
{

	//Connect TCP
	bool connected = false;
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
			socketTCP.receive(boost::asio::buffer(receiveBufferTCP));
			memcpy(&ID, &receiveBufferTCP[0], sizeof(uint32_t));//Receive ID from server
			socketTCP.async_receive(
				boost::asio::buffer(receiveBufferTCP),
				boost::bind(&Game::receiveHandlerTCP,
				this, boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
			connected = true;
		}
		catch (std::exception& e)
		{
			std::cout << "\n" << e.what();
		}
	} while (!connected);

	//Connect UDP
	connected = false;
	do
	{
		try
		{
			serverEndpointUDP = *resolverUDP.resolve(queryUDP);
			socketUDP.open(boost::asio::ip::udp::v4());
			socketUDP.connect(serverEndpointUDP);
			connected = true;
		}
		catch (std::exception& e)
		{
			std::cout << "\n" << e.what();
		}
	} while (!connected);
	

	while (!checkBit(state, GAME_EXIT_BIT))
	{
		mainWindow->clearBuffer();
		spehs::beginFPS();

		//Update
		spehs::console::update();
		inputManager->update();
		update();

		//Render
		for (unsigned i = 0; i < objectVisuals.size(); i++)
			objectVisuals[i]->polygon.draw();
		spehs::console::render();

		spehs::endFPS();
		spehs::drawFPS();
		mainWindow->swapBuffers();

		if (inputManager->isKeyDown(KEYBOARD_Q))
			enableBit(state, GAME_EXIT_BIT);
	}

	//Notify server
	exitGame();
}
void Game::update()
{
	std::array<PlayerStateData, 1> playerStateData;

	//Gather state contents
	playerStateData[0].ID = ID;
	playerStateData[0].mouseX = inputManager->getMouseX();
	playerStateData[0].mouseY = inputManager->getMouseY();

	//Synchronous update send/receive state data
	socketUDP.send(boost::asio::buffer(playerStateData));
	socketUDP.receive(boost::asio::buffer(receiveBufferUDP));
	receiveUpdate();//Wait untill a server update arrives
}
void Game::receiveUpdate()
{
	unsigned objectCount;
	memcpy(&objectCount, &receiveBufferUDP[0], sizeof(unsigned));
	size_t offset = sizeof(objectCount);
	ObjectData objectData;//Temp location
	for (unsigned i = 0; i < objectCount; i++)
	{
		memcpy(&objectData, &receiveBufferUDP[0] + offset, sizeof(ObjectData));
		offset += sizeof(ObjectData);
		for (unsigned o = 0; o < objectVisuals.size(); o++)
		{
			if (objectVisuals[i]->ID == objectData.ID)
			{
				objectVisuals[i]->polygon.setPosition(objectData.x - applicationData->getWindowWidthHalf(), objectData.y - applicationData->getWindowHeightHalf());
				break;
			}
		}
	}
}
void Game::receiveHandlerTCP(const boost::system::error_code& error, std::size_t bytes)
{
	//Handle data sent by the server
	size_t offset = sizeof(unsigned char);
	switch (receiveBufferTCP[0])
	{
	default:
	case packet::invalid:
		break;
	case packet::createObj:
		ObjectData objectData;
		memcpy(&objectData, &receiveBufferTCP[0] + offset, sizeof(ObjectData));
		objectVisuals.push_back(new ObjectVisual());
		objectVisuals.back()->ID = objectData.ID;
		objectVisuals.back()->polygon.setPosition(objectData.x - applicationData->getWindowWidthHalf(), objectData.y - applicationData->getWindowWidthHalf());
		break;
	}

	//Start receiving again
	socketTCP.async_receive(
		boost::asio::buffer(receiveBufferTCP),
		boost::bind(&Game::receiveHandlerTCP,
		this, boost::asio::placeholders::error,
		boost::asio::placeholders::bytes_transferred));
}