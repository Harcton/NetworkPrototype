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


#include "MakeDaytimeString.h"
std::string makeDaytimeString()
{
	std::time_t now = std::time(0);
	return std::ctime(&now);
}


///CONSOLE COMMANDS
void delClient(std::vector<std::string>& words);
UDPSynchronousServer* syncUDPServer = nullptr;
std::thread* syncUDPServerThread = nullptr;
UDPAsynchronousServer* asyncUDPServer = nullptr;
std::thread* asyncUDPServerThread = nullptr;
UDPClient* udpClientPtr;
std::thread* udpClientThread;
void udpClient(std::vector<std::string>& words)
{
	if (words.size() < 2)
	{
		spehs::console::log("Host not specified!");
		return;
	}
	else if (udpClientPtr)
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
	else if (asyncUDPServer || asyncUDPServerThread)
	{
		spehs::console::log("Stopping asynchronous server");
		asyncUDPServerThread->detach();
		delete asyncUDPServerThread;
		delete asyncUDPServer;
		asyncUDPServerThread = nullptr;
		asyncUDPServer = nullptr;
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

		//Render
		spehs::console::render();

		spehs::endFPS();
		spehs::drawFPS();
		mainWindow->swapBuffers();
	}
	std::vector<std::string> placeholder;
	delServers(placeholder);
	delClient(placeholder);

	spehs::uninitialize();
}