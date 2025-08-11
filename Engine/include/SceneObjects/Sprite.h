#pragma once
#include <vector>
#include <memory>
#include "Node.h"
#include "GlobalData/Platform.h"
#include "Containers/Texture.h"
#include "Common/interface/RefCntAutoPtr.hpp"
#include "Graphics/GraphicsEngine/interface/RenderDevice.h"

namespace Diligent {
struct IPipelineResourceSignature;
struct IPipelineState;
}

namespace Prisma {
class Sprite : public Node {
public:
    Sprite();
    void loadSprites(std::vector<std::shared_ptr<Texture>> textures);
    void numSprites(unsigned int numSprites);
    void size(glm::vec2 size);
    void render();
    Diligent::RefCntAutoPtr<Diligent::IBuffer> models();
    Diligent::RefCntAutoPtr<Diligent::IBuffer> spriteIds();

private:
    std::vector<Diligent::IDeviceObject*> m_sprites;

    struct ModelSizes {
        glm::mat4 model;
        glm::vec2 size;
        glm::vec2 padding;
    };

    struct SpriteData {
        glm::mat4 model=glm::mat4(1);
        glm::vec4 color=glm::vec4(1);
    };

    struct SpriteIds {
        int id;
        int maxSprites;
        glm::ivec2 padding;
    };

    unsigned int m_numSprites = 0;
    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pso;
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srb;
    Diligent::RefCntAutoPtr<Diligent::IPipelineResourceSignature> m_pResourceSignature;

    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_models;

    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_spriteIds;

    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_modelSizes;

    glm::vec2 m_size = glm::vec2(1);
};
}