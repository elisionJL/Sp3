
#ifndef APPLICATION_H
#define APPLICATION_H

#include "timer.h"
// Inputs
#include "..\Inputs\KeyboardController.h"
#include "..\Inputs\MouseController.h"
class Application
{
public:
	static Application& GetInstance()
	{
		static Application app;
		return app;
	}
	void Init();
	void Run();
	void Exit();
	static bool IsKeyPressed(unsigned short key);
	static bool IsMousePressed(unsigned short key);
	static void GetCursorPos(double *xpos, double *ypos);
	static int GetWindowWidth();
	static int GetWindowHeight();

private:
	Application();
	~Application();
	// Update input devices
	void UpdateInputDevices(void);
	void PostUpdateInputDevices(void);
	//Declare a window object
	StopWatch m_timer;
};

#endif