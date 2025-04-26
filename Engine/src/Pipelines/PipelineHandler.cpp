#include "Pipelines/PipelineHandler.h"
#include "Helpers/SettingsLoader.h"

void Prisma::PipelineHandler::initScene() {
}

std::shared_ptr<Prisma::PipelineForward> Prisma::PipelineHandler::forward() {
        if (!m_forwardPipeline) {
                auto settings = SettingsLoader::getInstance().getSettings();
                m_forwardPipeline = std::make_shared<PipelineForward>(settings.width, settings.height);
        }
        return m_forwardPipeline;
}

std::shared_ptr<Prisma::PipelineDeferred> Prisma::PipelineHandler::deferred() {
        return m_deferredPipeline;
}

std::shared_ptr<Prisma::PipelineDeferredForward> Prisma::PipelineHandler::deferredForward() {
        return m_deferredForwardPipeline;
}

std::shared_ptr<Prisma::PipelineRayTracing> Prisma::PipelineHandler::raytracing() {
        if (!m_raytracingPipeline) {
                auto settings = SettingsLoader::getInstance().getSettings();
                m_raytracingPipeline = std::make_shared<PipelineRayTracing>(settings.width, settings.height);
        }
        return m_raytracingPipeline;
}

Prisma::PipelineHandler::PipelineHandler() {
        auto& contextData = PrismaFunc::getInstance().contextData();

        Diligent::TextureDesc RTColorDesc;
        RTColorDesc.Name = "Offscreen render target";
        RTColorDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
        RTColorDesc.Width = contextData.m_pSwapChain->GetDesc().Width;
        RTColorDesc.Height = contextData.m_pSwapChain->GetDesc().Height;
        RTColorDesc.MipLevels = 1;
        RTColorDesc.Format = PrismaFunc::getInstance().renderFormat().RenderFormat;
        // The render target can be bound as a shader resource and as a render target
        RTColorDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE | Diligent::BIND_RENDER_TARGET;
        // Define optimal clear value
        RTColorDesc.ClearValue.Format = RTColorDesc.Format;
        RTColorDesc.ClearValue.Color[0] = 0.350f;
        RTColorDesc.ClearValue.Color[1] = 0.350f;
        RTColorDesc.ClearValue.Color[2] = 0.350f;
        RTColorDesc.ClearValue.Color[3] = 1.f;
        contextData.m_pDevice->CreateTexture(RTColorDesc, nullptr, &m_textureData.pColorRTV);

        // Create window-size depth buffer
        Diligent::TextureDesc RTDepthDesc = RTColorDesc;
        RTDepthDesc.Name = "Offscreen depth buffer MAIN";
        RTDepthDesc.Format = PrismaFunc::getInstance().renderFormat().DepthBufferFormat;
        RTDepthDesc.BindFlags = Diligent::BIND_DEPTH_STENCIL;
        // Define optimal clear value
        RTDepthDesc.ClearValue.Format = RTDepthDesc.Format;
        RTDepthDesc.ClearValue.DepthStencil.Depth = 1;
        RTDepthDesc.ClearValue.DepthStencil.Stencil = 0;
        contextData.m_pDevice->CreateTexture(RTDepthDesc, nullptr, &m_textureData.pDepthDSV);
        // Store the depth-stencil view
}

Prisma::PipelineHandler::TextureData Prisma::PipelineHandler::textureData() {
        return m_textureData;
}