#pragma once
#include <vector>

#include "../GlobalData/Platform.h"
#include "GlobalData/Platform.h"
#include "Graphics/GraphicsEngine/interface/Buffer.h"
#include "Graphics/GraphicsEngine/interface/Texture.h"
#include "Graphics/GraphicsEngine/interface/PipelineState.h"
#include <Common/interface/RefCntAutoPtr.hpp>

#include "Helpers/Blit.h"
#include <glm/glm.hpp>

#include "PipelineBlitRT.h"

namespace Prisma {
class PipelineSoftwareRT {
public:
    struct Sizes {
        unsigned int vertexBase;
        unsigned int indexBase;
        unsigned int vertexSize;
        unsigned int indexSize;
    };

    struct Vertex {
        glm::vec4 vertex;
        glm::vec4 uv;
    };

    PipelineSoftwareRT(unsigned int width, unsigned int height);
    void render();

private:
    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pso;
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srb;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_rtVertices;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_rtIndices;
    //Diligent::RefCntAutoPtr<Diligent::IBuffer> m_rtBvhVertices;
    //Diligent::RefCntAutoPtr<Diligent::IBuffer> m_rtBvhNodes;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_size;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_totalMeshes;
    Diligent::RefCntAutoPtr<Diligent::ITexture> m_texture;
    Diligent::RefCntAutoPtr<Diligent::IPipelineResourceSignature> m_pResourceSignature;
    std::unique_ptr<PipelineBlitRT> m_blitRT;
    unsigned int m_width;
    unsigned int m_height;
};
}