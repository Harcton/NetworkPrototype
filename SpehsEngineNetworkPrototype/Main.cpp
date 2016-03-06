#include <SpehsEngine/Time.h>
#include <SpehsEngine/SpehsEngine.h>
#include <SpehsEngine/Console.h>
#include <SpehsEngine/InputManager.h>
#include <SpehsEngine/Window.h>
#include <SpehsEngine/ApplicationData.h>
#include <SpehsEngine/PolygonBatch.h>
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



void udpClient(std::vector<std::string>& words)
{
	UDPClient client;
	client.run(words[1]);
}
void udpSyncServer(std::vector<std::string>& words)
{
	UDPSynchronousServer server;
	server.run(words[1]);
}
void main()
{

	spehs::initialize("Network prototype");
	mainWindow->clearColor(0, 0, 0, 1.0f);
	console->addVariable("fps", applicationData->showFps);
	console->addVariable("maxfps", applicationData->maxFps);
	console->addConsoleCommand("udpClient", udpClient);
	console->addConsoleCommand("udpSyncServer", udpSyncServer);

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

	spehs::uninitialize();
}