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
#include "Graphics/GraphicsTools/interface/GraphicsUtilities.h"
#include <Graphics/GraphicsTools/interface/MapHelper.hpp>

using namespace Diligent;

RefCntAutoPtr<IBuffer> m_CubeVertexBuffer;
RefCntAutoPtr<IBuffer> m_CubeIndexBuffer;

void CreateVertexBuffer()
{
    // Layout of this structure matches the one we defined in the pipeline state
    struct Vertex
    {
        glm::vec3 pos;
        glm::vec4 color;
    };

    // Cube vertices

    //      (-1,+1,+1)________________(+1,+1,+1)
    //               /|              /|
    //              / |             / |
    //             /  |            /  |
    //            /   |           /   |
    //(-1,-1,+1) /____|__________/(+1,-1,+1)
    //           |    |__________|____|
    //           |   /(-1,+1,-1) |    /(+1,+1,-1)
    //           |  /            |   /
    //           | /             |  /
    //           |/              | /
    //           /_______________|/
    //        (-1,-1,-1)       (+1,-1,-1)
    //

    constexpr Vertex CubeVerts[8] =
    {
        {{-1, -1, -1},{1, 0, 0, 1}},
        {{-1, +1, -1}, {0, 1, 0, 1}},
        {{+1, +1, -1}, {0, 0, 1, 1}},
        {{+1, -1, -1}, {1, 1, 1, 1}},

        {{-1, -1, +1}, {1, 1, 0, 1}},
        {{-1, +1, +1}, {0, 1, 1, 1}},
        {{+1, +1, +1}, {1, 0, 1, 1}},
        {{+1, -1, +1}, {0.2f, 0.2f, 0.2f, 1.f}},
    };
    // Create a vertex buffer that stores cube vertices
    BufferDesc VertBuffDesc;
    VertBuffDesc.Name = "Cube vertex buffer";
    VertBuffDesc.Usage = USAGE_IMMUTABLE;
    VertBuffDesc.BindFlags = BIND_VERTEX_BUFFER;
    VertBuffDesc.Size = sizeof(CubeVerts);
    BufferData VBData;
    VBData.pData = CubeVerts;
    VBData.DataSize = sizeof(CubeVerts);
    Prisma::PrismaFunc::getInstance().contextData().m_pDevice->CreateBuffer(VertBuffDesc, &VBData, &m_CubeVertexBuffer);
}

void CreateIndexBuffer()
{
    // clang-format off
    constexpr Uint32 Indices[] =
    {
        2,0,1, 2,3,0,
        4,6,5, 4,7,6,
        0,7,4, 0,3,7,
        1,0,4, 1,4,5,
        1,5,2, 5,6,2,
        3,6,7, 3,2,6
    };
    // clang-format on

    BufferDesc IndBuffDesc;
    IndBuffDesc.Name = "Cube index buffer";
    IndBuffDesc.Usage = USAGE_IMMUTABLE;
    IndBuffDesc.BindFlags = BIND_INDEX_BUFFER;
    IndBuffDesc.Size = sizeof(Indices);
    BufferData IBData;
    IBData.pData = Indices;
    IBData.DataSize = sizeof(Indices);
    Prisma::PrismaFunc::getInstance().contextData().m_pDevice->CreateBuffer(IndBuffDesc, &IBData, &m_CubeIndexBuffer);
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

    // Pipeline state object encompasses configuration of all GPU stages


    // Pipeline state object encompasses configuration of all GPU stages

    GraphicsPipelineStateCreateInfo PSOCreateInfo;

    // Pipeline state name is used by the engine to report issues.
    // It is always a good idea to give objects descriptive names.
    PSOCreateInfo.PSODesc.Name = "Cube PSO";

    // This is a graphics pipeline
    PSOCreateInfo.PSODesc.PipelineType = PIPELINE_TYPE_GRAPHICS;

    // clang-format off
    // This tutorial will render to a single render target
    PSOCreateInfo.GraphicsPipeline.NumRenderTargets = 1;
    // Set render target format which is the format of the swap chain's color buffer
    PSOCreateInfo.GraphicsPipeline.RTVFormats[0] = Prisma::PrismaFunc::getInstance().contextData().m_pSwapChain->GetDesc().ColorBufferFormat;
    // Set depth buffer format which is the format of the swap chain's back buffer
    PSOCreateInfo.GraphicsPipeline.DSVFormat = Prisma::PrismaFunc::getInstance().contextData().m_pSwapChain->GetDesc().DepthBufferFormat;
    // Primitive topology defines what kind of primitives will be rendered by this pipeline state
    PSOCreateInfo.GraphicsPipeline.PrimitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    // Cull back faces
    PSOCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode = CULL_MODE_BACK;
    // Enable depth testing
    PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable = True;
    // clang-format on

    ShaderCreateInfo ShaderCI;
    // Tell the system that the shader source code is in HLSL.
    // For OpenGL, the engine will convert this into GLSL under the hood.
    ShaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;

    // OpenGL backend requires emulated combined HLSL texture samplers (g_Texture + g_Texture_sampler combination)
    ShaderCI.Desc.UseCombinedTextureSamplers = true;

    // Pack matrices in row-major order
    ShaderCI.CompileFlags = SHADER_COMPILE_FLAG_PACK_MATRIX_ROW_MAJOR;

    // Presentation engine always expects input in gamma space. Normally, pixel shader output is
    // converted from linear to gamma space by the GPU. However, some platforms (e.g. Android in GLES mode,
    // or Emscripten in WebGL mode) do not support gamma-correction. In this case the application
    // has to do the conversion manually.
    bool convert = false;

    ShaderMacro Macros[] = { {"CONVERT_PS_OUTPUT_TO_GAMMA",convert ? "1" : "0"} };
    ShaderCI.Macros = { Macros, _countof(Macros) };

    // In this tutorial, we will load shaders from file. To be able to do that,
    // we need to create a shader source stream factory
    RefCntAutoPtr<IShaderSourceInputStreamFactory> pShaderSourceFactory;
    Prisma::PrismaFunc::getInstance().contextData().m_pEngineFactory->CreateDefaultShaderSourceStreamFactory(nullptr, &pShaderSourceFactory);
    ShaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;
    // Create a vertex shader
    RefCntAutoPtr<IShader> pVS;
    {
        ShaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
        ShaderCI.EntryPoint = "main";
        ShaderCI.Desc.Name = "Cube VS";
        ShaderCI.FilePath = "../../../Engine/Shaders/ForwardPipeline/vertex.hlsl";
        Prisma::PrismaFunc::getInstance().contextData().m_pDevice->CreateShader(ShaderCI, &pVS);
        // Create dynamic uniform buffer that will store our transformation matrix
        // Dynamic buffers can be frequently updated by the CPU
        BufferDesc CBDesc;
        CBDesc.Name = "VS constants CB";
        CBDesc.Size = sizeof(glm::mat4);
        CBDesc.Usage = USAGE_DYNAMIC;
        CBDesc.BindFlags = BIND_UNIFORM_BUFFER;
        CBDesc.CPUAccessFlags = CPU_ACCESS_WRITE;
        Prisma::PrismaFunc::getInstance().contextData().m_pDevice->CreateBuffer(CBDesc, nullptr, &m_mvpVS);
    }

    // Create a pixel shader
    RefCntAutoPtr<IShader> pPS;
    {
        ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
        ShaderCI.EntryPoint = "main";
        ShaderCI.Desc.Name = "Cube PS";
        ShaderCI.FilePath = "../../../Engine/Shaders/ForwardPipeline/fragment.hlsl";
        Prisma::PrismaFunc::getInstance().contextData().m_pDevice->CreateShader(ShaderCI, &pPS);
    }

    // clang-format off
    // Define vertex shader input layout
    LayoutElement LayoutElems[] =
    {
        // Attribute 0 - vertex position
        LayoutElement{0, 0, 3, VT_FLOAT32, False},
        // Attribute 1 - vertex color
        LayoutElement{1, 0, 4, VT_FLOAT32, False}
    };
    // clang-format on
    PSOCreateInfo.GraphicsPipeline.InputLayout.LayoutElements = LayoutElems;
    PSOCreateInfo.GraphicsPipeline.InputLayout.NumElements = _countof(LayoutElems);

    PSOCreateInfo.pVS = pVS;
    PSOCreateInfo.pPS = pPS;

    // Define variable type that will be used by default
    PSOCreateInfo.PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

    Prisma::PrismaFunc::getInstance().contextData().m_pDevice->CreateGraphicsPipelineState(PSOCreateInfo, &m_pso);

    // Since we did not explicitly specify the type for 'Constants' variable, default
    // type (SHADER_RESOURCE_VARIABLE_TYPE_STATIC) will be used. Static variables never
    // change and are bound directly through the pipeline state object.
    m_pso->GetStaticVariableByName(SHADER_TYPE_VERTEX, "Constants")->Set(m_mvpVS);

    // Create a shader resource binding object and bind all static resources in it
    m_pso->CreateShaderResourceBinding(&m_shader, true);
    CreateVertexBuffer();
    CreateIndexBuffer();
}

void Prisma::PipelineForward::render(){

	auto* pRTV = Prisma::PrismaFunc::getInstance().contextData().m_pSwapChain->GetCurrentBackBufferRTV();
	auto* pDSV = Prisma::PrismaFunc::getInstance().contextData().m_pSwapChain->GetDepthBufferDSV();
	// Clear the back buffer
	glm::vec4 ClearColor = { 0.350f, 0.350f, 0.350f, 1.0f };
    Prisma::PrismaFunc::getInstance().contextData().m_pImmediateContext->SetRenderTargets(1, &pRTV, pDSV, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    Prisma::PrismaFunc::getInstance().contextData().m_pImmediateContext->ClearRenderTarget(pRTV, glm::value_ptr(ClearColor), RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    Prisma::PrismaFunc::getInstance().contextData().m_pImmediateContext->ClearDepthStencil(pDSV, CLEAR_DEPTH_FLAG, 1.f, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

	{
	    // Map the buffer and write current world-view-projection matrix
	    MapHelper<glm::mat4> CBConstants(Prisma::PrismaFunc::getInstance().contextData().m_pImmediateContext, m_mvpVS, MAP_WRITE, MAP_FLAG_DISCARD);
	    *CBConstants = Prisma::GlobalData::getInstance().currentProjection()*glm::translate(glm::mat4(1.0),glm::vec3(0,0,-10));
	}

	// Bind vertex and index buffers
	const Uint64 offset = 0;
	IBuffer* pBuffs[] = { m_CubeVertexBuffer };
    Prisma::PrismaFunc::getInstance().contextData().m_pImmediateContext->SetVertexBuffers(0, 1, pBuffs, &offset, RESOURCE_STATE_TRANSITION_MODE_TRANSITION, SET_VERTEX_BUFFERS_FLAG_RESET);
    Prisma::PrismaFunc::getInstance().contextData().m_pImmediateContext->SetIndexBuffer(m_CubeIndexBuffer, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

	// Set the pipeline state
    Prisma::PrismaFunc::getInstance().contextData().m_pImmediateContext->SetPipelineState(m_pso);
	// Commit shader resources. RESOURCE_STATE_TRANSITION_MODE_TRANSITION mode
	// makes sure that resources are transitioned to required states.
    Prisma::PrismaFunc::getInstance().contextData().m_pImmediateContext->CommitShaderResources(m_shader, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

	DrawIndexedAttribs DrawAttrs;     // This is an indexed draw call
	DrawAttrs.IndexType = VT_UINT32; // Index type
	DrawAttrs.NumIndices = 36;
	// Verify the state of vertex and index buffers
	DrawAttrs.Flags = DRAW_FLAG_VERIFY_ALL;
    Prisma::PrismaFunc::getInstance().contextData().m_pImmediateContext->DrawIndexed(DrawAttrs);
    Prisma::PrismaFunc::getInstance().contextData().m_pSwapChain->Present();
}

Prisma::PipelineForward::~PipelineForward()
{
}
