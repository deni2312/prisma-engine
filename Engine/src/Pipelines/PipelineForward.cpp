#include "../../include/GlobalData/GlobalData.h"
#include "../../include/Pipelines/PipelineForward.h"
#include "../../include/Helpers/PrismaRender.h"
#include "../../include/SceneData/MeshIndirect.h"
#include "../../include/SceneObjects/Mesh.h"
#include "../../include/Pipelines/PipelineSkybox.h"
#include "../../include/Pipelines/PipelineDIffuseIrradiance.h"
#include "../../include/Pipelines/PipelinePrefilter.h"
#include "../../include/Pipelines/PipelineLUT.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <memory>
#include <iostream>
#include "../../include/Helpers/SettingsLoader.h"
#include "../../include/Helpers/ClusterCalculation.h"
#include <random>
#include "../../include/Postprocess/Postprocess.h"
#include "../../include/Handlers/ComponentsHandler.h"

#include "Graphics/GraphicsEngineD3D11/interface/EngineFactoryD3D11.h"
#include "Graphics/GraphicsEngineD3D12/interface/EngineFactoryD3D12.h"
#include "Graphics/GraphicsEngineOpenGL/interface/EngineFactoryOpenGL.h"
#include "Graphics/GraphicsEngineVulkan/interface/EngineFactoryVk.h"

#include "Graphics/GraphicsEngine/interface/RenderDevice.h"
#include "Graphics/GraphicsEngine/interface/DeviceContext.h"
#include "Graphics/GraphicsEngine/interface/SwapChain.h"

#include "Common/interface/RefCntAutoPtr.hpp"

static const char* VSSource = R"(
struct PSInput 
{ 
    float4 Pos   : SV_POSITION; 
    float3 Color : COLOR; 
};

void main(in  uint    VertId : SV_VertexID,
          out PSInput PSIn) 
{
    float4 Pos[3];
    Pos[0] = float4(-0.5, -0.5, 0.0, 1.0);
    Pos[1] = float4( 0.0, +0.5, 0.0, 1.0);
    Pos[2] = float4(+0.5, -0.5, 0.0, 1.0);

    float3 Col[3];
    Col[0] = float3(1.0, 0.0, 0.0); // red
    Col[1] = float3(0.0, 1.0, 0.0); // green
    Col[2] = float3(0.0, 0.0, 1.0); // blue

    PSIn.Pos   = Pos[VertId];
    PSIn.Color = Col[VertId];
}
)";

// Pixel shader simply outputs interpolated vertex color
static const char* PSSource = R"(
struct PSInput 
{ 
    float4 Pos   : SV_POSITION; 
    float3 Color : COLOR; 
};

struct PSOutput
{ 
    float4 Color : SV_TARGET; 
};

void main(in  PSInput  PSIn,
          out PSOutput PSOut)
{
    PSOut.Color = float4(PSIn.Color.rgb, 1.0);
}
)";

void CreateResources()
{
    // Pipeline state object encompasses configuration of all GPU stages

    Diligent::GraphicsPipelineStateCreateInfo PSOCreateInfo;

    // Pipeline state name is used by the engine to report issues.
    // It is always a good idea to give objects descriptive names.
    PSOCreateInfo.PSODesc.Name = "Simple triangle PSO";

    // This is a graphics pipeline
    PSOCreateInfo.PSODesc.PipelineType = Diligent::PIPELINE_TYPE_GRAPHICS;

    // clang-format off
    // This tutorial will render to a single render target
    PSOCreateInfo.GraphicsPipeline.NumRenderTargets = 1;
    // Set render target format which is the format of the swap chain's color buffer
    PSOCreateInfo.GraphicsPipeline.RTVFormats[0] = Prisma::PrismaFunc::getInstance().contextData().m_pSwapChain->GetDesc().ColorBufferFormat;
    // Use the depth buffer format from the swap chain
    PSOCreateInfo.GraphicsPipeline.DSVFormat = Prisma::PrismaFunc::getInstance().contextData().m_pSwapChain->GetDesc().DepthBufferFormat;
    // Primitive topology defines what kind of primitives will be rendered by this pipeline state
    PSOCreateInfo.GraphicsPipeline.PrimitiveTopology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    // No back face culling for this tutorial
    PSOCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode = Diligent::CULL_MODE_NONE;
    // Disable depth testing
    PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable = Diligent::False;
    // clang-format on

    Diligent::ShaderCreateInfo ShaderCI;
    // Tell the system that the shader source code is in HLSL.
    // For OpenGL, the engine will convert this into GLSL under the hood
    ShaderCI.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_HLSL;
    // OpenGL backend requires emulated combined HLSL texture samplers (g_Texture + g_Texture_sampler combination)
    ShaderCI.Desc.UseCombinedTextureSamplers = true;
    // Create a vertex shader
    Diligent::RefCntAutoPtr<Diligent::IShader> pVS;
    {
        ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_VERTEX;
        ShaderCI.EntryPoint = "main";
        ShaderCI.Desc.Name = "Triangle vertex shader";
        ShaderCI.Source = VSSource;
        Prisma::PrismaFunc::getInstance().contextData().m_pDevice->CreateShader(ShaderCI, &pVS);
    }

    // Create a pixel shader
    Diligent::RefCntAutoPtr<Diligent::IShader> pPS;
    {
        ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_PIXEL;
        ShaderCI.EntryPoint = "main";
        ShaderCI.Desc.Name = "Triangle pixel shader";
        ShaderCI.Source = PSSource;
        Prisma::PrismaFunc::getInstance().contextData().m_pDevice->CreateShader(ShaderCI, &pPS);
    }

    // Finally, create the pipeline state
    PSOCreateInfo.pVS = pVS;
    PSOCreateInfo.pPS = pPS;
    Prisma::PrismaFunc::getInstance().contextData().m_pDevice->CreateGraphicsPipelineState(PSOCreateInfo, &Prisma::PrismaFunc::getInstance().contextData().m_pPSO);
}

Prisma::PipelineForward::PipelineForward(const unsigned int& width, const unsigned int& height, bool srgb) : m_width{
		width
	}, m_height{height}
{
	/*Shader::ShaderHeaders header;
	header.fragment = "#version 460 core\n#extension GL_ARB_bindless_texture : enable\n";

	m_shader = std::make_shared<Shader>("../../../Engine/Shaders/ForwardPipeline/vertex.glsl",
	                                    "../../../Engine/Shaders/ForwardPipeline/fragment.glsl", nullptr, header);

	header.fragment = "#version 460 core\n#extension GL_ARB_bindless_texture : enable\n#define ANIMATE 1\n";

	m_shaderAnimate = std::make_shared<Shader>("../../../Engine/Shaders/AnimationPipeline/vertex_forward.glsl",
	                                           "../../../Engine/Shaders/ForwardPipeline/fragment.glsl", nullptr,
	                                           header);

	m_shaderTransparent = std::make_shared<Shader>("../../../Engine/Shaders/TransparentPipeline/compute.glsl");
	FBO::FBOData fboData;
	fboData.width = m_width;
	fboData.height = m_height;
	fboData.enableDepth = true;
	fboData.internalFormat = GL_RGBA16F;
	fboData.internalType = GL_FLOAT;
	fboData.enableMultisample = true;

	fboData.name = "FORWARD";
	m_fbo = std::make_shared<FBO>(fboData);
	fboData.enableMultisample = false;
	fboData.rbo = false;

	fboData.name = "FORWARD_COPY";
	m_fboCopy = std::make_shared<FBO>(fboData);
	m_shader->use();
	m_fullscreenPipeline = std::make_shared<PipelineFullScreen>();

	m_prepass = std::make_shared<PipelinePrePass>();*/
    CreateResources();
}

void Prisma::PipelineForward::render()
{        // Set render targets before issuing any draw command.
        // Note that Present() unbinds the back buffer if it is set as render target.
    auto* pRTV = Prisma::PrismaFunc::getInstance().contextData().m_pSwapChain->GetCurrentBackBufferRTV();
    auto* pDSV = Prisma::PrismaFunc::getInstance().contextData().m_pSwapChain->GetDepthBufferDSV();
    Prisma::PrismaFunc::getInstance().contextData().m_pImmediateContext->SetRenderTargets(1, &pRTV, pDSV, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    // Clear the back buffer
    const float ClearColor[] = { 0.350f, 0.350f, 0.350f, 1.0f };
    // Let the engine perform required state transitions
    Prisma::PrismaFunc::getInstance().contextData().m_pImmediateContext->ClearRenderTarget(pRTV, ClearColor, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    Prisma::PrismaFunc::getInstance().contextData().m_pImmediateContext->ClearDepthStencil(pDSV, Diligent::CLEAR_DEPTH_FLAG, 1.f, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    // Set the pipeline state in the immediate context
    Prisma::PrismaFunc::getInstance().contextData().m_pImmediateContext->SetPipelineState(Prisma::PrismaFunc::getInstance().contextData().m_pPSO);

    // Typically we should now call CommitShaderResources(), however shaders in this example don't
    // use any resources.

        Diligent::DrawAttribs drawAttrs;
    drawAttrs.NumVertices = 3; // Render 3 vertices
    Prisma::PrismaFunc::getInstance().contextData().m_pImmediateContext->Draw(drawAttrs);
    Prisma::PrismaFunc::getInstance().contextData().m_pSwapChain->Present();
}

Prisma::PipelineForward::~PipelineForward()
{
}
