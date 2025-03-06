#pragma once
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include <string>
#include "glm/glm.hpp"
#include <memory>

#include "../Helpers/Settings.h"
#include "../GlobalData/InstanceData.h"

#include <webgpu/webgpu.h>

#ifdef WEBGPU_BACKEND_WGPU
#  include <webgpu/wgpu.h>
#endif // WEBGPU_BACKEND_WGPU

#include <GLFW/glfw3.h>
#include "../Helpers/glfw3webgpu.h"

#ifdef __EMSCRIPTEN__
#  include <emscripten.h>
#endif // __EMSCRIPTEN__

namespace Prisma
{
	struct CallbackHandler;

	class PrismaFunc : public InstanceData<PrismaFunc>
	{
	public:
		void swapBuffers();
		void clear();
		void setCallback(std::shared_ptr<CallbackHandler> callbackHandler);
		void closeWindow();
		bool shouldClose();
		void hiddenMouse(bool hidden);
		void msaa(bool isMsaa, int samples);
		GLFWwindow* window();

		// Getter for m_device
		WGPUDevice device() const;

		// Getter for m_queue
		WGPUQueue queue() const;

		// Getter for m_surface
		WGPUSurface surface() const;

		void destroy();

		PrismaFunc();

		WGPUTextureView NextSurfaceTextureView();

	private:
		GLFWwindow* m_window;
		WGPUDevice m_device;
		WGPUQueue m_queue;
		WGPUSurface m_surface;
	};
}
