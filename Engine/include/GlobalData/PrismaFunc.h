#pragma once
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include <string>
#include "glm/glm.hpp"
#include <memory>

#include "../Helpers/Settings.h"
#include "../GlobalData/InstanceData.h"
#include "../Helpers/WindowsHelper.h"

#ifndef PLATFORM_WIN32
#    define PLATFORM_WIN32 1
#endif

#ifndef ENGINE_DLL
#    define ENGINE_DLL 1
#endif

#ifndef D3D11_SUPPORTED
#    define D3D11_SUPPORTED 1
#endif

#ifndef D3D12_SUPPORTED
#    define D3D12_SUPPORTED 1
#endif

#ifndef GL_SUPPORTED
#    define GL_SUPPORTED 1
#endif

#ifndef VULKAN_SUPPORTED
#    define VULKAN_SUPPORTED 1
#endif

#include "Graphics/GraphicsEngine/interface/RenderDevice.h"
#include "Graphics/GraphicsEngine/interface/DeviceContext.h"
#include "Graphics/GraphicsEngine/interface/SwapChain.h"

#include "Common/interface/RefCntAutoPtr.hpp"

namespace Prisma
{
	struct CallbackHandler;

	class PrismaFunc : public InstanceData<PrismaFunc>
	{
	public:
		struct ContextData
		{
			Diligent::RefCntAutoPtr<Diligent::IRenderDevice>  m_pDevice;
			Diligent::RefCntAutoPtr<Diligent::IDeviceContext> m_pImmediateContext;
			Diligent::RefCntAutoPtr<Diligent::ISwapChain>     m_pSwapChain;
			Diligent::RENDER_DEVICE_TYPE m_DeviceType = Diligent::RENDER_DEVICE_TYPE_VULKAN;
			Diligent::RefCntAutoPtr<Diligent::IEngineFactory> m_pEngineFactory;
		};

		void init(Prisma::WindowsHelper::WindowsData windowsData);
		bool update();
		void clear();
		void setCallback(std::shared_ptr<CallbackHandler> callbackHandler);
		void closeWindow();
		bool shouldClose();
		void hiddenMouse(bool hidden);
		void msaa(bool isMsaa, int samples);
		GLFWwindow* window();

		void destroy();

		PrismaFunc();

		ContextData& contextData();

	private:
		GLFWwindow* m_window;
		Prisma::WindowsHelper::WindowsData m_windowsData;
		ContextData m_contextData;
	};
}
