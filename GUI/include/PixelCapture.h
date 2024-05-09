#pragma once

#include "imgui.h"
#include <vector>
#include <memory>
#include "glm/glm.hpp"
#include <string>
#include "../../Engine/include/Containers/FBO.h"
#include "../../Engine/include/Containers/SSBO.h"
#include "../../Engine/include/SceneObjects/Mesh.h"

namespace Prisma {
    class PixelCapture {
    public:
        PixelCapture();
        std::shared_ptr<Prisma::Mesh> capture(glm::vec2 position);
        PixelCapture(const PixelCapture&) = delete;
        PixelCapture& operator=(const PixelCapture&) = delete;
        static PixelCapture& getInstance();

    private:
        std::shared_ptr<Prisma::FBO> m_fbo;

        std::shared_ptr<Prisma::Shader> m_shader;

        static std::shared_ptr<PixelCapture> instance;
    };
}

