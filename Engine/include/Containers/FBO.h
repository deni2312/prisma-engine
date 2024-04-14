#pragma once

#include "GL/glew.h"
#include <memory>
#include "../Helpers/Shader.h"

namespace Prisma {
    class FBO {
    public:
        struct FBOData {
            int width = 1920;
            int height = 1080;
            unsigned int internalFormat = GL_RGBA;
            unsigned int internalType = GL_UNSIGNED_INT;
            bool enableDepth = false;
            bool enableSrgb = false;
        };
        FBO(FBOData fboData);
        ~FBO();

        void bind();
        void unbind();

        uint64_t texture() const;

        unsigned int frameBufferID();

    private:
        unsigned int m_framebufferID;
        uint64_t m_id;
        std::shared_ptr<Shader> m_shader;
        unsigned int m_vao;
        FBOData m_fboData;
    };
}
