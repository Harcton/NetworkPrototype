#include <SpehsEngine/Time.h>
#include <SpehsEngine/SpehsEngine.h>
#include <SpehsEngine/Console.h>
#include <SpehsEngine/InputManager.h>
#include <SpehsEngine/Window.h>
#include <SpehsEngine/ApplicationData.h>
#include <SpehsEngine/PolygonBatch.h>
#include "TimerTutorial.h"
#include "TCPDaytimeTutorial.h"



void main()
{

	spehs::initialize("Network prototype");
	mainWindow->clearColor(0, 0, 0, 1.0f);
	console->addVariable("fps", applicationData->showFps);
	console->addVariable("maxfps", applicationData->maxFps);
	

	////TIMER
	//TimerTutorial timerTutorial;
	//timerTutorial.tutorial5();

	//TCP DAYTIME
	TCPDaytimeTutorial tcpDaytimeTutorial;
	tcpDaytimeTutorial.tutorial1("derrmahgerd");


	spehs::uninitialize();
	getchar();
}