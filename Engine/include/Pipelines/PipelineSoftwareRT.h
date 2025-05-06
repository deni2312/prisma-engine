#pragma once
#include "../GlobalData/Platform.h"
#include "GlobalData/Platform.h"
#include "Graphics/GraphicsEngine/interface/Buffer.h"
#include "Graphics/GraphicsEngine/interface/Texture.h"
#include "Graphics/GraphicsEngine/interface/PipelineState.h"
#include <Common/interface/RefCntAutoPtr.hpp>

#include "Helpers/Blit.h"
#include <glm/glm.hpp>

namespace Prisma {
class PipelineSoftwareRT {
public:
    PipelineSoftwareRT(unsigned int width, unsigned int height);
    void render();

private:
    struct Vertex {
        glm::vec4 vertex;
    };

    struct Sizes {
        unsigned int vertexSize;
        unsigned int indexSize;
        glm::ivec2 padding;
    };

    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pso;
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srb;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_rtVertices;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_rtIndices;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_size;
    Diligent::RefCntAutoPtr<Diligent::ITexture> m_texture;
    std::unique_ptr<Blit> m_blit;
    unsigned int m_width;
    unsigned int m_height;
};
}