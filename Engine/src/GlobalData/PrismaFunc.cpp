#include "../../include/GlobalData/PrismaFunc.h"
#include "../../include/GlobalData/GlobalData.h"
#include "../../include/GlobalData/Defines.h"
#include "../../include/Helpers/Settings.h"
#include "../../include/Helpers/SettingsLoader.h"
#include "../../include/Helpers/Logger.h"

#include <iostream>

#include <fstream>

struct PrivatePrisma
{
	std::shared_ptr<Prisma::CallbackHandler> callback;
	bool initCallback = false;
	std::map<std::string, std::string> errorMap;
};

std::shared_ptr<PrivatePrisma> privatePrisma;

namespace Prisma
{
	void framebufferSizeCallback(GLFWwindow* window, int width, int height)
	{
		if (privatePrisma->callback->resize)
		{
			privatePrisma->callback->resize(width, height);
		}
		glViewport(0, 0, width, height);
	}

	void mouseCallback(GLFWwindow* window, double x, double y)
	{
		if (privatePrisma->callback->mouse)
		{
			privatePrisma->callback->mouse(x, y);
		}
	}

	void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
	{
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		if (privatePrisma->callback->mouseClick)
		{
			privatePrisma->callback->mouseClick(button, action, xpos, ypos);
		}
	}

	void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		if (privatePrisma->callback->keyboard)
		{
			privatePrisma->callback->keyboard(key, scancode, action, mods);
		}
	}

	void rollCallback(GLFWwindow* window, double x, double y)
	{
		if (privatePrisma->callback->rollMouse)
		{
			privatePrisma->callback->rollMouse(x, y);
		}
	}

	/*void GLAPIENTRY MessageCallback(GLenum source,
	                                GLenum type,
	                                GLuint id,
	                                GLenum severity,
	                                GLsizei length,
	                                const GLchar* message,
	                                const void* userParam)
	{
		// Create a unique key based on the error type and ID
		std::string errorKey = std::to_string(type) + "_" + std::to_string(id);

		// Check if the error has been logged before
		auto it = privatePrisma->errorMap.find(errorKey);
		if (it == privatePrisma->errorMap.end())
		{
			// Log the error using Logger instance
			Prisma::Logger& logger = Prisma::Logger::getInstance();

			// Define the log message string
			std::stringstream logMessage;
			logMessage << "GL CALLBACK: "
				<< (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "")
				<< " type = 0x" << std::hex << type
				<< ", severity = 0x" << std::hex << severity
				<< ", message = " << message
				<< std::dec;

			// Log the message based on its type and severity
			if (type == GL_DEBUG_TYPE_ERROR)
			{
				logger.log(Prisma::LogLevel::ERROR, logMessage.str());
			}
			else
			{
				logger.log(Prisma::LogLevel::WARN, logMessage.str());
			}

			// Store the error with the message in the map (to prevent logging it again)
			privatePrisma->errorMap[errorKey] = message;
		}
	}*/
}


Prisma::PrismaFunc::PrismaFunc()
{
	Settings settings = SettingsLoader::getInstance().getSettings();
	privatePrisma = std::make_shared<PrivatePrisma>();
	/*glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE); // Allow resizing
	glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);

	int x, y, width, height;
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	glfwGetMonitorWorkarea(monitor, &x, &y, &width, &height);

	// Set settings to match monitor work area
	settings.width = width;
	settings.height = height;
	SettingsLoader::getInstance().settings(settings);

	// Create window
	m_window = glfwCreateWindow(settings.width, settings.height, settings.name.c_str(), nullptr, nullptr);
	//glfwMaximizeWindow(m_window);
	glfwSetWindowPos(m_window, x, y);
	glfwSetWindowAttrib(m_window, GLFW_RESIZABLE, GLFW_FALSE);

	if (m_window == nullptr)
	{
		Prisma::Logger::getInstance().log(Prisma::LogLevel::ERROR,
		                                  "Window not created correctly.");
		glfwTerminate();
		return;
	}

	// Position the window to the work area (top-left corner)
	//glfwSetWindowPos(m_window, x, y);
	glfwMakeContextCurrent(m_window);
	glewInit();
	glewExperimental = GL_TRUE;
	glEnable(GL_DEPTH_TEST);
	glClearColor(CLEAR_COLOR.x, CLEAR_COLOR.y, CLEAR_COLOR.z, CLEAR_COLOR.w);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(MessageCallback, 0);
	Prisma::GlobalData::getInstance().defaultBlack().loadTexture({DIR_DEFAULT_BLACK});
	Prisma::GlobalData::getInstance().defaultWhite().loadTexture({DIR_DEFAULT_WHITE});
	Prisma::GlobalData::getInstance().defaultNormal().loadTexture({DIR_DEFAULT_NORMAL});*/

}

void Prisma::PrismaFunc::swapBuffers()
{
	glfwPollEvents();
}

void Prisma::PrismaFunc::clear()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Prisma::PrismaFunc::setCallback(std::shared_ptr<CallbackHandler> callbackHandler)
{
	privatePrisma->callback = callbackHandler;
	if (!privatePrisma->initCallback)
	{
		glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback);
		glfwSetCursorPosCallback(m_window, mouseCallback);
		glfwSetMouseButtonCallback(m_window, mouseButtonCallback);
		glfwSetInputMode(m_window, GLFW_STICKY_KEYS, GLFW_TRUE);
		glfwSetKeyCallback(m_window, keyboardCallback);
		glfwSetScrollCallback(m_window, rollCallback);
		privatePrisma->initCallback = true;
	}
}

void Prisma::PrismaFunc::closeWindow()
{
	glfwSetWindowShouldClose(m_window, true);
}

bool Prisma::PrismaFunc::shouldClose()
{
	return glfwWindowShouldClose(m_window);
}

void Prisma::PrismaFunc::hiddenMouse(bool hidden)
{
	glfwSetInputMode(m_window, GLFW_CURSOR, hidden ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
}

void Prisma::PrismaFunc::msaa(bool isMsaa, int samples)
{
	if (isMsaa)
	{
		glfwWindowHint(GLFW_SAMPLES, samples);
		glEnable(GL_MULTISAMPLE);
	}
	else
	{
		glDisable(GL_MULTISAMPLE);
	}
}

GLFWwindow* Prisma::PrismaFunc::window()
{
	return m_window;
}

void Prisma::PrismaFunc::destroy()
{
	glfwDestroyWindow(m_window);
	glfwTerminate();
}
