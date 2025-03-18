#pragma once
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include <string>
#include "glm/glm.hpp"
#include <memory>
#include "../GlobalData/Platform.h"


#include "../Helpers/Settings.h"
#include "../GlobalData/InstanceData.h"
#include "../Helpers/WindowsHelper.h"

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
			Diligent::RENDER_DEVICE_TYPE m_DeviceType = Diligent::RENDER_DEVICE_TYPE_D3D12;
			Diligent::RefCntAutoPtr<Diligent::IEngineFactory> m_pEngineFactory;
		};

		struct UIInput
		{
			std::function<void(int, int)> mouseClick;
			std::function<void(int, int, int)> keyboard;
			std::function<void(int, int)> mouseRoll;
		};

		struct RenderTargetFormat
		{
			const Diligent::TEXTURE_FORMAT RenderFormat = Diligent::TEX_FORMAT_RGBA8_UNORM;
			const Diligent::TEXTURE_FORMAT DepthBufferFormat = Diligent::TEX_FORMAT_D32_FLOAT;
		};

		void init();
		void poll();
		void update();
		void bindMainRenderTarget();
		void clear();
		void setCallback(std::shared_ptr<CallbackHandler> callbackHandler);
		void closeWindow();
		bool shouldClose();
		void hiddenMouse(bool hidden);
		void msaa(bool isMsaa, int samples);
		GLFWwindow* window();

		void* windowNative();

		void destroy();

		PrismaFunc();

		ContextData& contextData();

		void inputUI(UIInput inputUi);

		RenderTargetFormat renderFormat() const;
	private:
		GLFWwindow* m_window;
		Prisma::WindowsHelper::WindowsData m_windowsData;
		ContextData m_contextData;
		RenderTargetFormat m_renderFormat;
	};
}
