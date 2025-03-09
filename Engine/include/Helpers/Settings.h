#pragma once

#include <functional>
#include <string>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace Prisma
{
	struct Settings
	{
		std::string name = "";
		unsigned int width = 1920;
		unsigned int height = 1080;
		bool fullscreen = false;
	};

	struct CallbackHandler
	{
		enum ACTION
		{
			KEY_PRESS,
			KEY_RELEASE
		};
		std::function<void(int width, int height)> resize;
		std::function<void(int x, int y)> mouse;
		std::function<void(int button, ACTION action, double x, double y)> mouseClick;
		std::function<void(int key, ACTION action)> keyboard;
		std::function<void(double xOffset, double yOffset)> rollMouse;
	};
}
