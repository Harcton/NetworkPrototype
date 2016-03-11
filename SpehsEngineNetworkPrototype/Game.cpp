#include <boost/asio/ip/udp.hpp>
#include <boost/bind.hpp>
#include <boost/asio/placeholders.hpp>
#include <SpehsEngine/BitwiseOperations.h>
#include <SpehsEngine/Window.h>
#include <SpehsEngine/Time.h>
#include <SpehsEngine/InputManager.h>
#include <SpehsEngine/Console.h>
#include "Game.h"

Game::Game(std::string hostName) : resolver(ioService), socket(ioService), state(0),
	query(boost::asio::ip::udp::v4(), hostName, std::to_string(PORT_NUMBER))
{
}
Game::~Game()
{
}
void Game::exitGame()
{
	enableBit(state, GAME_EXIT_BIT);
	boost::array<unsigned char, 3> exitPacket;
	exitPacket[0] = packet::exit;
	memcpy(&exitPacket[1], &ID, sizeof(int16_t));
	socket.send(boost::asio::buffer(exitPacket));
	socket.receive(boost::asio::buffer(receiveBuffer));//TODO: make sure that server gets exit message
}
void Game::run()
{
	serverEndpoint = *resolver.resolve(query);
	socket.open(boost::asio::ip::udp::v4());
	socket.async_connect(serverEndpoint, boost::bind(&Game::connectHandler, this, boost::asio::placeholders::error));

	boost::array<unsigned char, 1> enterPacket = { packet::enter };
	socket.send(boost::asio::buffer(enterPacket));
	socket.receive(boost::asio::buffer(receiveBuffer));//TODO: if server response doens't get back, resend
	memcpy(&ID, &receiveBuffer[0], sizeof(int16_t));//Receive my ID from server

	while (!(checkBit(state, GAME_EXIT_BIT)))
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
void Game::connectHandler(const boost::system::error_code& error)
{

}
void Game::update()
{
	std::array<PlayerStateData, 1> playerStateData;

	//Gather state contents
	playerStateData[0].mouseX = inputManager->getMouseX();
	playerStateData[0].mouseY = inputManager->getMouseY();

	//Synchronous update send/receive state data
	socket.send(boost::asio::buffer(playerStateData));
	socket.receive(boost::asio::buffer(receiveBuffer));
	
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
void Game::receiveUpdate(const boost::system::error_code& error)
{

}