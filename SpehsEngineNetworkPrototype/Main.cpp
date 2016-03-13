#include <SpehsEngine/Time.h>
#include <SpehsEngine/SpehsEngine.h>
#include <SpehsEngine/Console.h>
#include <SpehsEngine/InputManager.h>
#include <SpehsEngine/Window.h>
#include <SpehsEngine/ApplicationData.h>
#include <SpehsEngine/PolygonBatch.h>
#include <thread>
#include <boost/bind.hpp>
#include "TimerTutorial.h"
#include "TCPDaytimeTutorial.h"
#include "UDPClient.h"
#include "UDPSynchronousServer.h"
#include "UDPAsynchronousServer.h"
#include "Game.h"
#include "GameServer.h"
#include "MakeDaytimeString.h"
std::string makeDaytimeString()
{
	std::time_t now = std::time(0);
	return std::ctime(&now);
}
int loopState = 1;


///CONSOLE COMMANDS
void delClient(std::vector<std::string>& words);
UDPSynchronousServer* syncUDPServer = nullptr;
std::thread* syncUDPServerThread = nullptr;
UDPAsynchronousServer* asyncUDPServer = nullptr;
std::thread* asyncUDPServerThread = nullptr;
UDPClient* udpClientPtr;
std::thread* udpClientThread;
GameServer* gameServerPtr;
std::thread* gameServerThread;
Game* gamePtr;
std::thread* gameThread;
void udpClient(std::vector<std::string>& words)
{
	if (words.size() < 2)
	{
		spehs::console::log("Host not specified!");
		return;
	}
	if (udpClientPtr)
		delClient(words);
	udpClientPtr = new UDPClient;
	udpClientThread = new std::thread(boost::bind(&UDPClient::run, udpClientPtr, words[1]));
}
void udpSyncServer(std::vector<std::string>& words)
{
	if (asyncUDPServer || asyncUDPServerThread)
	{
		spehs::console::warning("Async server already running!");
		return;
	}
	if (syncUDPServer || syncUDPServerThread)
	{
		spehs::console::warning("Sync server already running!");
		return;
	}
	syncUDPServer = new UDPSynchronousServer;
	syncUDPServerThread = new std::thread(boost::bind(&UDPSynchronousServer::run, syncUDPServer));
}
void udpAsyncServer(std::vector<std::string>& words)
{
	if (asyncUDPServer || asyncUDPServerThread)
	{
		spehs::console::warning("Async server already running!");
		return;
	}
	if (syncUDPServer || syncUDPServerThread)
	{
		spehs::console::warning("Sync server already running!");
		return;
	}
	asyncUDPServer = new UDPAsynchronousServer;
	asyncUDPServerThread = new std::thread(boost::bind(&UDPAsynchronousServer::run, asyncUDPServer));
}
void gameServer(std::vector<std::string>& words)
{
	if (gameServerPtr || gameServerThread)
	{
		spehs::console::log("Game server already running");
		return;
	}
	spehs::console::log("Creating game server...");
	gameServerPtr = new GameServer();
	gameServerThread = new std::thread(boost::bind(&GameServer::run, gameServerPtr));
}
void game(std::vector<std::string>& words)
{
	if (gamePtr || gameThread)
	{
		spehs::console::log("Game already running");
		return;
	}
	if (words.size() < 2)
	{
		spehs::console::log("Hostname not specified!");
		return;
	}
	gamePtr = new Game(words[1]);
	loopState = 2;
}
void delClient(std::vector<std::string>& words)
{
	if (udpClientThread)
	{
		spehs::console::log("Deleting UDP client...");
		udpClientThread->detach();
		delete udpClientThread;
		delete udpClientPtr;
		udpClientThread = nullptr;
		udpClientPtr = nullptr;
	}
}
void delServers(std::vector<std::string>& words)
{
	if (syncUDPServer || syncUDPServerThread)
	{
		spehs::console::log("Stopping synchronous server");
		syncUDPServerThread->detach();
		delete syncUDPServerThread;
		delete syncUDPServer;
		syncUDPServerThread = nullptr;
		syncUDPServer = nullptr;
	}
	if (asyncUDPServer || asyncUDPServerThread)
	{
		spehs::console::log("Stopping asynchronous server");
		asyncUDPServerThread->detach();
		delete asyncUDPServerThread;
		delete asyncUDPServer;
		asyncUDPServerThread = nullptr;
		asyncUDPServer = nullptr;
	}
	if (gameServerPtr || gameServerThread)
	{
		spehs::console::log("Stopping game server");
		gameServerThread->detach();
		delete gameServerThread;
		delete gameServerPtr;
		gameServerThread = nullptr;
		gameServerPtr = nullptr;
	}
}


void main()
{

	spehs::initialize("Network prototype");
	mainWindow->clearColor(0, 0, 0, 1.0f);
	spehs::console::addVariable("fps", applicationData->showFps);
	spehs::console::addVariable("maxfps", applicationData->maxFps);
	spehs::console::addCommand("udpClient", udpClient);
	spehs::console::addCommand("udpSyncServer", udpSyncServer);
	spehs::console::addCommand("udpAsyncServer", udpAsyncServer);
	spehs::console::addCommand("delServers", delServers);
	spehs::console::addCommand("delClient", delClient);
	spehs::console::addCommand("gameServer", gameServer);
	spehs::console::addCommand("game", game);
	std::vector<std::string> placeholder = {"placeholder"};

	//placeholder.push_back("192.162.1.233");
	placeholder.push_back("85.29.96.209");
	if (1)
	{
		//gameServer(placeholder);
		//game(placeholder);
	}
	else
	{
		udpAsyncServer(placeholder);
		udpClient(placeholder);
	}

	while (loopState != 0)
	{
		
		while (loopState == 1)
		{//Display and loop plain console

			mainWindow->clearBuffer();
			spehs::beginFPS();


			//Update
			spehs::console::update();
			inputManager->update();
			if (inputManager->isKeyDown(KEYBOARD_ESCAPE))
				loopState = 0;

			//Render
			spehs::console::render();

			spehs::endFPS();
			spehs::drawFPS();
			mainWindow->swapBuffers();
		}

		while (loopState == 2)
		{//Run the game
			gamePtr->run();
			delete gamePtr;
			gamePtr = nullptr;
			loopState = 1;
		}
	}

	delServers(placeholder);
	delClient(placeholder);

	spehs::uninitialize();
}