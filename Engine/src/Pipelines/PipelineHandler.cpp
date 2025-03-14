#include "../../include/Pipelines/PipelineHandler.h"
#include "../../include/Helpers/SettingsLoader.h"

bool Prisma::PipelineHandler::initScene(SceneLoader::SceneParameters sceneParameters)
{
	//if (Prisma::GlobalData::getInstance().currentGlobalScene() && Prisma::GlobalData::getInstance().currentGlobalScene()->camera)
	{
		auto settings = SettingsLoader::getInstance().getSettings();
		m_forwardPipeline = std::make_shared<PipelineForward>(settings.width, settings.height, sceneParameters.srgb);

		//m_deferredPipeline = std::make_shared<PipelineDeferred>(settings.width, settings.height, sceneParameters.srgb);

		//m_deferredForwardPipeline = std::make_shared<PipelineDeferredForward>(settings.width, settings.height, sceneParameters.srgb);
		return true;
	}
	return false;
}

std::shared_ptr<Prisma::PipelineForward> Prisma::PipelineHandler::forward()
{
	return m_forwardPipeline;
}

std::shared_ptr<Prisma::PipelineDeferred> Prisma::PipelineHandler::deferred()
{
	return m_deferredPipeline;
}

std::shared_ptr<Prisma::PipelineDeferredForward> Prisma::PipelineHandler::deferredForward()
{
	return m_deferredForwardPipeline;
}

Prisma::PipelineHandler::PipelineHandler()
{
    auto& contextData = Prisma::PrismaFunc::getInstance().contextData();

	Diligent::TextureDesc RTColorDesc;
    RTColorDesc.Name = "Offscreen render target";
    RTColorDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
    RTColorDesc.Width = contextData.m_pSwapChain->GetDesc().Width;
    RTColorDesc.Height = contextData.m_pSwapChain->GetDesc().Height;
    RTColorDesc.MipLevels = 1;
    RTColorDesc.Format = Prisma::PrismaFunc::getInstance().renderFormat().RenderFormat;
    // The render target can be bound as a shader resource and as a render target
    RTColorDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE | Diligent::BIND_RENDER_TARGET;
    // Define optimal clear value
    RTColorDesc.ClearValue.Format = RTColorDesc.Format;
    RTColorDesc.ClearValue.Color[0] = 0.350f;
    RTColorDesc.ClearValue.Color[1] = 0.350f;
    RTColorDesc.ClearValue.Color[2] = 0.350f;
    RTColorDesc.ClearValue.Color[3] = 1.f;
	Diligent::RefCntAutoPtr<Diligent::ITexture> pRTColor;
    contextData.m_pDevice->CreateTexture(RTColorDesc, nullptr, &pRTColor);

    m_textureData.pColorRTV = pRTColor->GetDefaultView(Diligent::TEXTURE_VIEW_RENDER_TARGET);

    // Create window-size depth buffer
    Diligent::TextureDesc RTDepthDesc = RTColorDesc;
    RTDepthDesc.Name = "Offscreen depth buffer";
    RTDepthDesc.Format = Prisma::PrismaFunc::getInstance().renderFormat().DepthBufferFormat;
    RTDepthDesc.BindFlags = Diligent::BIND_DEPTH_STENCIL;
    // Define optimal clear value
    RTDepthDesc.ClearValue.Format = RTDepthDesc.Format;
    RTDepthDesc.ClearValue.DepthStencil.Depth = 1;
    RTDepthDesc.ClearValue.DepthStencil.Stencil = 0;
    Diligent::RefCntAutoPtr<Diligent::ITexture> pRTDepth;
    contextData.m_pDevice->CreateTexture(RTDepthDesc, nullptr, &pRTDepth);
    // Store the depth-stencil view
    m_textureData.pDepthDSV = pRTDepth->GetDefaultView(Diligent::TEXTURE_VIEW_DEPTH_STENCIL);
}

Prisma::PipelineHandler::TextureData Prisma::PipelineHandler::textureData()
{
	return m_textureData;
}
