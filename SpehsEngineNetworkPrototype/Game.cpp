#include <iostream>
#include <boost/asio/ip/udp.hpp>
#include <boost/bind.hpp>
#include <boost/asio/placeholders.hpp>
#include <SpehsEngine/BitwiseOperations.h>
#include <SpehsEngine/Window.h>
#include <SpehsEngine/Time.h>
#include <SpehsEngine/InputManager.h>
#include <SpehsEngine/Console.h>
#include "Game.h"

Game::Game(std::string hostName) :  state(0),
	resolverTCP(ioService),
	socketTCP(ioService),
	queryTCP(hostName, std::to_string(PORT_NUMBER_TCP)),
	resolverUDP(ioService),
	socketUDP(ioService),
	queryUDP(boost::asio::ip::udp::v4(), hostName, std::to_string(PORT_NUMBER_UDP))
{

}
Game::~Game()
{
}
void Game::exitGame()
{
	enableBit(state, GAME_EXIT_BIT);
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
			socketTCP.receive(boost::asio::buffer(receiveBuffer));
			memcpy(&ID, &receiveBuffer[0], sizeof(uint32_t));//Receive ID from server

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
			objectVisuals[i].polygon.draw();
		spehs::console::render();

		spehs::endFPS();
		spehs::drawFPS();
		mainWindow->swapBuffers();

		if (inputManager->isKeyDown(KEYBOARD_Q))
			exitGame();
	}
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
	//socketUDP.async_receive(
	//	boost::asio::buffer(receiveBuffer),
	//	boost::bind(
	//		&Game::receiveUpdate, this,
	//		boost::asio::placeholders::error,
	//		boost::asio::placeholders::bytes_transferred));
	socketUDP.receive(boost::asio::buffer(receiveBuffer));
	boost::system::error_code e;
	receiveUpdate(e, 1);
}
void Game::receiveUpdate(const boost::system::error_code& error, std::size_t bytesTransferred)
{
	unsigned objectCount;
	memcpy(&objectCount, &receiveBuffer[0], sizeof(unsigned));
	size_t offset = sizeof(objectCount);
	ObjectData objectData;//Temp location
	for (unsigned i = 0; i < objectCount; i++)
	{
		memcpy(&objectData, &receiveBuffer[0] + offset, sizeof(ObjectData));
		offset += sizeof(ObjectData);
		bool found = false;
		for (unsigned o = 0; o < objectVisuals.size(); o++)
		{
			if (objectVisuals[i].ID == objectData.ID)
			{
				objectVisuals[i].polygon.setPosition(objectData.x, objectData.y);
				found = true;
				break;
			}
		}
		if (!found)
		{
			objectVisuals.push_back(ObjectVisual());
			objectVisuals.back().ID = objectData.ID;
			objectVisuals.back().polygon.setPosition(objectData.x, objectData.y);
		}
	}
}