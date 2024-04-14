#pragma once

#include "GL/glew.h"

namespace Prisma {

    class EBO {
    public:
        EBO();

        void bind();

        void writeData(unsigned int size, void *offset, unsigned int type = GL_STATIC_DRAW);

        unsigned int id() const;

    private:
        unsigned int m_id;
    };

}

