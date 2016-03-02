#include <SpehsEngine/Time.h>
#include <SpehsEngine/SpehsEngine.h>
#include <SpehsEngine/Console.h>
#include <SpehsEngine/InputManager.h>
#include <SpehsEngine/Window.h>
#include <SpehsEngine/ApplicationData.h>
#include <SpehsEngine/PolygonBatch.h>
#include "Game.h"

void main()
{

	SpehsEngine::initialize("Network prototype");
	mainWindow->clearColor(0, 0, 0, 1.0f);
	console->addVariable("fps", applicationData->showFps);
	console->addVariable("maxfps", applicationData->maxFps);


	Game game;
	game.asioTimerTutorial5();


	SpehsEngine::uninitialize();
	getchar();
}