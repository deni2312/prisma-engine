#include "Pipelines/PipelineBlitRT.h"

#include "GlobalData/PrismaFunc.h"
#include "Helpers/PrismaRender.h"
#include "Pipelines/PipelineHandler.h"
#include "Graphics/GraphicsEngine/interface/GraphicsTypes.h"
#include "Graphics/GraphicsTools/interface/MapHelper.hpp"

Prisma::PipelineBlitRT::PipelineBlitRT(Diligent::RefCntAutoPtr<Diligent::ITexture> texture) {
        auto& contextData = PrismaFunc::getInstance().contextData();

        // Pipeline state object encompasses configuration of all GPU stages

        Diligent::GraphicsPipelineStateCreateInfo PSOCreateInfo;

        // Pipeline state name is used by the engine to report issues.
        // It is always a good idea to give objects descriptive names.
        PSOCreateInfo.PSODesc.Name = "BlitRT";

        // This is a graphics pipeline
        PSOCreateInfo.PSODesc.PipelineType = Diligent::PIPELINE_TYPE_GRAPHICS;

    // clang-format off
    // This tutorial will render to a single render target
    PSOCreateInfo.GraphicsPipeline.NumRenderTargets = 1;
    // Set render target format which is the format of the swap chain's color buffer
    PSOCreateInfo.GraphicsPipeline.RTVFormats[0] = PrismaFunc::getInstance().renderFormat().RenderFormat;
    // Set depth buffer format which is the format of the swap chain's back buffer
    PSOCreateInfo.GraphicsPipeline.DSVFormat = PrismaFunc::getInstance().renderFormat().DepthBufferFormat;
    // Primitive topology defines what kind of primitives will be rendered by this pipeline state
    PSOCreateInfo.GraphicsPipeline.PrimitiveTopology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    PSOCreateInfo.GraphicsPipeline.RasterizerDesc.FrontCounterClockwise = true;
    // Cull back faces
    PSOCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode = Diligent::CULL_MODE_BACK;
    // Enable depth testing
    PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable = true;
        // clang-format on

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
        PrismaFunc::getInstance().contextData().m_pEngineFactory->CreateDefaultShaderSourceStreamFactory(
                nullptr, &pShaderSourceFactory);
        ShaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;
        // Create a vertex shader
        Diligent::RefCntAutoPtr<Diligent::IShader> pVS;
        {
                ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_VERTEX;
                ShaderCI.EntryPoint = "main";
                ShaderCI.Desc.Name = "BlitRT VS";
                ShaderCI.FilePath = "../../../Engine/Shaders/BlitRTPipeline/vertex.hlsl";
                contextData.m_pDevice->CreateShader(ShaderCI, &pVS);
        }

        // Create a pixel shader
        Diligent::RefCntAutoPtr<Diligent::IShader> pPS;
        {
                ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_PIXEL;
                ShaderCI.EntryPoint = "main";
                ShaderCI.Desc.Name = "BlitRT PS";
                ShaderCI.FilePath = "../../../Engine/Shaders/BlitRTPipeline/fragment.hlsl";
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
                {Diligent::SHADER_TYPE_PIXEL, "g_Texture", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC}
        };
        // clang-format on
        PSOCreateInfo.PSODesc.ResourceLayout.Variables = Vars;
        PSOCreateInfo.PSODesc.ResourceLayout.NumVariables = _countof(Vars);

    // clang-format off
    // Define immutable sampler for g_Texture. Immutable samplers should be used whenever possible
    Diligent::SamplerDesc SamLinearClampDesc
    {
        Diligent::FILTER_TYPE_LINEAR, Diligent::FILTER_TYPE_LINEAR, Diligent::FILTER_TYPE_LINEAR,
        Diligent::TEXTURE_ADDRESS_WRAP, Diligent::TEXTURE_ADDRESS_WRAP, Diligent::TEXTURE_ADDRESS_WRAP
    };
    Diligent::ImmutableSamplerDesc ImtblSamplers[] =
    {
        {Diligent::SHADER_TYPE_PIXEL, "g_Texture", SamLinearClampDesc}
    };
        // clang-format on
        PSOCreateInfo.PSODesc.ResourceLayout.ImmutableSamplers = ImtblSamplers;
        PSOCreateInfo.PSODesc.ResourceLayout.NumImmutableSamplers = _countof(ImtblSamplers);
        contextData.m_pDevice->CreateGraphicsPipelineState(PSOCreateInfo, &m_pso);
        m_pso->GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Texture")->Set(
                texture->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE));

        m_pso->CreateShaderResourceBinding(&m_srb, true);
}

void Prisma::PipelineBlitRT::blit() {
        auto& contextData = PrismaFunc::getInstance().contextData();
        auto pRTV = PipelineHandler::getInstance().textureData().pColorRTV->GetDefaultView(
                Diligent::TEXTURE_VIEW_RENDER_TARGET);
        auto pDSV = PipelineHandler::getInstance().textureData().pDepthDSV->GetDefaultView(
                Diligent::TEXTURE_VIEW_DEPTH_STENCIL);
        contextData.m_pImmediateContext->SetRenderTargets(1, &pRTV, pDSV,
                                                          Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        contextData.m_pImmediateContext->ClearRenderTarget(pRTV, value_ptr(Define::CLEAR_COLOR),
                                                           Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        contextData.m_pImmediateContext->ClearDepthStencil(pDSV, Diligent::CLEAR_DEPTH_FLAG, 1.f, 0,
                                                           Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        contextData.m_pImmediateContext->SetPipelineState(m_pso);

        auto quadBuffer = PrismaRender::getInstance().quadBuffer();

        // Bind vertex and index buffers
        constexpr Diligent::Uint64 offset = 0;
        Diligent::IBuffer* pBuffs[] = {quadBuffer.vBuffer};
        contextData.m_pImmediateContext->SetVertexBuffers(0, 1, pBuffs, &offset,
                                                          Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
                                                          Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);
        contextData.m_pImmediateContext->SetIndexBuffer(quadBuffer.iBuffer, 0,
                                                        Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        // Set texture SRV in the SRB
        contextData.m_pImmediateContext->CommitShaderResources(
                m_srb, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        Diligent::DrawIndexedAttribs DrawAttrs; // This is an indexed draw call
        DrawAttrs.IndexType = Diligent::VT_UINT32; // Index type
        DrawAttrs.NumIndices = quadBuffer.iBufferSize;
        // Verify the state of vertex and index buffers
        DrawAttrs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL;
        contextData.m_pImmediateContext->DrawIndexed(DrawAttrs);
}