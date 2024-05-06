#pragma once

#include "imgui.h"
#include <vector>
#include <memory>
#include "glm/glm.hpp"

namespace Prisma {
    class TextureInfo {
    public:
        TextureInfo();

        void showTextures();

        void add(std::pair<unsigned int, glm::vec2> id);

        TextureInfo(const TextureInfo&) = delete;
        TextureInfo& operator=(const TextureInfo&) = delete;
        static TextureInfo& getInstance();

    private:
        std::vector<std::pair<unsigned int,glm::vec2>> m_textures;
        static std::shared_ptr<TextureInfo> instance;
        bool m_position = false;
    };
}

