#include "GlobalData/PrismaFunc.h"
#include "GlobalData/GlobalData.h"
#include "GlobalData/Defines.h"
#include "Helpers/Settings.h"
#include "Helpers/SettingsLoader.h"
#include "Helpers/Logger.h"

#include <iostream>

#include <fstream>


#include "Graphics/GraphicsEngineD3D11/interface/EngineFactoryD3D11.h"
#include "Graphics/GraphicsEngineD3D12/interface/EngineFactoryD3D12.h"
#include "Graphics/GraphicsEngineOpenGL/interface/EngineFactoryOpenGL.h"
#include "Graphics/GraphicsEngineVulkan/interface/EngineFactoryVk.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#define GLFW_NATIVE_INCLUDE_NONE
#include <GLFW/glfw3native.h>

#include "glm/gtc/type_ptr.inl"

using namespace Diligent;


struct PrivatePrisma {
        std::shared_ptr<Prisma::CallbackHandler> callback;
        bool initCallback = false;
        std::map<std::string, std::string> errorMap;
        Prisma::PrismaFunc::UIInput inputUi;
};

std::shared_ptr<PrivatePrisma> privatePrisma;

namespace Prisma {
void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
        if (privatePrisma->callback->resize) {
                privatePrisma->callback->resize(width, height);
        }
}

void mouseCallback(GLFWwindow* window, double x, double y) {
        if (privatePrisma->callback->mouse) {
                privatePrisma->callback->mouse(x, y);
        }
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        if (privatePrisma->inputUi.mouseClick) {
                privatePrisma->inputUi.mouseClick(button, action);
        }
        if (privatePrisma->callback->mouseClick) {
                privatePrisma->callback->mouseClick(button, action, xpos, ypos);
        }
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        if (privatePrisma->inputUi.keyboard) {
                privatePrisma->inputUi.keyboard(key, action, scancode);
        }
        if (privatePrisma->callback->keyboard) {
                privatePrisma->callback->keyboard(key, scancode, action, mods);
        }
}

void rollCallback(GLFWwindow* window, double x, double y) {
        if (privatePrisma->inputUi.mouseRoll) {
                privatePrisma->inputUi.mouseRoll(x, y);
        }

        if (privatePrisma->callback->rollMouse) {
                privatePrisma->callback->rollMouse(x, y);
        }
}
}


Prisma::PrismaFunc::PrismaFunc() {

}

Prisma::PrismaFunc::RenderTargetFormat Prisma::PrismaFunc::renderFormat() const {
        return m_renderFormat;
}

Prisma::PrismaFunc::ContextData& Prisma::PrismaFunc::contextData() {
        return m_contextData;
}

void Prisma::PrismaFunc::inputUI(UIInput inputUi) {
        privatePrisma->inputUi = inputUi;
}

void Prisma::PrismaFunc::init() {
        Settings settings = SettingsLoader::getInstance().getSettings();
        privatePrisma = std::make_shared<PrivatePrisma>();

        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
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

        m_window = glfwCreateWindow(settings.width, settings.height, settings.name.c_str(), nullptr, nullptr);
        if (m_window == nullptr) {
                LOG_ERROR_MESSAGE("Failed to create GLFW window");
        }
        glfwSetWindowPos(m_window, x, y);
        glfwSetWindowAttrib(m_window, GLFW_RESIZABLE, GLFW_FALSE);

        glfwSetWindowUserPointer(m_window, this);

#if PLATFORM_WIN32
        Win32NativeWindow Window{glfwGetWin32Window(m_window)};
#endif
#if PLATFORM_LINUX
	LinuxNativeWindow Window;
	Window.WindowId = glfwGetX11Window(m_Window);
	Window.pDisplay = glfwGetX11Display();
	if (DevType == RENDER_DEVICE_TYPE_GL)
		glfwMakeContextCurrent(m_Window);
#endif
#if PLATFORM_MACOS
	MacOSNativeWindow Window;
	if (DevType == RENDER_DEVICE_TYPE_GL)
		glfwMakeContextCurrent(m_Window);
	else
		Window.pNSView = GetNSWindowView(m_Window);
#endif

        SwapChainDesc SCDesc;

        switch (m_contextData.m_DeviceType) {
#if D3D11_SUPPORTED
                case RENDER_DEVICE_TYPE_D3D11: {
                        EngineD3D11CreateInfo EngineCI;
#    if ENGINE_DLL
                        // Load the dll and import GetEngineFactoryD3D11() function
                        auto* GetEngineFactoryD3D11 = LoadGraphicsEngineD3D11();
#    endif
                        auto* pFactoryD3D11 = GetEngineFactoryD3D11();
                        pFactoryD3D11->CreateDeviceAndContextsD3D11(EngineCI, &m_contextData.m_pDevice,
                                                                    &m_contextData.m_pImmediateContext);
                        pFactoryD3D11->CreateSwapChainD3D11(m_contextData.m_pDevice, m_contextData.m_pImmediateContext,
                                                            SCDesc, FullScreenModeDesc{}, Window,
                                                            &m_contextData.m_pSwapChain);
                        m_contextData.m_pEngineFactory = pFactoryD3D11;
                }
                break;
#endif

#if D3D12_SUPPORTED
                case RENDER_DEVICE_TYPE_D3D12: {
#    if ENGINE_DLL
                        // Load the dll and import GetEngineFactoryD3D12() function
                        auto GetEngineFactoryD3D12 = LoadGraphicsEngineD3D12();
#    endif
                        EngineD3D12CreateInfo EngineCI;

                        auto* pFactoryD3D12 = GetEngineFactoryD3D12();
                        pFactoryD3D12->CreateDeviceAndContextsD3D12(EngineCI, &m_contextData.m_pDevice,
                                                                    &m_contextData.m_pImmediateContext);
                        pFactoryD3D12->CreateSwapChainD3D12(m_contextData.m_pDevice, m_contextData.m_pImmediateContext,
                                                            SCDesc, FullScreenModeDesc{}, Window,
                                                            &m_contextData.m_pSwapChain);
                        m_contextData.m_pEngineFactory = pFactoryD3D12;
                }
                break;
#endif

#if GL_SUPPORTED
                case RENDER_DEVICE_TYPE_GL: {
#    if EXPLICITLY_LOAD_ENGINE_GL_DLL
                        // Load the dll and import GetEngineFactoryOpenGL() function
                        auto GetEngineFactoryOpenGL = LoadGraphicsEngineOpenGL();
#    endif
                        auto* pFactoryOpenGL = GetEngineFactoryOpenGL();

                        EngineGLCreateInfo EngineCI;
                        EngineCI.Window = Window;

                        pFactoryOpenGL->CreateDeviceAndSwapChainGL(EngineCI, &m_contextData.m_pDevice,
                                                                   &m_contextData.m_pImmediateContext, SCDesc,
                                                                   &m_contextData.m_pSwapChain);
                        m_contextData.m_pEngineFactory = pFactoryOpenGL;
                }
                break;
#endif

#if VULKAN_SUPPORTED
                case RENDER_DEVICE_TYPE_VULKAN: {
#    if EXPLICITLY_LOAD_ENGINE_VK_DLL
                        // Load the dll and import GetEngineFactoryVk() function
                        auto* GetEngineFactoryVk = LoadGraphicsEngineVk();
#    endif
                        auto* pFactoryVk = GetEngineFactoryVk();

                        EngineVkCreateInfo EngineCI;
                        EngineCI.Features.OcclusionQueries = DEVICE_FEATURE_STATE_OPTIONAL;
                        EngineCI.Features.BinaryOcclusionQueries = DEVICE_FEATURE_STATE_OPTIONAL;
                        EngineCI.Features.TimestampQueries = DEVICE_FEATURE_STATE_OPTIONAL;
                        EngineCI.Features.PipelineStatisticsQueries = DEVICE_FEATURE_STATE_OPTIONAL;
                        EngineCI.Features.DurationQueries = DEVICE_FEATURE_STATE_OPTIONAL;
                        EngineCI.Features.ShaderResourceRuntimeArrays = DEVICE_FEATURE_STATE_ENABLED;
                        EngineCI.Features.GeometryShaders = DEVICE_FEATURE_STATE_ENABLED;
                        EngineCI.Features.DepthClamp = DEVICE_FEATURE_STATE_ENABLED;
                        EngineCI.Features.RayTracing = DEVICE_FEATURE_STATE_ENABLED;
                        EngineCI.Features.PixelUAVWritesAndAtomics = DEVICE_FEATURE_STATE_ENABLED;
                        EngineCI.Features.NativeMultiDraw = DEVICE_FEATURE_STATE_ENABLED;

                        pFactoryVk->CreateDeviceAndContextsVk(EngineCI, &m_contextData.m_pDevice,
                                                              &m_contextData.m_pImmediateContext);
                        pFactoryVk->CreateSwapChainVk(m_contextData.m_pDevice, m_contextData.m_pImmediateContext,
                                                      SCDesc, Window, &m_contextData.m_pSwapChain);
                        m_contextData.m_pEngineFactory = pFactoryVk;
                }
                break;
#endif

                default:
                        std::cerr << "Unknown/unsupported device type";
                        break;
        }

        GlobalData::getInstance().defaultBlack().loadTexture({DIR_DEFAULT_BLACK, true});
        GlobalData::getInstance().defaultWhite().loadTexture({DIR_DEFAULT_WHITE, false});
        GlobalData::getInstance().defaultNormal().loadTexture({DIR_DEFAULT_NORMAL, false});

        RefCntAutoPtr<ITexture> pDummyTexture;

        TextureDesc TexDesc;
        TexDesc.Name = "Dummy Texture";
        TexDesc.Type = RESOURCE_DIM_TEX_2D;
        TexDesc.Width = 1;
        TexDesc.Height = 1;
        TexDesc.Format = TEX_FORMAT_RGBA8_UNORM;
        TexDesc.Usage = USAGE_DEFAULT;
        TexDesc.BindFlags = BIND_SHADER_RESOURCE;

        m_contextData.m_pDevice->CreateTexture(TexDesc, nullptr, &pDummyTexture);

        RefCntAutoPtr<ITexture> pDummyTextureArray;

        TextureDesc TexDescArray;
        TexDescArray.Name = "Dummy Texture";
        TexDescArray.Type = RESOURCE_DIM_TEX_2D_ARRAY;
        TexDescArray.Width = 1;
        TexDescArray.Height = 1;
        TexDescArray.Format = TEX_FORMAT_RGBA8_UNORM;
        TexDescArray.Usage = USAGE_DEFAULT;
        TexDescArray.BindFlags = BIND_SHADER_RESOURCE;

        TexDescArray.ArraySize = 5;

        m_contextData.m_pDevice->CreateTexture(TexDescArray, nullptr, &pDummyTextureArray);

        GlobalData::getInstance().dummyTexture(pDummyTexture);

        GlobalData::getInstance().dummyTextureArray(pDummyTextureArray);
}

void Prisma::PrismaFunc::poll() {
        glfwPollEvents();
}

void Prisma::PrismaFunc::update() {
        m_contextData.m_pImmediateContext->Flush();
        m_contextData.m_pSwapChain->Present();
}

void Prisma::PrismaFunc::bindMainRenderTarget() {
        auto pRTV = m_contextData.m_pSwapChain->GetCurrentBackBufferRTV();
        auto pDSV = m_contextData.m_pSwapChain->GetDepthBufferDSV();
        // Clear the back buffer
        m_contextData.m_pImmediateContext->SetRenderTargets(1, &pRTV, pDSV, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
}

void Prisma::PrismaFunc::clear() {
        auto pRTV = m_contextData.m_pSwapChain->GetCurrentBackBufferRTV();
        auto pDSV = m_contextData.m_pSwapChain->GetDepthBufferDSV();
        m_contextData.m_pImmediateContext->ClearRenderTarget(pRTV, value_ptr(Define::CLEAR_COLOR),
                                                             RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        m_contextData.m_pImmediateContext->ClearDepthStencil(pDSV, CLEAR_DEPTH_FLAG, 1.f, 0,
                                                             RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
}

void Prisma::PrismaFunc::setCallback(std::shared_ptr<CallbackHandler> callbackHandler) {
        privatePrisma->callback = callbackHandler;
        if (!privatePrisma->initCallback) {
                glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback);
                glfwSetCursorPosCallback(m_window, mouseCallback);
                glfwSetMouseButtonCallback(m_window, mouseButtonCallback);
                glfwSetKeyCallback(m_window, keyboardCallback);
                glfwSetScrollCallback(m_window, rollCallback);
                privatePrisma->initCallback = true;
        }
}

void Prisma::PrismaFunc::closeWindow() {
        glfwSetWindowShouldClose(m_window, true);
}

bool Prisma::PrismaFunc::shouldClose() {
        return glfwWindowShouldClose(m_window);
}

void Prisma::PrismaFunc::hiddenMouse(bool hidden) {
        //glfwSetInputMode(m_window, GLFW_CURSOR, hidden ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
}

void Prisma::PrismaFunc::msaa(bool isMsaa, int samples) {
        if (isMsaa) {
                //glfwWindowHint(GLFW_SAMPLES, samples);
                //glEnable(GL_MULTISAMPLE);
        } else {
                //glDisable(GL_MULTISAMPLE);
        }
}

GLFWwindow* Prisma::PrismaFunc::window() {
        return m_window;
}

void* Prisma::PrismaFunc::windowNative() {
        return glfwGetWin32Window(m_window);
}

void Prisma::PrismaFunc::destroy() {
        if (m_contextData.m_pImmediateContext)
                m_contextData.m_pImmediateContext->Flush();

        m_contextData.m_pSwapChain = nullptr;
        m_contextData.m_pImmediateContext = nullptr;
        m_contextData.m_pDevice = nullptr;

        if (m_window) {
                glfwDestroyWindow(m_window);
                glfwTerminate();
        }
        //glfwDestroyWindow(m_window);
        //glfwTerminate();
}