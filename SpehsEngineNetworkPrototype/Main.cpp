#include <SpehsEngine/Time.h>
#include <SpehsEngine/SpehsEngine.h>
#include <SpehsEngine/Console.h>
#include <SpehsEngine/InputManager.h>
#include <SpehsEngine/Window.h>
#include <SpehsEngine/ApplicationData.h>
#include <SpehsEngine/PolygonBatch.h>

void main()
{

	SpehsEngine::initialize("Network prototype");
	mainWindow->clearColor(0, 0, 0, 1.0f);
	console->addVariable("fps", applicationData->showFps);
	console->addVariable("maxfps", applicationData->maxFps);

	//Drawables
	SpehsEngine::PolygonBatch square(4, 0.5f, 0.5f);

	bool run = true;
	while (run)
	{
		mainWindow->clearBuffer();
		SpehsEngine::beginFPS();


		//Update
		console->update();
		inputManager->update();
		if (inputManager->isKeyDown(KEYBOARD_ESCAPE))
			run = false;

		//Render
		square.draw();
		console->render();

		SpehsEngine::endFPS();
		SpehsEngine::drawFPS();
		mainWindow->swapBuffers();
	}

	SpehsEngine::uninitialize();
}