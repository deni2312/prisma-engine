#include "Pipelines/PipelineOmniShadow.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "SceneData/MeshIndirect.h"
#include "GlobalData/GlobalData.h"
#include "GlobalData/GlobalShaderNames.h"
#include "Helpers/SettingsLoader.h"
#include <glm/gtc/type_ptr.hpp>

#include "Handlers/OmniShadowHandler.h"


Prisma::PipelineOmniShadow::PipelineOmniShadow(unsigned int width, unsigned int height, bool post): m_width{width},
                                                                                                    m_height{height} {
    if (!post) {
        init();
    }
}

void Prisma::PipelineOmniShadow::update(glm::vec3 lightPos) {
    OmniShadowHandler::getInstance().render({m_depth, m_nearPlane, m_farPlane, m_width, m_height, lightPos});
}

Diligent::RefCntAutoPtr<Diligent::ITexture> Prisma::PipelineOmniShadow::shadowTexture() {
    return m_depth;
}

float Prisma::PipelineOmniShadow::farPlane() {
    return m_farPlane;
}

void Prisma::PipelineOmniShadow::farPlane(float farPlane) {
    m_farPlane = farPlane;
    CacheScene::getInstance().updateStatus(true);
}

void Prisma::PipelineOmniShadow::init() {
    if (!m_init) {
        auto& contextData = PrismaFunc::getInstance().contextData();

        // Create window-size depth buffer
        Diligent::TextureDesc RTDepthDesc;
        RTDepthDesc.Type = Diligent::RESOURCE_DIM_TEX_CUBE;  // Set cubemap type
        RTDepthDesc.Width = m_width;
        RTDepthDesc.Height = m_height;
        RTDepthDesc.MipLevels = 1;
        RTDepthDesc.ArraySize = 6;
        RTDepthDesc.Name = "Offscreen depth buffer OmniShadow";
        RTDepthDesc.Format = PrismaFunc::getInstance().renderFormat().DepthBufferFormat;
        RTDepthDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE | Diligent::BIND_DEPTH_STENCIL;
        // Define optimal clear value
        RTDepthDesc.ClearValue.Format = RTDepthDesc.Format;
        RTDepthDesc.ClearValue.DepthStencil.Depth = 1;
        RTDepthDesc.ClearValue.DepthStencil.Stencil = 0;
        contextData.device->CreateTexture(RTDepthDesc, nullptr, &m_depth);

        // Create render target views for each face
        for (int i = 0; i < 6; ++i) {
            Diligent::TextureViewDesc DepthDesc;
            DepthDesc.ViewType = Diligent::TEXTURE_VIEW_DEPTH_STENCIL;
            DepthDesc.TextureDim = Diligent::RESOURCE_DIM_TEX_2D;
            DepthDesc.MostDetailedMip = 0;
            DepthDesc.NumMipLevels = 1;
            DepthDesc.FirstArraySlice = i;  // Select the specific face
            DepthDesc.NumArraySlices = 1;
            Diligent::RefCntAutoPtr<Diligent::ITextureView> depth;
            m_depth->CreateView(DepthDesc, &depth);
        }
    }
    m_init = true;
}

float Prisma::PipelineOmniShadow::nearPlane() {
    return m_nearPlane;
}

void Prisma::PipelineOmniShadow::nearPlane(float nearPlane) {
    m_nearPlane = nearPlane;
    CacheScene::getInstance().updateStatus(true);
}