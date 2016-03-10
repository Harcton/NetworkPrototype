#include <boost/asio/ip/udp.hpp>
#include <boost/bind.hpp>
#include <boost/asio/placeholders.hpp>
#include <SpehsEngine/BitwiseOperations.h>
#include <SpehsEngine/Window.h>
#include <SpehsEngine/Time.h>
#include <SpehsEngine/InputManager.h>
#include <SpehsEngine/Console.h>
#include "Game.h"
#include "Network.h"

Game::Game(std::string hostName) : resolver(ioService), socket(ioService),
	query(boost::asio::ip::udp::v4(), hostName, std::to_string(PORT_NUMBER))
{
	serverEndpoint = *resolver.resolve(query);
	socket.open(boost::asio::ip::udp::v4());
	socket.async_connect(serverEndpoint, boost::bind(&Game::connectHandler, this, boost::asio::placeholders::error));
}
Game::~Game()
{
}
void Game::run()
{



	bool run = true;
	while (run)
	{
		mainWindow->clearBuffer();
		spehs::beginFPS();


		//Update
		spehs::console::update();
		inputManager->update();
		if (inputManager->isKeyDown(KEYBOARD_ESCAPE))
			run = false;
		update();

		//Render
		spehs::console::render();

		spehs::endFPS();
		spehs::drawFPS();
		mainWindow->swapBuffers();
	}
}
void Game::connectHandler(const boost::system::error_code& error)
{

}
void Game::update()
{
	int16_t x = inputManager->getMouseX();
	int16_t y = inputManager->getMouseY();
	size_t offset = 0;//Byte offset

	memcpy(&sendBuffer[0] + offset, &x, sizeof(x));
	offset += sizeof(x);
	memcpy(&sendBuffer[0] + offset, &y, sizeof(y));
	offset += sizeof(y);

	socket.send(boost::asio::buffer(sendBuffer));
	socket.receive(boost::asio::buffer(receiveBuffer));
	spehs::console::log("Game received data size: " + std::to_string(receiveBuffer.size()));
	//socket.async_receive(boost::asio::buffer(receiveBuffer), boost::bind(&Game::receiveUpdate, this, boost::asio::placeholders::error));
}
void Game::receiveUpdate(const boost::system::error_code& error)
{

}
