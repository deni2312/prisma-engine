#include "../../include/Pipelines/PipelineForward.h"

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

#include "../../include/Handlers/LightHandler.h"
#include "../../include/Pipelines/PipelineHandler.h"
#include "../../include/GlobalData/GlobalShaderNames.h"

using namespace Diligent;

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


    auto& contextData = Prisma::PrismaFunc::getInstance().contextData();

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
    PSOCreateInfo.GraphicsPipeline.RTVFormats[0] = Prisma::PrismaFunc::getInstance().renderFormat().RenderFormat;
    // Set depth buffer format which is the format of the swap chain's back buffer
    PSOCreateInfo.GraphicsPipeline.DSVFormat = Prisma::PrismaFunc::getInstance().renderFormat().DepthBufferFormat;
    // Primitive topology defines what kind of primitives will be rendered by this pipeline state
    PSOCreateInfo.GraphicsPipeline.PrimitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    PSOCreateInfo.GraphicsPipeline.RasterizerDesc.FrontCounterClockwise = true;
    // Cull back faces
    PSOCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode = CULL_MODE_BACK;
    // Enable depth testing
    PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable = true;
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
    contextData.m_pEngineFactory->CreateDefaultShaderSourceStreamFactory(nullptr, &pShaderSourceFactory);
    ShaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;
    // Create a vertex shader
    RefCntAutoPtr<IShader> pVS;
    {
        ShaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
        ShaderCI.EntryPoint = "main";
        ShaderCI.Desc.Name = "Cube VS";
        ShaderCI.FilePath = "../../../Engine/Shaders/ForwardPipeline/vertex.hlsl";
        contextData.m_pDevice->CreateShader(ShaderCI, &pVS);
        // Create dynamic uniform buffer that will store our transformation matrix
        // Dynamic buffers can be frequently updated by the CPU
        BufferDesc CBDesc;
        CBDesc.Name = "VS";
        CBDesc.Size = sizeof(ModelNormal);
        CBDesc.Usage = USAGE_DYNAMIC;
        CBDesc.BindFlags = BIND_UNIFORM_BUFFER;
        CBDesc.CPUAccessFlags = CPU_ACCESS_WRITE;
        contextData.m_pDevice->CreateBuffer(CBDesc, nullptr, &m_mvpVS);
    }

    // Create a pixel shader
    RefCntAutoPtr<IShader> pPS;
    {
        ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
        ShaderCI.EntryPoint = "main";
        ShaderCI.Desc.Name = "PS";
        ShaderCI.FilePath = "../../../Engine/Shaders/ForwardPipeline/fragment.hlsl";
        contextData.m_pDevice->CreateShader(ShaderCI, &pPS);
    }

    // clang-format off
    // Define vertex shader input layout
    LayoutElement LayoutElems[] =
    {
        // Attribute 0 - vertex position
        LayoutElement{0, 0, 3, VT_FLOAT32, False},
        // Attribute 1 - texture coordinates
        LayoutElement{1, 0, 3, VT_FLOAT32, False},

        LayoutElement{2, 0, 2, VT_FLOAT32, False},

        LayoutElement{3, 0, 3, VT_FLOAT32, False},

        LayoutElement{4, 0, 3, VT_FLOAT32, False}
    };
    // clang-format on
    PSOCreateInfo.GraphicsPipeline.InputLayout.LayoutElements = LayoutElems;
    PSOCreateInfo.GraphicsPipeline.InputLayout.NumElements = _countof(LayoutElems);

    PSOCreateInfo.pVS = pVS;
    PSOCreateInfo.pPS = pPS;

    // Define variable type that will be used by default
    PSOCreateInfo.PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

    ShaderResourceVariableDesc Vars[] =
    {
        {SHADER_TYPE_PIXEL, Prisma::ShaderNames::CONSTANT_DIFFUSE_TEXTURE.c_str(), SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        {SHADER_TYPE_PIXEL, Prisma::ShaderNames::CONSTANT_NORMAL_TEXTURE.c_str(), SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        {SHADER_TYPE_PIXEL, Prisma::ShaderNames::CONSTANT_ROUGHNESS_METALNESS_TEXTURE.c_str(), SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
    };
    // clang-format on
    PSOCreateInfo.PSODesc.ResourceLayout.Variables = Vars;
    PSOCreateInfo.PSODesc.ResourceLayout.NumVariables = _countof(Vars);

    // clang-format off
	// Define immutable sampler for g_Texture. Immutable samplers should be used whenever possible
    SamplerDesc SamLinearClampDesc
    {
        FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR,
        TEXTURE_ADDRESS_WRAP, TEXTURE_ADDRESS_WRAP, TEXTURE_ADDRESS_WRAP
    };
    ImmutableSamplerDesc ImtblSamplers[] =
    {
        {SHADER_TYPE_PIXEL, Prisma::ShaderNames::CONSTANT_DIFFUSE_TEXTURE.c_str(), SamLinearClampDesc},
        { SHADER_TYPE_PIXEL, Prisma::ShaderNames::CONSTANT_NORMAL_TEXTURE.c_str(), SamLinearClampDesc },
        { SHADER_TYPE_PIXEL, Prisma::ShaderNames::CONSTANT_ROUGHNESS_METALNESS_TEXTURE.c_str(), SamLinearClampDesc }
    };
    // clang-format on
    PSOCreateInfo.PSODesc.ResourceLayout.ImmutableSamplers = ImtblSamplers;
    PSOCreateInfo.PSODesc.ResourceLayout.NumImmutableSamplers = _countof(ImtblSamplers);

    /*const auto& ColorFmtInfo = contextData.m_pDevice->GetTextureFormatInfoExt(contextData.m_pSwapChain->GetDesc().ColorBufferFormat);
    const auto& DepthFmtInfo = contextData.m_pDevice->GetTextureFormatInfoExt(contextData.m_pSwapChain->GetDesc().DepthBufferFormat);
    m_SupportedSampleCounts = ColorFmtInfo.SampleCounts & DepthFmtInfo.SampleCounts;
    if (m_SupportedSampleCounts & SAMPLE_COUNT_4)
        m_SampleCount = 4;
    else if (m_SupportedSampleCounts & SAMPLE_COUNT_2)
        m_SampleCount = 2;
    else
    {
        LOG_WARNING_MESSAGE(ColorFmtInfo.Name, " + ", DepthFmtInfo.Name, " pair does not allow multisampling on this device");
        m_SampleCount = 1;
    }
    PSOCreateInfo.GraphicsPipeline.SmplDesc.Count = m_SampleCount;*/

    contextData.m_pDevice->CreateGraphicsPipelineState(PSOCreateInfo, &m_pso);

    // Since we did not explicitly specify the type for 'Constants' variable, default
    // type (SHADER_RESOURCE_VARIABLE_TYPE_STATIC) will be used. Static variables never
    // change and are bound directly through the pipeline state object.
    m_pso->GetStaticVariableByName(SHADER_TYPE_VERTEX, "Constants")->Set(m_mvpVS);

    m_pso->GetStaticVariableByName(SHADER_TYPE_VERTEX, Prisma::ShaderNames::CONSTANT_VIEW_PROJECTION.c_str())->Set(Prisma::MeshHandler::getInstance().viewProjection());

    m_pso->GetStaticVariableByName(SHADER_TYPE_PIXEL, Prisma::ShaderNames::CONSTANT_VIEW_PROJECTION.c_str())->Set(Prisma::MeshHandler::getInstance().viewProjection());

    m_pso->GetStaticVariableByName(SHADER_TYPE_PIXEL, Prisma::ShaderNames::CONSTANT_OMNI_DATA.c_str())->Set(Prisma::LightHandler::getInstance().omniLights()->GetDefaultView(BUFFER_VIEW_SHADER_RESOURCE));

    m_pso->GetStaticVariableByName(SHADER_TYPE_PIXEL, Prisma::ShaderNames::CONSTANT_LIGHT_SIZES.c_str())->Set(Prisma::LightHandler::getInstance().lightSizes());
    // Create a shader resource binding object and bind all static resources in it

    //CreateMSAARenderTarget();
}

void Prisma::PipelineForward::render(){
    auto& contextData = Prisma::PrismaFunc::getInstance().contextData();

    auto pRTV = Prisma::PipelineHandler::getInstance().textureData().pColorRTV;
    auto pDSV = Prisma::PipelineHandler::getInstance().textureData().pDepthDSV;
    // Clear the back buffer
    contextData.m_pImmediateContext->SetRenderTargets(1, &pRTV, pDSV, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    contextData.m_pImmediateContext->ClearRenderTarget(pRTV, glm::value_ptr(CLEAR_COLOR), RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    contextData.m_pImmediateContext->ClearDepthStencil(pDSV, CLEAR_DEPTH_FLAG, 1.f, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);


    // Set the pipeline state
    contextData.m_pImmediateContext->SetPipelineState(m_pso);
    // Commit shader resources. RESOURCE_STATE_TRANSITION_MODE_TRANSITION mode
    // makes sure that resources are transitioned to required states.
    auto& meshes = Prisma::GlobalData::getInstance().currentGlobalScene()->meshes;
    for (auto mesh : meshes) {
        // Bind vertex and index buffers
        const Uint64 offset = 0;
        IBuffer* pBuffs[] = { mesh->vBuffer() };
        contextData.m_pImmediateContext->SetVertexBuffers(0, 1, pBuffs, &offset, RESOURCE_STATE_TRANSITION_MODE_TRANSITION, SET_VERTEX_BUFFERS_FLAG_RESET);
        contextData.m_pImmediateContext->SetIndexBuffer(mesh->iBuffer(), 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        {
            // Map the buffer and write current world-view-projection matrix
            MapHelper<ModelNormal> CBConstants(contextData.m_pImmediateContext, m_mvpVS, MAP_WRITE, MAP_FLAG_DISCARD);
            CBConstants->model = mesh->parent()->finalMatrix();
            CBConstants->normal = glm::transpose(glm::inverse(mesh->parent()->finalMatrix()));
        }

        // Set texture SRV in the SRB
        contextData.m_pImmediateContext->CommitShaderResources(mesh->material()->srb(), RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        DrawIndexedAttribs DrawAttrs;     // This is an indexed draw call
        DrawAttrs.IndexType = VT_UINT32; // Index type
        DrawAttrs.NumIndices = mesh->verticesData().indices.size();
        // Verify the state of vertex and index buffers
        DrawAttrs.Flags = DRAW_FLAG_VERIFY_ALL;
        contextData.m_pImmediateContext->DrawIndexed(DrawAttrs);
    }
    // Resolve multi-sampled render target into the current swap chain back buffer.
    /*auto pCurrentBackBuffer = contextData.m_pSwapChain->GetCurrentBackBufferRTV()->GetTexture();

    ResolveTextureSubresourceAttribs ResolveAttribs;
    ResolveAttribs.SrcTextureTransitionMode = RESOURCE_STATE_TRANSITION_MODE_TRANSITION;
    ResolveAttribs.DstTextureTransitionMode = RESOURCE_STATE_TRANSITION_MODE_TRANSITION;
    contextData.m_pImmediateContext->ResolveTextureSubresource(m_pMSColorRTV->GetTexture(), pCurrentBackBuffer, ResolveAttribs);*/
    Prisma::PrismaFunc::getInstance().bindMainRenderTarget();
}

Prisma::PipelineForward::~PipelineForward()
{
}

Diligent::RefCntAutoPtr<Diligent::IPipelineState> Prisma::PipelineForward::pso()
{
    return m_pso;
}

void Prisma::PipelineForward::CreateMSAARenderTarget()
{
	if (m_SampleCount == 1)
		return;
    auto& contextData = Prisma::PrismaFunc::getInstance().contextData();

	const auto& SCDesc = contextData.m_pSwapChain->GetDesc();
	// Create window-size multi-sampled offscreen render target
	TextureDesc ColorDesc;
	ColorDesc.Name = "Multisampled render target";
	ColorDesc.Type = RESOURCE_DIM_TEX_2D;
	ColorDesc.BindFlags = BIND_RENDER_TARGET;
	ColorDesc.Width = SCDesc.Width;
	ColorDesc.Height = SCDesc.Height;
	ColorDesc.MipLevels = 1;
	ColorDesc.Format = SCDesc.ColorBufferFormat;
	bool NeedsSRGBConversion = contextData.m_pDevice->GetDeviceInfo().IsD3DDevice() && (ColorDesc.Format == TEX_FORMAT_RGBA8_UNORM_SRGB || ColorDesc.Format == TEX_FORMAT_BGRA8_UNORM_SRGB);
	if (NeedsSRGBConversion)
	{
		// Internally Direct3D swap chain images are not SRGB, and ResolveSubresource
		// requires source and destination formats to match exactly or be typeless.
		// So we will have to create a typeless texture and use SRGB render target view with it.
		ColorDesc.Format = ColorDesc.Format == TEX_FORMAT_RGBA8_UNORM_SRGB ? TEX_FORMAT_RGBA8_TYPELESS : TEX_FORMAT_BGRA8_TYPELESS;
	}

	// Set the desired number of samples
	ColorDesc.SampleCount = m_SampleCount;
	// Define optimal clear value
	ColorDesc.ClearValue.Format = SCDesc.ColorBufferFormat;
	ColorDesc.ClearValue.Color[0] = 0.125f;
	ColorDesc.ClearValue.Color[1] = 0.125f;
	ColorDesc.ClearValue.Color[2] = 0.125f;
	ColorDesc.ClearValue.Color[3] = 1.f;
	RefCntAutoPtr<ITexture> pColor;
    contextData.m_pDevice->CreateTexture(ColorDesc, nullptr, &pColor);

	// Store the render target view
    m_pMSColorRTV.Release();
	if (NeedsSRGBConversion)
	{
		TextureViewDesc RTVDesc;
		RTVDesc.ViewType = TEXTURE_VIEW_RENDER_TARGET;
		RTVDesc.Format = SCDesc.ColorBufferFormat;
		pColor->CreateView(RTVDesc, &m_pMSColorRTV);
	}
	else
	{
		m_pMSColorRTV = pColor->GetDefaultView(TEXTURE_VIEW_RENDER_TARGET);
	}


	// Create window-size multi-sampled depth buffer
	TextureDesc DepthDesc = ColorDesc;
	DepthDesc.Name = "Multisampled depth buffer";
	DepthDesc.Format = contextData.m_pSwapChain->GetDesc().DepthBufferFormat;
	DepthDesc.BindFlags = BIND_DEPTH_STENCIL;
	// Define optimal clear value
	DepthDesc.ClearValue.Format = DepthDesc.Format;
	DepthDesc.ClearValue.DepthStencil.Depth = 1;
	DepthDesc.ClearValue.DepthStencil.Stencil = 0;

	RefCntAutoPtr<ITexture> pDepth;
    contextData.m_pDevice->CreateTexture(DepthDesc, nullptr, &pDepth);
	// Store the depth-stencil view
	m_pMSDepthDSV = pDepth->GetDefaultView(TEXTURE_VIEW_DEPTH_STENCIL);
}
