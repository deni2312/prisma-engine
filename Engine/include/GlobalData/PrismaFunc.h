#pragma once
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include <string>
#include "glm/glm.hpp"
#include <memory>

#include "../Helpers/Settings.h"

namespace Prisma {

	class CallbackHandler;

	class PrismaFunc {
	public:
		static PrismaFunc& getInstance();

		PrismaFunc(const PrismaFunc&) = delete;
		PrismaFunc& operator=(const PrismaFunc&) = delete;

		void swapBuffers();
		void clear();
		void setCallback(std::shared_ptr<CallbackHandler> callbackHandler);
		void closeWindow();
		bool shouldClose();
		void hiddenMouse(bool hidden);
		GLFWwindow* window();
		void destroy();

        PrismaFunc();

    private:

        GLFWwindow* m_window;
		static std::shared_ptr<PrismaFunc> instance;
	};
}
