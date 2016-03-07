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
void udpClient(std::vector<std::string>& words)
{
	UDPClient client;
	//client.run(words[1]);
	client.run("192.168.1.37");
}
UDPSynchronousServer* syncUDPServer = nullptr;
std::thread* syncUDPServerThread = nullptr;
UDPAsynchronousServer* asyncUDPServer = nullptr;
std::thread* asyncUDPServerThread = nullptr;
void udpSyncServer(std::vector<std::string>& words)
{
	if (asyncUDPServer || asyncUDPServerThread)
	{
		console->warning("Async server already running!");
		return;
	}
	if (syncUDPServer || syncUDPServerThread)
	{
		console->warning("Sync server already running!");
		return;
	}
	syncUDPServer = new UDPSynchronousServer;
	syncUDPServerThread = new std::thread(boost::bind(&UDPSynchronousServer::run, syncUDPServer));
}
void udpAsyncServer(std::vector<std::string>& words)
{
	if (asyncUDPServer || asyncUDPServerThread)
	{
		console->warning("Async server already running!");
		return;
	}
	if (syncUDPServer || syncUDPServerThread)
	{
		console->warning("Sync server already running!");
		return;
	}
	asyncUDPServer = new UDPAsynchronousServer;
	asyncUDPServerThread = new std::thread(boost::bind(&UDPAsynchronousServer::run, asyncUDPServer));
}
void stopServer(std::vector<std::string>& words)
{
	if (syncUDPServer || syncUDPServerThread)
	{
		console->log("Stopping synchronous server");
		syncUDPServerThread->detach();
		delete syncUDPServerThread;
		delete syncUDPServer;
		syncUDPServerThread = nullptr;
		syncUDPServer = nullptr;
		return;
	}
	else if (asyncUDPServer || asyncUDPServerThread)
	{
		console->log("Stopping asynchronous server");
		asyncUDPServerThread->detach();
		delete asyncUDPServerThread;
		delete asyncUDPServer;
		asyncUDPServerThread = nullptr;
		asyncUDPServer = nullptr;
		return;
	}
}


void main()
{

	spehs::initialize("Network prototype");
	mainWindow->clearColor(0, 0, 0, 1.0f);
	console->addVariable("fps", applicationData->showFps);
	console->addVariable("maxfps", applicationData->maxFps);
	console->addConsoleCommand("udpClient", udpClient);
	console->addConsoleCommand("udpSyncServer", udpSyncServer);
	console->addConsoleCommand("udpAsyncServer", udpAsyncServer);
	console->addConsoleCommand("stopServer", stopServer);

	bool run = true;
	while (run)
	{
		mainWindow->clearBuffer();
		spehs::beginFPS();


		//Update
		console->update();
		inputManager->update();
		if (inputManager->isKeyDown(KEYBOARD_ESCAPE))
			run = false;

		//Render
		console->render();

		spehs::endFPS();
		spehs::drawFPS();
		mainWindow->swapBuffers();
	}	

	if (syncUDPServerThread)
	{
		syncUDPServerThread->detach();
		delete syncUDPServerThread;
		delete syncUDPServer;
	}
	if (asyncUDPServerThread)
	{
		asyncUDPServerThread->detach();
		delete asyncUDPServerThread;
		delete asyncUDPServer;
	}
	spehs::uninitialize();
}