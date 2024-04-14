#pragma once

#include "GL/glew.h"
#include "GLFW/glfw3.h"

namespace Prisma {
    class SSBO {
    public:
        SSBO(unsigned int ssbo);

        void resize(unsigned int size,unsigned int type=GL_DYNAMIC_COPY);

        void modifyData(unsigned int offset, unsigned int size, void* data);

        void bind();

        void unbind();

    private:
        unsigned int m_ssbo;
        unsigned int m_id;
    };
}
