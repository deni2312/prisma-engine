#include "../../include/GlobalData/PrismaFunc.h"
#include "../../include/GlobalData/GlobalData.h"
#include "../../include/GlobalData/Defines.h"
#include "../../include/Helpers/Settings.h"
#include "../../include/Helpers/SettingsLoader.h"
#include "../../include/Helpers/Logger.h"

#include <iostream>

#include <fstream>


#include "Graphics/GraphicsEngineD3D11/interface/EngineFactoryD3D11.h"
#include "Graphics/GraphicsEngineD3D12/interface/EngineFactoryD3D12.h"
#include "Graphics/GraphicsEngineOpenGL/interface/EngineFactoryOpenGL.h"
#include "Graphics/GraphicsEngineVulkan/interface/EngineFactoryVk.h"

using namespace Diligent;


struct PrivatePrisma
{
	std::shared_ptr<Prisma::CallbackHandler> callback;
	bool initCallback = false;
	std::map<std::string, std::string> errorMap;
	Diligent::RefCntAutoPtr<Diligent::ISwapChain> m_pSwapChain;
	MSG msg = { 0 };
};

std::shared_ptr<PrivatePrisma> privatePrisma;

namespace Prisma
{
	void framebufferSizeCallback(int width, int height)
	{
		if (privatePrisma->callback->resize)
		{
			privatePrisma->callback->resize(width, height);
		}
	}

	void mouseCallback(GLFWwindow* window, double x, double y)
	{
		if (privatePrisma->callback->mouse)
		{
			privatePrisma->callback->mouse(x, y);
		}
	}

	void mouseButtonCallback(int button, Prisma::CallbackHandler::ACTION action, int mods)
	{
		double xpos, ypos;
		if (privatePrisma->callback->mouseClick)
		{
			privatePrisma->callback->mouseClick(button, action, xpos, ypos);
		}
	}

	void keyboardCallback(int key, Prisma::CallbackHandler::ACTION action)
	{
		if (privatePrisma->callback->keyboard)
		{
			privatePrisma->callback->keyboard(key, action);
		}
	}

	void rollCallback(double x, double y)
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

Prisma::PrismaFunc::ContextData& Prisma::PrismaFunc::contextData()
{
	return m_contextData;
}

LRESULT CALLBACK MessageProc(HWND, UINT, WPARAM, LPARAM);
// Called every time the NativeNativeAppBase receives a message
LRESULT CALLBACK MessageProc(HWND wnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
		case WM_KEYDOWN:
		{
			// Map this key to a InputKeys enum and update the
			// state of m_aKeys[] by adding the INPUT_KEY_STATE_FLAG_KEY_WAS_DOWN|INPUT_KEY_STATE_FLAG_KEY_IS_DOWN mask
			// only if the key is not down
			Prisma::keyboardCallback((UINT)wParam,Prisma::CallbackHandler::ACTION::KEY_PRESS);
			break;
		}

		case WM_KEYUP:
		{
			Prisma::keyboardCallback((UINT)wParam, Prisma::CallbackHandler::ACTION::KEY_RELEASE);
			break;
		}

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(wnd, &ps);
            EndPaint(wnd, &ps);
            return 0;
        }
        case WM_SIZE: // Window size has been changed
			//Prisma::framebufferSizeCallback(LOWORD(lParam), HIWORD(lParam));
			if (privatePrisma->m_pSwapChain) {
				privatePrisma->m_pSwapChain->Resize(LOWORD(lParam), HIWORD(lParam));
				Prisma::framebufferSizeCallback(LOWORD(lParam), HIWORD(lParam));
			}
            return 0;

        case WM_CHAR:
            if (wParam == VK_ESCAPE)
                PostQuitMessage(0);
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_GETMINMAXINFO:
        {
            LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;

            lpMMI->ptMinTrackSize.x = 320;
            lpMMI->ptMinTrackSize.y = 240;
            return 0;
        }

        default:
            return DefWindowProc(wnd, message, wParam, lParam);
    }
}

void Prisma::PrismaFunc::init(Prisma::WindowsHelper::WindowsData windowsData)
{
	Settings settings = SettingsLoader::getInstance().getSettings();
	privatePrisma = std::make_shared<PrivatePrisma>();


	// Register our window class
	WNDCLASSEX wcex = { sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW, MessageProc,
					   0L, 0L, *(HINSTANCE*)windowsData.hInstance, NULL, NULL, NULL, NULL, settings.name.c_str(), NULL};
	RegisterClassEx(&wcex);
	// Create a window
	LONG WindowWidth = settings.width;
	LONG WindowHeight = settings.height;
	RECT rc = { 0, 0, WindowWidth, WindowHeight };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	HWND hWnd = CreateWindow(settings.name.c_str(), settings.name.c_str(),
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, *(HINSTANCE*)windowsData.hInstance, NULL);
	if (!hWnd)
	{
		MessageBox(NULL, "Cannot create window", "Error", MB_OK | MB_ICONERROR);
	}
	ShowWindow(hWnd, windowsData.nShowCmd);
	UpdateWindow(hWnd);


	Diligent::SwapChainDesc SCDesc;

	switch (m_contextData.m_DeviceType)
	{
#if D3D11_SUPPORTED
	case Diligent::RENDER_DEVICE_TYPE_D3D11:
	{
		Diligent::EngineD3D11CreateInfo EngineCI;
#    if ENGINE_DLL
		// Load the dll and import GetEngineFactoryD3D11() function
		auto* GetEngineFactoryD3D11 = LoadGraphicsEngineD3D11();
#    endif
		auto* pFactoryD3D11 = GetEngineFactoryD3D11();
		pFactoryD3D11->CreateDeviceAndContextsD3D11(EngineCI, &m_contextData.m_pDevice, &m_contextData.m_pImmediateContext);
		Win32NativeWindow Window{ hWnd };
		pFactoryD3D11->CreateSwapChainD3D11(m_contextData.m_pDevice, m_contextData.m_pImmediateContext, SCDesc, FullScreenModeDesc{}, Window, &m_contextData.m_pSwapChain);
		m_contextData.m_pEngineFactory = pFactoryD3D11;
	}
	break;
#endif


#if D3D12_SUPPORTED
	case RENDER_DEVICE_TYPE_D3D12:
	{
#    if ENGINE_DLL
		// Load the dll and import GetEngineFactoryD3D12() function
		auto GetEngineFactoryD3D12 = LoadGraphicsEngineD3D12();
#    endif
		EngineD3D12CreateInfo EngineCI;

		auto* pFactoryD3D12 = GetEngineFactoryD3D12();
		pFactoryD3D12->CreateDeviceAndContextsD3D12(EngineCI, &m_contextData.m_pDevice, &m_contextData.m_pImmediateContext);
		Win32NativeWindow Window{ hWnd };
		pFactoryD3D12->CreateSwapChainD3D12(m_contextData.m_pDevice, m_contextData.m_pImmediateContext, SCDesc, FullScreenModeDesc{}, Window, &m_contextData.m_pSwapChain);
		m_contextData.m_pEngineFactory = pFactoryD3D12;

	}
	break;
#endif


#if GL_SUPPORTED
	case RENDER_DEVICE_TYPE_GL:
	{
#    if EXPLICITLY_LOAD_ENGINE_GL_DLL
		// Load the dll and import GetEngineFactoryOpenGL() function
		auto GetEngineFactoryOpenGL = LoadGraphicsEngineOpenGL();
#    endif
		auto* pFactoryOpenGL = GetEngineFactoryOpenGL();

		EngineGLCreateInfo EngineCI;
		EngineCI.Window.hWnd = hWnd;

		pFactoryOpenGL->CreateDeviceAndSwapChainGL(EngineCI, &m_contextData.m_pDevice, &m_contextData.m_pImmediateContext, SCDesc, &m_contextData.m_pSwapChain);
		m_contextData.m_pEngineFactory = pFactoryOpenGL;

	}
	break;
#endif


#if VULKAN_SUPPORTED
	case RENDER_DEVICE_TYPE_VULKAN:
	{
#    if EXPLICITLY_LOAD_ENGINE_VK_DLL
		// Load the dll and import GetEngineFactoryVk() function
		auto GetEngineFactoryVk = LoadGraphicsEngineVk();
#    endif
		EngineVkCreateInfo EngineCI;

		auto* pFactoryVk = GetEngineFactoryVk();
		pFactoryVk->CreateDeviceAndContextsVk(EngineCI, &m_contextData.m_pDevice, &m_contextData.m_pImmediateContext);

		if (!m_contextData.m_pSwapChain && hWnd != nullptr)
		{
			Win32NativeWindow Window{ hWnd };
			pFactoryVk->CreateSwapChainVk(m_contextData.m_pDevice, m_contextData.m_pImmediateContext, SCDesc, Window, &m_contextData.m_pSwapChain);
		}
		m_contextData.m_pEngineFactory = pFactoryVk;
	}
	break;
#endif

	privatePrisma->m_pSwapChain = m_contextData.m_pSwapChain;
	default:
		std::cerr << "Unknown/unsupported device type";
		break;
	}

}

bool Prisma::PrismaFunc::update()
{
	bool update = !PeekMessage(&privatePrisma->msg, NULL, 0, 0, PM_REMOVE);
	//glfwPollEvents();
	if (!update)
	{
		TranslateMessage(&privatePrisma->msg);
		DispatchMessage(&privatePrisma->msg);
	}
	return update;
}

void Prisma::PrismaFunc::clear()
{
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Prisma::PrismaFunc::setCallback(std::shared_ptr<CallbackHandler> callbackHandler)
{
	privatePrisma->callback = callbackHandler;
	if (!privatePrisma->initCallback)
	{
		/*glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback);
		glfwSetCursorPosCallback(m_window, mouseCallback);
		glfwSetMouseButtonCallback(m_window, mouseButtonCallback);
		glfwSetInputMode(m_window, GLFW_STICKY_KEYS, GLFW_TRUE);
		glfwSetKeyCallback(m_window, keyboardCallback);
		glfwSetScrollCallback(m_window, rollCallback);*/
		privatePrisma->initCallback = true;
	}
}

void Prisma::PrismaFunc::closeWindow()
{
	//glfwSetWindowShouldClose(m_window, true);
	int msgOut = (int)privatePrisma->msg.wParam;
}

bool Prisma::PrismaFunc::shouldClose()
{
	//return glfwWindowShouldClose(m_window);
	return WM_QUIT == privatePrisma->msg.message;
}

void Prisma::PrismaFunc::hiddenMouse(bool hidden)
{
	//glfwSetInputMode(m_window, GLFW_CURSOR, hidden ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
}

void Prisma::PrismaFunc::msaa(bool isMsaa, int samples)
{
	if (isMsaa)
	{
		//glfwWindowHint(GLFW_SAMPLES, samples);
		//glEnable(GL_MULTISAMPLE);
	}
	else
	{
		//glDisable(GL_MULTISAMPLE);
	}
}

GLFWwindow* Prisma::PrismaFunc::window()
{
	return m_window;
}

void Prisma::PrismaFunc::destroy()
{
	//glfwDestroyWindow(m_window);
	//glfwTerminate();
}
