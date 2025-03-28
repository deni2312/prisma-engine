#include "../../include/Pipelines/PipelineSkybox.h"
#include "../../include/GlobalData/GlobalData.h"
#include "../../include/Pipelines/PipelineLUT.h"
#include "../../include/Helpers/PrismaRender.h"
#include "../../include/Pipelines/PipelineDIffuseIrradiance.h"
#include "../../include/Pipelines/PipelinePrefilter.h"


#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "Graphics/GraphicsTools/interface/MapHelper.hpp"

Prisma::PipelineSkybox::PipelineSkybox()
{
    auto& contextData = Prisma::PrismaFunc::getInstance().contextData();

    // Pipeline state object encompasses configuration of all GPU stages

    Diligent::GraphicsPipelineStateCreateInfo PSOCreateInfo;

    // Pipeline state name is used by the engine to report issues.
    // It is always a good idea to give objects descriptive names.
    PSOCreateInfo.PSODesc.Name = "Skybox Render";

    // This is a graphics pipeline
    PSOCreateInfo.PSODesc.PipelineType = Diligent::PIPELINE_TYPE_GRAPHICS;

    // clang-format off
    // This tutorial will render to a single render target
    PSOCreateInfo.GraphicsPipeline.NumRenderTargets = 1;
    // Set render target format which is the format of the swap chain's color buffer
    PSOCreateInfo.GraphicsPipeline.RTVFormats[0] = Diligent::TEX_FORMAT_RGBA16_FLOAT;
    // Primitive topology defines what kind of primitives will be rendered by this pipeline state
    PSOCreateInfo.GraphicsPipeline.PrimitiveTopology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    PSOCreateInfo.GraphicsPipeline.RasterizerDesc.FrontCounterClockwise = true;
    // Cull back faces
    PSOCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode = Diligent::CULL_MODE_NONE;
    // Enable depth testing
    PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable = false;

    Diligent::ShaderCreateInfo ShaderCI;
    // Tell the system that the shader source code is in HLSL.
    // For OpenGL, the engine will convert this into GLSL under the hood.
    ShaderCI.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_HLSL;

    // OpenGL backend requires emulated combined HLSL texture samplers (g_Texture + g_Texture_sampler combination)
    ShaderCI.Desc.UseCombinedTextureSamplers = true;

    // Pack matrices in row-major order
    ShaderCI.CompileFlags = Diligent::SHADER_COMPILE_FLAG_PACK_MATRIX_ROW_MAJOR;

    // In this tutorial, we will load shaders from file. To be able to do that,
    // we need to create a shader source stream factory
    Diligent::RefCntAutoPtr<Diligent::IShaderSourceInputStreamFactory> pShaderSourceFactory;
    Prisma::PrismaFunc::getInstance().contextData().m_pEngineFactory->CreateDefaultShaderSourceStreamFactory(nullptr, &pShaderSourceFactory);
    ShaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;
    // Create a vertex shader
    Diligent::RefCntAutoPtr<Diligent::IShader> pVS;
    {
        ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_VERTEX;
        ShaderCI.EntryPoint = "main";
        ShaderCI.Desc.Name = "Skybox VS";
        ShaderCI.FilePath = "../../../Engine/Shaders/EquirectangularPipeline/vertex.hlsl";
        contextData.m_pDevice->CreateShader(ShaderCI, &pVS);
    }

    // Create a pixel shader
    Diligent::RefCntAutoPtr<Diligent::IShader> pPS;
    {
        ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_PIXEL;
        ShaderCI.EntryPoint = "main";
        ShaderCI.Desc.Name = "Skybox PS";
        ShaderCI.FilePath = "../../../Engine/Shaders/EquirectangularPipeline/fragment.hlsl";
        contextData.m_pDevice->CreateShader(ShaderCI, &pPS);
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


    Diligent::ShaderResourceVariableDesc Vars[] =
    {
        {Diligent::SHADER_TYPE_PIXEL, "equirectangularMap", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE}
    };
    // clang-format on
    PSOCreateInfo.PSODesc.ResourceLayout.Variables = Vars;
    PSOCreateInfo.PSODesc.ResourceLayout.NumVariables = _countof(Vars);

    // clang-format off
    // Define immutable sampler for g_Texture. Immutable samplers should be used whenever possible
    Diligent::SamplerDesc SamLinearClampDesc
    {
        Diligent::FILTER_TYPE_LINEAR, Diligent::FILTER_TYPE_LINEAR, Diligent::FILTER_TYPE_LINEAR,
        Diligent::TEXTURE_ADDRESS_CLAMP, Diligent::TEXTURE_ADDRESS_CLAMP, Diligent::TEXTURE_ADDRESS_CLAMP
    };

	Diligent::ImmutableSamplerDesc ImtblSamplers[] =
    {
        {Diligent::SHADER_TYPE_PIXEL, "equirectangularMap", SamLinearClampDesc}
    };
    // clang-format on
    PSOCreateInfo.PSODesc.ResourceLayout.ImmutableSamplers = ImtblSamplers;
    PSOCreateInfo.PSODesc.ResourceLayout.NumImmutableSamplers = _countof(ImtblSamplers);
    contextData.m_pDevice->CreateGraphicsPipelineState(PSOCreateInfo, &m_pso);

    Diligent::BufferDesc CBDesc;
    CBDesc.Name = "IBLData";
    CBDesc.Size = sizeof(IBLViewProjection);
    CBDesc.Usage = Diligent::USAGE_DYNAMIC;
    CBDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
    CBDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;

    contextData.m_pDevice->CreateBuffer(CBDesc, nullptr, &m_iblData);

    m_pso->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "IBLData")->Set(m_iblData);

    m_pso->CreateShaderResourceBinding(&m_srb, true);

    Diligent::TextureDesc RTColorDesc;
    RTColorDesc.Name = "Offscreen render target";
    RTColorDesc.Type = Diligent::RESOURCE_DIM_TEX_CUBE;  // Set cubemap type
    RTColorDesc.Width = m_dimensions.x;
    RTColorDesc.Height = m_dimensions.y;
    RTColorDesc.MipLevels = 8;
    RTColorDesc.Format = Diligent::TEX_FORMAT_RGBA16_FLOAT;
    // Specify 6 faces for cubemap
    RTColorDesc.ArraySize = 6;
    // Allow it to be used as both a shader resource and a render target
    RTColorDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE | Diligent::BIND_RENDER_TARGET;
    // Define optimal clear value
    RTColorDesc.ClearValue.Format = RTColorDesc.Format;
    RTColorDesc.MiscFlags = Diligent::MISC_TEXTURE_FLAG_GENERATE_MIPS;
    // Create the cubemap texture
    contextData.m_pDevice->CreateTexture(RTColorDesc, nullptr, &m_pMSColorRTV);

    // Create render target views for each face
    for (int i = 0; i < 6; ++i)
    {
	    Diligent::TextureViewDesc RTVDesc;
        RTVDesc.ViewType = Diligent::TEXTURE_VIEW_RENDER_TARGET;
        RTVDesc.TextureDim = Diligent::RESOURCE_DIM_TEX_2D_ARRAY;
        RTVDesc.MostDetailedMip = 0;
        RTVDesc.NumMipLevels = 1;
        RTVDesc.FirstArraySlice = i;  // Select the specific face
        RTVDesc.NumArraySlices = 1;

        m_pMSColorRTV->CreateView(RTVDesc,&m_pRTColor[i]);
    }


    m_skyboxRenderer = std::make_shared<Prisma::PipelineSkyboxRenderer>();
}

const Prisma::Texture& Prisma::PipelineSkybox::texture() const
{
	return m_texture;
}

bool Prisma::PipelineSkybox::isInit()
{
    return m_init;
}

void Prisma::PipelineSkybox::addUpdate(std::function<void()> update)
{
    m_update.push_back(update);
}

void Prisma::PipelineSkybox::calculateSkybox()
{
    m_srb.Release();
    m_pso->CreateShaderResourceBinding(&m_srb, true);
    auto& contextData = Prisma::PrismaFunc::getInstance().contextData();

    m_srb->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "equirectangularMap")->Set(m_texture.texture()->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE));
    for (unsigned int i = 0; i < 6; ++i) {
        Diligent::MapHelper<IBLViewProjection> viewProjection(contextData.m_pImmediateContext, m_iblData, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
        viewProjection->view = m_iblTransform.captureViews[i];
        viewProjection->projection = m_iblTransform.captureProjection;


        Diligent::TextureViewDesc   RTVDesc{ "Skybox", Diligent::TEXTURE_VIEW_RENDER_TARGET, Diligent::RESOURCE_DIM_TEX_2D_ARRAY };

        Diligent::ITextureView* ppRTVs[] = { m_pRTColor[i] };

        contextData.m_pImmediateContext->SetRenderTargets(1, ppRTVs, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        contextData.m_pImmediateContext->ClearRenderTarget(m_pRTColor[i], glm::value_ptr(Prisma::Define::CLEAR_COLOR), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        contextData.m_pImmediateContext->SetPipelineState(m_pso);

        auto cubeBuffer = Prisma::PrismaRender::getInstance().cubeBuffer();

        // Bind vertex and index buffers
        const Diligent::Uint64 offset = 0;
        Diligent::IBuffer* pBuffs[] = { cubeBuffer.vBuffer };
        contextData.m_pImmediateContext->SetVertexBuffers(0, 1, pBuffs, &offset, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);
        contextData.m_pImmediateContext->SetIndexBuffer(cubeBuffer.iBuffer, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        // Set texture SRV in the SRB
        contextData.m_pImmediateContext->CommitShaderResources(m_srb, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        Diligent::DrawIndexedAttribs DrawAttrs;     // This is an indexed draw call
        DrawAttrs.IndexType = Diligent::VT_UINT32; // Index type
        DrawAttrs.NumIndices = cubeBuffer.iBufferSize;
        // Verify the state of vertex and index buffers
        DrawAttrs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL;
        contextData.m_pImmediateContext->DrawIndexed(DrawAttrs);
    }
    contextData.m_pImmediateContext->GenerateMips(m_pMSColorRTV->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE));
    m_skyboxRenderer->texture(m_pMSColorRTV);
    Prisma::PrismaFunc::getInstance().bindMainRenderTarget();

}

void Prisma::PipelineSkybox::render()
{
    if (m_init) {
        m_skyboxRenderer->render();
    }
}

void Prisma::PipelineSkybox::texture(Texture texture)
{
	m_texture = texture;
	calculateSkybox();
	PipelineLUT::getInstance().texture();
    PipelinePrefilter::getInstance().texture(m_pMSColorRTV);
    PipelineDiffuseIrradiance::getInstance().texture(m_pMSColorRTV);
    m_init = true;

    for (auto& update:m_update)
    {
        update();
    }
}
