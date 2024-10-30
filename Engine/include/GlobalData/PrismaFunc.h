#pragma once
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include <string>
#include "glm/glm.hpp"
#include <memory>

#include "../Helpers/Settings.h"
#include "../GlobalData/InstanceData.h"

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
		void destroy();

		PrismaFunc();

	private:
		GLFWwindow* m_window;
	};
}
