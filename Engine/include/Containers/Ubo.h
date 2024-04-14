#pragma once

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "glm/gtc/type_ptr.hpp"

namespace Prisma {
	class Ubo {
	public:
		Ubo(unsigned int size,int ubo);

        void modifyData(unsigned int offset,unsigned int size,void* data);

    private:
        unsigned int m_ubo;
	};
}