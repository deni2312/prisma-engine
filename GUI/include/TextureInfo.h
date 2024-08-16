#pragma once

#include "imgui.h"
#include <vector>
#include <memory>
#include "glm/glm.hpp"
#include <string>

namespace Prisma {
    class TextureInfo {
    public:
        TextureInfo();

        void showTextures();

        void add(std::pair<unsigned int, std::string> id);

        TextureInfo(const TextureInfo&) = delete;
        TextureInfo& operator=(const TextureInfo&) = delete;
        static TextureInfo& getInstance();

    private:
        std::vector<std::pair<unsigned int,std::string>> m_textures;
        static std::shared_ptr<TextureInfo> instance;
    };
}

