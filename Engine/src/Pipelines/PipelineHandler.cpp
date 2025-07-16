#include "Pipelines/PipelineHandler.h"
#include "Helpers/SettingsLoader.h"
#include "Helpers/ScenePipeline.h"
#include "engine.h"

void Prisma::PipelineHandler::initScene(bool init) {
    if (!init) {
        auto sceneHandler = std::make_shared<SceneHandler>();
        sceneHandler->onBeginRender = []() {};
        sceneHandler->onEndRender = []() {
            auto& contextData = PrismaFunc::getInstance().contextData();
            glm::mat4 model = glm::mat4(1);
            auto pRTV = contextData.swapChain->GetCurrentBackBufferRTV();
            auto pDSV = contextData.swapChain->GetDepthBufferDSV();
            Prisma::ScenePipeline::getInstance().render(model, pRTV, pDSV);
        };
        sceneHandler->onDestroy = []() {};
        sceneHandler->onLoading = [](auto node) {};
        Prisma::Engine::getInstance().setGuiData(sceneHandler);
        Prisma::Engine::getInstance().debug(false);
        auto camera = std::make_shared<Camera>();
        Prisma::Engine::getInstance().mainCamera(camera);
        std::cout << "aaaaaaaaaaaaaaaaaaaa" << std::endl;
    }
}

std::shared_ptr<Prisma::PipelineForward> Prisma::PipelineHandler::forward() {
    if (!m_forwardPipeline) {
        auto settings = SettingsLoader::getInstance().getSettings();
        m_forwardPipeline = std::make_shared<PipelineForward>(settings.width, settings.height);
    }
    return m_forwardPipeline;
}

std::shared_ptr<Prisma::PipelineDeferredForward> Prisma::PipelineHandler::deferredForward() {
    if (!m_deferredForwardPipeline) {
        auto settings = SettingsLoader::getInstance().getSettings();
        m_deferredForwardPipeline = std::make_shared<PipelineDeferredForward>(settings.width, settings.height);
    }

    return m_deferredForwardPipeline;
}

std::shared_ptr<Prisma::PipelineRayTracing> Prisma::PipelineHandler::raytracing() {
    if (!m_raytracingPipeline) {
        auto settings = SettingsLoader::getInstance().getSettings();
        m_raytracingPipeline = std::make_shared<PipelineRayTracing>(settings.width, settings.height);
    }
    return m_raytracingPipeline;
}

std::shared_ptr<Prisma::PipelineSoftwareRT> Prisma::PipelineHandler::softwareRt() {
    if (!m_softwarePipeline) {
        auto settings = SettingsLoader::getInstance().getSettings();
        m_softwarePipeline = std::make_shared<PipelineSoftwareRT>(settings.width, settings.height);
    }
    return m_softwarePipeline;
}

Prisma::PipelineHandler::PipelineHandler() {
    m_format = Diligent::TEX_FORMAT_RGBA16_FLOAT;
    auto& contextData = PrismaFunc::getInstance().contextData();

    Diligent::TextureDesc RTColorDesc;
    RTColorDesc.Name = "Offscreen render target";
    RTColorDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
    RTColorDesc.Width = contextData.swapChain->GetDesc().Width;
    RTColorDesc.Height = contextData.swapChain->GetDesc().Height;
    RTColorDesc.MipLevels = 1;
    RTColorDesc.Format = m_format;
    // The render target can be bound as a shader resource and as a render target
    RTColorDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE | Diligent::BIND_RENDER_TARGET;
    // Define optimal clear value
    RTColorDesc.ClearValue.Format = RTColorDesc.Format;
    RTColorDesc.ClearValue.Color[0] = 0.350f;
    RTColorDesc.ClearValue.Color[1] = 0.350f;
    RTColorDesc.ClearValue.Color[2] = 0.350f;
    RTColorDesc.ClearValue.Color[3] = 1.f;
    contextData.device->CreateTexture(RTColorDesc, nullptr, &m_textureData.pColorRTV);

    // Create window-size depth buffer
    Diligent::TextureDesc RTDepthDesc = RTColorDesc;
    RTDepthDesc.Name = "Offscreen depth buffer MAIN";
    RTDepthDesc.Format = PrismaFunc::getInstance().renderFormat().DepthBufferFormat;
    RTDepthDesc.BindFlags = Diligent::BIND_DEPTH_STENCIL | Diligent::BIND_SHADER_RESOURCE;
    // Define optimal clear value
    RTDepthDesc.ClearValue.Format = RTDepthDesc.Format;
    RTDepthDesc.ClearValue.DepthStencil.Depth = 1;
    RTDepthDesc.ClearValue.DepthStencil.Stencil = 0;
    contextData.device->CreateTexture(RTDepthDesc, nullptr, &m_textureData.pDepthDSV);
    // Store the depth-stencil view
}

void Prisma::PipelineHandler::render(Prisma::EngineSettings::Pipeline pipeline) {
    switch (pipeline) {
        case EngineSettings::Pipeline::FORWARD:
            PipelineHandler::getInstance().forward()->render();
            break;
        case EngineSettings::Pipeline::DEFERRED_FORWARD:
            PipelineHandler::getInstance().deferredForward()->render();
            break;

        case EngineSettings::Pipeline::RAYTRACING:
            PipelineHandler::getInstance().raytracing()->render();
            break;
        case EngineSettings::Pipeline::SOFTWARE_RAYTRACING:
            PipelineHandler::getInstance().softwareRt()->render();
            break;
    }
}

Diligent::TEXTURE_FORMAT Prisma::PipelineHandler::textureFormat() { return m_format; }

Prisma::PipelineHandler::TextureData Prisma::PipelineHandler::textureData() {
    return m_textureData;
}