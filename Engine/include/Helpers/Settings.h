#pragma once

#include <functional>
#include <string>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace Prisma {

	struct Settings {
		std::string name = "";
		unsigned int width=1920;
		unsigned int height=1080;
		bool fullscreen = false;
		float angle = 90.0f;
		float nearPlane = 0.1f;
		float farPlane = 0.1f;
	};

	struct CallbackHandler {
		std::function<void(int width,int height)> resize;
		std::function<void(int x, int y)> mouse;
        std::function<void(int button,int action,double x, double y)> mouseClick;
    };
}