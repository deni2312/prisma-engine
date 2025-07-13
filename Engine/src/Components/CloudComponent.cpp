# include "Components/CloudComponent.h"

#include "GlobalData/PrismaFunc.h"
#include "Helpers/PrismaRender.h"
#include "Pipelines/PipelineHandler.h"
#include <Graphics/GraphicsTools/interface/MapHelper.hpp>
#include "Postprocess/Postprocess.h"
#include <GlobalData/GlobalShaderNames.h>
#include "Helpers/SettingsLoader.h"
#include <Helpers/NoiseGenerator.h>

Prisma::CloudComponent::CloudComponent() { name("CloudComponent"); }

void Prisma::CloudComponent::ui() { 
    
    Prisma::Component::ui(); 
    ComponentType componentAmplitude;
    componentAmplitude = std::make_tuple(TYPES::FLOAT, "Cloud Amplitude", &m_constants.amplitude);
    addGlobal({componentAmplitude, false});

    ComponentType componentFrequency;
    componentFrequency = std::make_tuple(TYPES::FLOAT, "Cloud Frequency", &m_constants.frequency);
    addGlobal({componentFrequency, false});

    ComponentType componentBeers;
    componentBeers = std::make_tuple(TYPES::FLOAT, "Cloud Beers", &m_constants.beersLaw);
    addGlobal({componentBeers, false});


}

void Prisma::CloudComponent::update() {

}

void Prisma::CloudComponent::start() {
    Component::start();
    auto& contextData = PrismaFunc::getInstance().contextData();

    // Pipeline state object encompasses configuration of all GPU stages

    Diligent::GraphicsPipelineStateCreateInfo PSOCreateInfo;

    // Pipeline state name is used by the engine to report issues.
    // It is always a good idea to give objects descriptive names.
    PSOCreateInfo.PSODesc.Name = "CloudComponent Render";
    // This is a graphics pipeline
    PSOCreateInfo.PSODesc.PipelineType = Diligent::PIPELINE_TYPE_GRAPHICS;

    // clang-format off
    PSOCreateInfo.GraphicsPipeline.NumRenderTargets = 2;
    // Set render target format which is the format of the swap chain's color buffer
    PSOCreateInfo.GraphicsPipeline.RTVFormats[0] = PipelineHandler::getInstance().textureFormat();
    PSOCreateInfo.GraphicsPipeline.RTVFormats[1] = Diligent::TEX_FORMAT_R16_FLOAT;
    // Set depth buffer format which is the format of the swap chain's back buffer
    PSOCreateInfo.GraphicsPipeline.DSVFormat = PrismaFunc::getInstance().renderFormat().DepthBufferFormat;
    // Primitive topology defines what kind of primitives will be rendered by this pipeline state
    PSOCreateInfo.GraphicsPipeline.PrimitiveTopology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    PSOCreateInfo.GraphicsPipeline.RasterizerDesc.FrontCounterClockwise = true;
    // Cull back faces
    PSOCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode = Diligent::CULL_MODE_NONE;
    // Enable depth testing
    PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable = true;
    PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthWriteEnable = false;

    // Enable blending
    PSOCreateInfo.GraphicsPipeline.BlendDesc.AlphaToCoverageEnable = false;
    PSOCreateInfo.GraphicsPipeline.BlendDesc.IndependentBlendEnable = true;

    // Configure blending for Render Target 0
    auto& RT0 = PSOCreateInfo.GraphicsPipeline.BlendDesc.RenderTargets[0];
    RT0.BlendEnable = true;
    RT0.SrcBlend = Diligent::BLEND_FACTOR_ONE;
    RT0.DestBlend = Diligent::BLEND_FACTOR_ONE;
    RT0.BlendOp = Diligent::BLEND_OPERATION_ADD;
    RT0.SrcBlendAlpha = Diligent::BLEND_FACTOR_ONE;
    RT0.DestBlendAlpha = Diligent::BLEND_FACTOR_ONE;
    RT0.BlendOpAlpha = Diligent::BLEND_OPERATION_ADD;
    RT0.RenderTargetWriteMask = Diligent::COLOR_MASK_ALL;

    // Configure blending for Render Target 1
    auto& RT1 = PSOCreateInfo.GraphicsPipeline.BlendDesc.RenderTargets[1];
    RT1.BlendEnable = true;
    RT1.SrcBlend = Diligent::BLEND_FACTOR_ZERO;
    RT1.DestBlend = Diligent::BLEND_FACTOR_INV_SRC_COLOR;
    RT1.BlendOp = Diligent::BLEND_OPERATION_ADD;
    RT1.SrcBlendAlpha = Diligent::BLEND_FACTOR_ZERO;
    RT1.DestBlendAlpha = Diligent::BLEND_FACTOR_INV_SRC_COLOR;
    RT1.BlendOpAlpha = Diligent::BLEND_OPERATION_ADD;
    RT1.RenderTargetWriteMask = Diligent::COLOR_MASK_ALL;

    Diligent::ShaderCreateInfo ShaderCI;
    // Tell the system that the shader source code is in HLSL.
    // For OpenGL, the engine will convert this into GLSL under the hood.
    ShaderCI.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_GLSL;

    // OpenGL backend requires emulated combined HLSL texture samplers (g_Texture + g_Texture_sampler combination)
    ShaderCI.Desc.UseCombinedTextureSamplers = true;
    // In this tutorial, we will load shaders from file. To be able to do that,
    // we need to create a shader source stream factory
    Diligent::RefCntAutoPtr<Diligent::IShaderSourceInputStreamFactory> pShaderSourceFactory;
    PrismaFunc::getInstance().contextData().engineFactory->CreateDefaultShaderSourceStreamFactory(nullptr, &pShaderSourceFactory);
    ShaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;
    // Create a vertex shader
    Diligent::RefCntAutoPtr<Diligent::IShader> pVS;
    {
        ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_VERTEX;
        ShaderCI.EntryPoint = "main";
        ShaderCI.Desc.Name = "Cloud VS";
        ShaderCI.FilePath = "../../../Engine/Shaders/CloudPipeline/vertex.glsl";
        contextData.device->CreateShader(ShaderCI, &pVS);
    }

    // Create a pixel shader
    Diligent::RefCntAutoPtr<Diligent::IShader> pPS;
    {
        ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_PIXEL;
        ShaderCI.EntryPoint = "main";
        ShaderCI.Desc.Name = "Cloud PS";
        ShaderCI.FilePath = "../../../Engine/Shaders/CloudPipeline/fragment.glsl";
        contextData.device->CreateShader(ShaderCI, &pPS);
    }

    // clang-format off
    // Define vertex shader input layout
	Diligent::LayoutElement LayoutElems[] =
    {
        // Attribute 0 - vertex position
        Diligent::LayoutElement{0, 0, 3, Diligent::VT_FLOAT32, Diligent::False},
        // Attribute 1 - texture coordinates
        Diligent::LayoutElement{1, 0, 2, Diligent::VT_FLOAT32, Diligent::False}
    };
    // clang-format on
    PSOCreateInfo.GraphicsPipeline.InputLayout.LayoutElements = LayoutElems;
    PSOCreateInfo.GraphicsPipeline.InputLayout.NumElements = _countof(LayoutElems);

    PSOCreateInfo.pVS = pVS;
    PSOCreateInfo.pPS = pPS;

    // Define variable type that will be used by default
    PSOCreateInfo.PSODesc.ResourceLayout.DefaultVariableType = Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

    Diligent::ShaderResourceVariableDesc Vars[] = {{Diligent::SHADER_TYPE_PIXEL, "Constants", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
                                                   {Diligent::SHADER_TYPE_PIXEL, Prisma::ShaderNames::CONSTANT_VIEW_PROJECTION.c_str(), Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC}};
    // clang-format on
    PSOCreateInfo.PSODesc.ResourceLayout.Variables = Vars;
    PSOCreateInfo.PSODesc.ResourceLayout.NumVariables = _countof(Vars);

    Diligent::SamplerDesc SamLinearClampDesc{Diligent::FILTER_TYPE_LINEAR, Diligent::FILTER_TYPE_LINEAR, Diligent::FILTER_TYPE_LINEAR, Diligent::TEXTURE_ADDRESS_WRAP, Diligent::TEXTURE_ADDRESS_WRAP, Diligent::TEXTURE_ADDRESS_WRAP};

    contextData.device->CreateGraphicsPipelineState(PSOCreateInfo, &m_pso);

    Diligent::BufferDesc CBDesc;
    CBDesc.Name = "CloudComponent Constants";
    CBDesc.Size = sizeof(CloudConstants);
    CBDesc.Usage = Diligent::USAGE_DYNAMIC;
    CBDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
    CBDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
    contextData.device->CreateBuffer(CBDesc, nullptr, &m_cloudConstants);

    Diligent::TextureDesc RTColorDesc;
    RTColorDesc.Name = "Offscreen render target";
    RTColorDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
    RTColorDesc.Width = contextData.swapChain->GetDesc().Width;
    RTColorDesc.Height = contextData.swapChain->GetDesc().Height;
    RTColorDesc.MipLevels = 1;
    RTColorDesc.Format = Prisma::PipelineHandler::getInstance().textureFormat();
    // The render target can be bound as a shader resource and as a render target
    RTColorDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE | Diligent::BIND_RENDER_TARGET;
    // Define optimal clear value
    RTColorDesc.ClearValue.Format = RTColorDesc.Format;
    RTColorDesc.ClearValue.Color[0] = 0.350f;
    RTColorDesc.ClearValue.Color[1] = 0.350f;
    RTColorDesc.ClearValue.Color[2] = 0.350f;
    RTColorDesc.ClearValue.Color[3] = 1.f;
    contextData.device->CreateTexture(RTColorDesc, nullptr, &m_cloudTexture);
    m_pso->GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "Constants")->Set(m_cloudConstants);

    m_pso->GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, Prisma::ShaderNames::CONSTANT_VIEW_PROJECTION.c_str())->Set(Prisma::MeshHandler::getInstance().viewProjection());

    m_blit = std::make_unique<Blit>(m_cloudTexture);

    m_pso->CreateShaderResourceBinding(&m_srb, true);
    GlobalData::getInstance().addGlobalTexture({m_cloudTexture, "Cloud Texture"});
    m_counter.start();
    m_settings = Prisma::SettingsLoader::getInstance().getSettings();

    m_constants.resolution = glm::vec4(m_settings.width, m_settings.height, 0,0);

    m_texture = Prisma::NoiseGenerator::getInstance().generate("../../../Engine/Shaders/PerlinPipeline/vertex.glsl", "../../../Engine/Shaders/PerlinPipeline/fragment.glsl", {512, 512}, "Perlin Texture");
}

void Prisma::CloudComponent::destroy() { 
    Component::destroy(); 
}

void Prisma::CloudComponent::updatePreRender(Diligent::RefCntAutoPtr<Diligent::ITexture> texture, Diligent::RefCntAutoPtr<Diligent::ITexture> depth) {}

void Prisma::CloudComponent::updatePostRender(Diligent::RefCntAutoPtr<Diligent::ITexture> texture, Diligent::RefCntAutoPtr<Diligent::ITexture> depth) {

}

void Prisma::CloudComponent::updateTransparentRender(Diligent::RefCntAutoPtr<Diligent::ITexture> accum, Diligent::RefCntAutoPtr<Diligent::ITexture> reveal, Diligent::RefCntAutoPtr<Diligent::ITexture> depth) {
    auto& contextData = PrismaFunc::getInstance().contextData();

    auto accumData = accum->GetDefaultView(Diligent::TEXTURE_VIEW_RENDER_TARGET);
    auto revealData = reveal->GetDefaultView(Diligent::TEXTURE_VIEW_RENDER_TARGET);
    Diligent::ITextureView* textures[] = {accumData, revealData};
    auto pDSV = PipelineHandler::getInstance().textureData().pDepthDSV->GetDefaultView(Diligent::TEXTURE_VIEW_DEPTH_STENCIL);

    // Clear the back buffer
    contextData.immediateContext->SetRenderTargets(2, textures, pDSV, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    // Clear the back buffer
    contextData.immediateContext->SetPipelineState(m_pso);

    auto quadBuffer = PrismaRender::getInstance().quadBuffer();

    m_constants.cloudPosition = parent()->finalMatrix()[3];
    m_constants.time = m_counter.duration_seconds();

    auto camera = GlobalData::getInstance().currentGlobalScene()->camera;
    Diligent::MapHelper<CloudConstants> cloudConstants(contextData.immediateContext, m_cloudConstants, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);

    *cloudConstants = m_constants;


    constexpr Diligent::Uint64 offset = 0;
    Diligent::IBuffer* pBuffs[] = {quadBuffer.vBuffer};
    contextData.immediateContext->SetVertexBuffers(0, 1, pBuffs, &offset, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);
    contextData.immediateContext->SetIndexBuffer(quadBuffer.iBuffer, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    // Set texture SRV in the SRB
    contextData.immediateContext->CommitShaderResources(m_srb, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    Diligent::DrawIndexedAttribs DrawAttrs;     // This is an indexed draw call
    DrawAttrs.IndexType = Diligent::VT_UINT32;  // Index type
    DrawAttrs.NumIndices = quadBuffer.iBufferSize;
    // Verify the state of vertex and index buffers
    DrawAttrs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL;
    contextData.immediateContext->DrawIndexed(DrawAttrs);

    auto pRTV = PipelineHandler::getInstance().textureData().pColorRTV->GetDefaultView(Diligent::TEXTURE_VIEW_RENDER_TARGET);

    // Clear the back buffer
    contextData.immediateContext->SetRenderTargets(1, &pRTV, pDSV, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    
}

