#include <boost/bind.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/socket_base.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/shared_ptr.hpp>
#include <SpehsEngine/Console.h>
#include <SpehsEngine/Time.h>
#include "GameServer.h"
#include "Network.h"
#include "MakeDaytimeString.h"


GameServer::GameServer() : socket(ioService)
{
}
GameServer::~GameServer()
{
}
void GameServer::run()
{
	startReceive();
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

		//Manage received data
		bool newPlayer = true;
		Player* player = nullptr;
		for (unsigned i = 0; i < players.size(); i++)
		{
			if (players[i].address == playerEndpoint.address())
			{
				player = &players[i];
				newPlayer = false;
				break;
			}
		}
		if (newPlayer)
		{
			players.push_back(Player());
			players.back().address = playerEndpoint.address();
			player = &players.back();
		}

		size_t offset = 0;
		memcpy(&player->position[0], &receiveBuffer[0] + offset, sizeof(int));
		offset += sizeof(int16_t);
		memcpy(&player->position[1], &receiveBuffer[0] + offset, sizeof(int));
		offset += sizeof(int16_t);

		socket.async_send_to(boost::asio::buffer(players, players.size()), playerEndpoint,
			boost::bind(&GameServer::handleSend, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));

	}

	//Start receiving again
	startReceive();
}
void GameServer::handleSend(const boost::system::error_code& error, std::size_t bytesTransferred)
{
	if (LOG_NETWORK)
		spehs::console::log("Game server: Handle send called");
}