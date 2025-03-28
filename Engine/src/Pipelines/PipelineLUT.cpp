#include "../../include/Pipelines/PipelineLUT.h"
#include "../../include/GlobalData/GlobalData.h"
#include "../../include/Pipelines/PipelineDIffuseIrradiance.h"
#include "../../include/Pipelines/PipelinePrefilter.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "../../include/Helpers/PrismaRender.h"
#include "../../include/Helpers/SettingsLoader.h"


Prisma::PipelineLUT::PipelineLUT()
{
	//m_shader = std::make_shared<Shader>("../../../Engine/Shaders/LUTPipeline/vertex.glsl","../../../Engine/Shaders/LUTPipeline/fragment.glsl");

    auto& contextData = Prisma::PrismaFunc::getInstance().contextData();

    // Pipeline state object encompasses configuration of all GPU stages

    Diligent::GraphicsPipelineStateCreateInfo PSOCreateInfo;

    // Pipeline state name is used by the engine to report issues.
    // It is always a good idea to give objects descriptive names.
    PSOCreateInfo.PSODesc.Name = "LUT Render";

    // This is a graphics pipeline
    PSOCreateInfo.PSODesc.PipelineType = Diligent::PIPELINE_TYPE_GRAPHICS;

    // clang-format off
    // This tutorial will render to a single render target
    PSOCreateInfo.GraphicsPipeline.NumRenderTargets = 1;
    // Set render target format which is the format of the swap chain's color buffer
    PSOCreateInfo.GraphicsPipeline.RTVFormats[0] = Diligent::TEX_FORMAT_RG16_FLOAT;
    // Primitive topology defines what kind of primitives will be rendered by this pipeline state
    PSOCreateInfo.GraphicsPipeline.PrimitiveTopology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    PSOCreateInfo.GraphicsPipeline.RasterizerDesc.FrontCounterClockwise = true;
    // Cull back faces
    PSOCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode = Diligent::CULL_MODE_BACK;
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
        ShaderCI.Desc.Name = "LUT VS";
        ShaderCI.FilePath = "../../../Engine/Shaders/LUTPipeline/vertex.hlsl";
        contextData.m_pDevice->CreateShader(ShaderCI, &pVS);
    }

    // Create a pixel shader
    Diligent::RefCntAutoPtr<Diligent::IShader> pPS;
    {
        ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_PIXEL;
        ShaderCI.EntryPoint = "main";
        ShaderCI.Desc.Name = "LUT PS";
        ShaderCI.FilePath = "../../../Engine/Shaders/LUTPipeline/fragment.hlsl";
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
    contextData.m_pDevice->CreateGraphicsPipelineState(PSOCreateInfo, &m_pso);

    Diligent::TextureDesc RTColorDesc;
    RTColorDesc.Name = "Offscreen render target";
    RTColorDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
    RTColorDesc.Width = m_dimensions.x;
    RTColorDesc.Height = m_dimensions.y;
    RTColorDesc.MipLevels = 1;
    RTColorDesc.Format = Diligent::TEX_FORMAT_RG16_FLOAT;
    // The render target can be bound as a shader resource and as a render target
    RTColorDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE | Diligent::BIND_RENDER_TARGET;
    // Define optimal clear value
    RTColorDesc.ClearValue.Format = RTColorDesc.Format;

    contextData.m_pDevice->CreateTexture(RTColorDesc, nullptr, &m_pRTColor);

    m_pMSColorRTV = m_pRTColor->GetDefaultView(Diligent::TEXTURE_VIEW_RENDER_TARGET);
    m_pso->CreateShaderResourceBinding(&m_srb, true);


    Prisma::GlobalData::getInstance().addGlobalTexture({ m_pRTColor,"LUT" });
}

void Prisma::PipelineLUT::texture()
{

    auto& contextData = Prisma::PrismaFunc::getInstance().contextData();

    // Clear the back buffer
    contextData.m_pImmediateContext->SetRenderTargets(1, &m_pMSColorRTV, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    contextData.m_pImmediateContext->ClearRenderTarget(m_pMSColorRTV, glm::value_ptr(Define::CLEAR_COLOR), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    contextData.m_pImmediateContext->SetPipelineState(m_pso);

    auto quadBuffer = Prisma::PrismaRender::getInstance().quadBuffer();

    // Bind vertex and index buffers
    const Diligent::Uint64 offset = 0;
    Diligent::IBuffer* pBuffs[] = { quadBuffer.vBuffer };
    contextData.m_pImmediateContext->SetVertexBuffers(0, 1, pBuffs, &offset, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);
    contextData.m_pImmediateContext->SetIndexBuffer(quadBuffer.iBuffer, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    // Set texture SRV in the SRB
    contextData.m_pImmediateContext->CommitShaderResources(m_srb, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    Diligent::DrawIndexedAttribs DrawAttrs;     // This is an indexed draw call
    DrawAttrs.IndexType = Diligent::VT_UINT32; // Index type
    DrawAttrs.NumIndices = quadBuffer.iBufferSize;
    // Verify the state of vertex and index buffers
    DrawAttrs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL;
    contextData.m_pImmediateContext->DrawIndexed(DrawAttrs);
    Prisma::PrismaFunc::getInstance().bindMainRenderTarget();
}

Diligent::RefCntAutoPtr<Diligent::ITexture> Prisma::PipelineLUT::lutTexture()
{
    return m_pRTColor;
}
