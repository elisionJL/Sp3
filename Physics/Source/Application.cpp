
#include "Application.h"

//Include GLEW
#include <GL/glew.h>

//Include GLFW
#include <GLFW/glfw3.h>

//Include the standard C++ headers
#include <stdio.h>
#include <stdlib.h>

#include "SceneCollision.h"

GLFWwindow* m_window;
const unsigned char FPS = 60; // FPS of this game
const unsigned int frameTime = 1000 / FPS; // time for each frame
int m_width, m_height;

/**
 @brief Define the key input callback
 @param window The window to receive the the instructions
 @param key The key code
 @param scancode The scan code
 @param action The action to take
 @param mods The modifications
 */
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	CKeyboardController::GetInstance()->Update(key, action);
}

//Define an error callback
static void error_callback(int error, const char* description)
{
	fputs(description, stderr);
	_fgetchar();
}

////Define the key input callback
//static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
//{
//	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
//		glfwSetWindowShouldClose(window, GL_TRUE);
//}


void resize_callback(GLFWwindow* window, int w, int h)
{
	m_width = w;
	m_height = h;
	glViewport(0, 0, w, h);
}

bool Application::IsKeyPressed(unsigned short key)
{
	bool bKeyPressed = false;
	while (1)
	{
		if ((GetAsyncKeyState(key) & 0x8001) != 0)
			bKeyPressed = true;
		else
		{
			if (GetAsyncKeyState(key) == false)
				break;
		}
	}

	return bKeyPressed; // ((GetAsyncKeyState(key) & 0x8001) != 0);
}
/*
 @brief Callback function whenever the mouse has events
 @param window The window to receive the the instructions
 @param button An integer value of the mouse button causing the event
 @param action An integer value of the action caused by the mouse button
 @param mods An integer value storing the mods of the event
 */
	void MouseButtonCallbacks(GLFWwindow* window, int button, int action, int mods)
{
	// Send the callback to the mouse controller to handle
	if (action == GLFW_PRESS)
		CMouseController::GetInstance()->UpdateMouseButtonPressed(button);
	else
		CMouseController::GetInstance()->UpdateMouseButtonReleased(button);
}

/**
 @brief Callback function whenever the mouse has events
 @param window The window to receive the the instructions
 @param xoffset A double value of the mouse scroll offset in the x-axis
 @param yoffset A double value of the mouse scroll offset in the y-axis
 */
void MouseScrollCallbacks(GLFWwindow* window, double xoffset, double yoffset)
{
	CMouseController::GetInstance()->UpdateMouseScroll(xoffset, yoffset);
}

bool Application::IsMousePressed(unsigned short key) //0 - Left, 1 - Right, 2 - Middle
{
	return glfwGetMouseButton(m_window, key) != 0;
}
void Application::GetCursorPos(double *xpos, double *ypos)
{
	glfwGetCursorPos(m_window, xpos, ypos);
}
int Application::GetWindowWidth()
{
	return m_width;
}
int Application::GetWindowHeight()
{
	return m_height;
}

Application::Application()
{
}

Application::~Application()
{
}

void Application::Init()
{
	//Set the error callback
	glfwSetErrorCallback(error_callback);
	//Initialize GLFW
	if (!glfwInit())
	{
		exit(EXIT_FAILURE);
	}

	//Set the GLFW window creation hints - these are optional
	glfwWindowHint(GLFW_SAMPLES, 4); //Request 4x antialiasing
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); //Request a specific OpenGL version
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); //Request a specific OpenGL version
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //We don't want the old OpenGL




	//Create a window and create its OpenGL context
	m_width = 1920;
	m_height = 1080;
	m_window = glfwCreateWindow(m_width, m_height, "Physics", NULL, NULL);
	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	m_width = mode->width;
	m_height = mode->height;
	glfwWindowHint(GLFW_MAXIMIZED, GL_TRUE);

	//If the window couldn't be created
	if (!m_window)
	{
		fprintf( stderr, "Failed to open GLFW window.\n" );
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	//This function makes the context of the specified window current on the calling thread. 
	glfwMakeContextCurrent(m_window);

	//Sets the key callback
	//Set the mouse button callback function
	glfwSetMouseButtonCallback(m_window, MouseButtonCallbacks);
	glfwSetKeyCallback(m_window, key_callback);
	glfwSetWindowSizeCallback(m_window, resize_callback);

	glewExperimental = true; // Needed for core profile
	//Initialize GLEW
	GLenum err = glewInit();

	//If GLEW hasn't initialized
	if (err != GLEW_OK) 
	{
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		//return -1;
	}
}

void Application::Run()
{
	glfwWindowHint(GLFW_MAXIMIZED, 1);
	//Main Loop
	Scene *scene = new SceneCollision();
	scene->Init();

	m_timer.startTimer();    // Start timer to calculate how long it takes to render this frame
	while (!glfwWindowShouldClose(m_window) && !IsKeyPressed(VK_ESCAPE) && scene->quit == false)
	{
		scene->Update(m_timer.getElapsedTime());
		scene->Render();
		//Swap buffers
		glfwSwapBuffers(m_window);
		// Perform Post Update Input Devices
		PostUpdateInputDevices();

		// Poll events
		glfwPollEvents();

		// Update Input Devices
		UpdateInputDevices();
        m_timer.waitUntil(frameTime);       // Frame rate limiter. Limits each frame to a specified time in ms.   

	} //Check if the ESC key had been pressed or if the window had been closed
	scene->Exit();
	delete scene;
}
/**
@brief Get updates from the input devices
*/
void Application::UpdateInputDevices(void)
{
	// Update Mouse Position
	double dMouse_X, dMouse_Y;
	glfwGetCursorPos(m_window, &dMouse_X, &dMouse_Y);
	CMouseController::GetInstance()->UpdateMousePosition(dMouse_X, dMouse_Y);
}

/**
 @brief End updates from the input devices. This method is not used anymore
 */
void Application::PostUpdateInputDevices(void)
{
	CKeyboardController::GetInstance()->PostUpdate();
}

void Application::Exit()
{
	// Destroy the mouse instance
	CMouseController::GetInstance()->Destroy();
	// Destroy the keyboard instance
	CKeyboardController::GetInstance()->Destroy();
	//Close OpenGL window and terminate GLFW
	glfwDestroyWindow(m_window);
	//Finalize and clean up GLFW
	glfwTerminate();
}
