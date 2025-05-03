#pragma once
#include <vector>
#include <memory>
#include "glm/glm.hpp"
#include <string>
#include "GlobalData/InstanceData.h"
#include "Containers/Texture.h"

namespace Prisma::GUI {
class TextureInfo : public InstanceData<TextureInfo> {
public:
    TextureInfo();

    void showTextures();

    bool textureTab();

private:
    int m_index;

    bool m_textureTab = false;

    float m_scale = 1;
};
}