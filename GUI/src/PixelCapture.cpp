#include "../include/PixelCapture.h"
#include "../../Engine/include/Helpers/SettingsLoader.h"
#include "../../Engine/include/Helpers/PrismaRender.h"
#include "../../Engine/include/SceneData/MeshIndirect.h"
#include <glm/gtx/string_cast.hpp>
#include "../../Engine/include/GlobalData/GlobalData.h"
#include "../../Engine/include/Handlers/MeshHandler.h"
#include "../../Engine/include/GlobalData/GlobalShaderNames.h"
#include "glm/gtc/type_ptr.hpp"

//static std::shared_ptr<Prisma::Shader> shader = nullptr;
//static std::shared_ptr<Prisma::Shader> shaderAnimation = nullptr;
//static std::shared_ptr<Prisma::Shader> shaderData = nullptr;

Prisma::PixelCapture::PixelCapture()
{
    createDrawPipeline();
}

std::shared_ptr<Prisma::Mesh> Prisma::PixelCapture::capture(glm::vec2 position, const glm::mat4& model)
{
	/*m_fbo->bind();
	GLfloat bkColor[4];
	glGetFloatv(GL_COLOR_CLEAR_VALUE, bkColor);

	float color = bkColor[3];

	bkColor[3] = 0.2;

	glClearColor(bkColor[0], bkColor[1], bkColor[2], bkColor[3]);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	bkColor[3] = color;

	glClearColor(bkColor[0], bkColor[1], bkColor[2], bkColor[3]);

	shader->use();

	MeshIndirect::getInstance().renderMeshesCopy();

	shaderAnimation->use();

	MeshIndirect::getInstance().renderAnimateMeshes();

	m_fbo->unbind();


	m_fboData->bind();
	GLfloat bkColor1[4];
	glGetFloatv(GL_COLOR_CLEAR_VALUE, bkColor1);

	float color1 = bkColor[3];

	bkColor1[3] = 0.2;

	glClearColor(bkColor1[0], bkColor1[1], bkColor1[2], bkColor1[3]);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	bkColor1[3] = color1;

	glClearColor(bkColor1[0], bkColor1[1], bkColor1[2], bkColor1[3]);

	shaderData->use();
	shaderData->setMat4(m_modelLocation, model);
	shaderData->setInt64(m_textureLocation, m_fbo->texture());
	//Prisma::PrismaRender::getInstance().renderQuad();

	glFlush();
	glFinish();

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	unsigned char data[4];

	glReadPixels(position.x, position.y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);

	m_fboData->unbind();


	uint32_t encodedUUID = (data[0] << 16) | (data[1] << 8) | data[2];
	if (data[3] < 0.1)
	{
		if (encodedUUID < Prisma::GlobalData::getInstance().currentGlobalScene()->meshes.size() && encodedUUID >= 0)
		{
			return Prisma::GlobalData::getInstance().currentGlobalScene()->meshes[encodedUUID];
		}
	}
	else if (data[3] < 255 && data[3] > 0)
	{
		return nullptr;
	}
	else
	{
		if (encodedUUID < Prisma::GlobalData::getInstance().currentGlobalScene()->animateMeshes.size() && encodedUUID >=
			0)
		{
			return Prisma::GlobalData::getInstance().currentGlobalScene()->animateMeshes[encodedUUID];
		}
	}
	return nullptr;*/
    auto& contextData = Prisma::PrismaFunc::getInstance().contextData();
    // Clear the back buffer
    contextData.m_pImmediateContext->SetRenderTargets(1, &m_pRTColor, m_pRTDepth, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    const glm::vec4 clear_color(0.0,0,0,1);
    contextData.m_pImmediateContext->ClearRenderTarget(m_pRTColor, glm::value_ptr(clear_color), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    contextData.m_pImmediateContext->ClearDepthStencil(m_pRTDepth, Diligent::CLEAR_DEPTH_FLAG, 1.f, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    // Set the pipeline state
    contextData.m_pImmediateContext->SetPipelineState(m_pso);
    // Commit shader resources. RESOURCE_STATE_TRANSITION_MODE_TRANSITION mode
    // makes sure that resources are transitioned to required states.
    auto& meshes = Prisma::GlobalData::getInstance().currentGlobalScene()->meshes;
    if (!meshes.empty())
    {
        Prisma::MeshIndirect::getInstance().setupBuffers();
        // Set texture SRV in the SRB
        contextData.m_pImmediateContext->CommitShaderResources(m_srb, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        Prisma::MeshIndirect::getInstance().renderMeshes();
    }
    contextData.m_pImmediateContext->SetRenderTargets(0, nullptr, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_NONE);


    Diligent::CopyTextureAttribs CopyAttribs;
    CopyAttribs.pSrcTexture = m_pRTColor->GetTexture();
    CopyAttribs.pDstTexture = m_pStagingTexture;
    CopyAttribs.SrcMipLevel = 0;
    CopyAttribs.DstMipLevel = 0;
    contextData.m_pImmediateContext->CopyTexture(CopyAttribs);

    Diligent::MappedTextureSubresource MappedData;
    contextData.m_pImmediateContext->MapTextureSubresource(
        m_pStagingTexture, 0, 0, Diligent::MAP_READ, Diligent::MAP_FLAG_DO_NOT_WAIT, nullptr, MappedData
    );

    uint8_t* pData = static_cast<uint8_t*>(MappedData.pData);
    uint32_t rowPitch = MappedData.Stride;

    uint32_t pixelOffset = (position.y * rowPitch) + (position.x * 4); // Assuming 4 bytes per pixel (RGBA8)
    uint8_t r = pData[pixelOffset + 0];
    uint8_t g = pData[pixelOffset + 1];
    uint8_t b = pData[pixelOffset + 2];
    uint8_t a = pData[pixelOffset + 3];

    std::cout << "Pixel (" << position.x << ", " << position.y << ") RGBA: ("
        << (int)r << ", " << (int)g << ", " << (int)b << ", " << (int)a << ")\n";

    // 5. Unmap the texture
    contextData.m_pImmediateContext->UnmapTextureSubresource(m_pStagingTexture, 0, 0);

    Prisma::PrismaFunc::getInstance().bindMainRenderTarget();

    return nullptr;
}

void Prisma::PixelCapture::createDrawPipeline()
{

    auto settings = SettingsLoader::getInstance().getSettings();
    auto& contextData = Prisma::PrismaFunc::getInstance().contextData();

    // Pipeline state object encompasses configuration of all GPU stages

    Diligent::GraphicsPipelineStateCreateInfo PSOCreateInfo;

    // Pipeline state name is used by the engine to report issues.
    // It is always a good idea to give objects descriptive names.
    PSOCreateInfo.PSODesc.Name = "ImGui Render";

    // This is a graphics pipeline
    PSOCreateInfo.PSODesc.PipelineType = Diligent::PIPELINE_TYPE_GRAPHICS;

    // clang-format off
    // This tutorial will render to a single render target
    PSOCreateInfo.GraphicsPipeline.NumRenderTargets = 1;
    // Set render target format which is the format of the swap chain's color buffer
    PSOCreateInfo.GraphicsPipeline.RTVFormats[0] = Prisma::PrismaFunc::getInstance().renderFormat().RenderFormat;
    // Set depth buffer format which is the format of the swap chain's back buffer
    PSOCreateInfo.GraphicsPipeline.DSVFormat = Prisma::PrismaFunc::getInstance().renderFormat().DepthBufferFormat;
    // Primitive topology defines what kind of primitives will be rendered by this pipeline state
    PSOCreateInfo.GraphicsPipeline.PrimitiveTopology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    PSOCreateInfo.GraphicsPipeline.RasterizerDesc.FrontCounterClockwise = true;
    // Cull back faces
    PSOCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode = Diligent::CULL_MODE_NONE;
    // Enable depth testing
    PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable = true;
    // clang-format on

    Diligent::ShaderCreateInfo ShaderCI;
    // Tell the system that the shader source code is in HLSL.
    // For OpenGL, the engine will convert this into GLSL under the hood.
    ShaderCI.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_GLSL;

    // OpenGL backend requires emulated combined HLSL texture samplers (g_Texture + g_Texture_sampler combination)
    ShaderCI.Desc.UseCombinedTextureSamplers = true;

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
        ShaderCI.Desc.Name = "ImGui VS";
        ShaderCI.FilePath = "../../../GUI/Shaders/PixelCapture/vertex.glsl";
        contextData.m_pDevice->CreateShader(ShaderCI, &pVS);
    }

    // Create a pixel shader
    Diligent::RefCntAutoPtr<Diligent::IShader> pPS;
    {
        ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_PIXEL;
        ShaderCI.EntryPoint = "main";
        ShaderCI.Desc.Name = "ImGui PS";
        ShaderCI.FilePath = "../../../GUI/Shaders/PixelCapture/fragment.glsl";
        contextData.m_pDevice->CreateShader(ShaderCI, &pPS);
    }

    // clang-format off
    // Define vertex shader input layout
    Diligent::LayoutElement LayoutElems[] =
    {
        // Attribute 0 - vertex position
        Diligent::LayoutElement{0, 0, 3, Diligent::VT_FLOAT32, Diligent::False},
        // Attribute 1 - texture coordinates
        Diligent::LayoutElement{1, 0, 3, Diligent::VT_FLOAT32, Diligent::False},

        Diligent::LayoutElement{2, 0, 2, Diligent::VT_FLOAT32, Diligent::False},

        Diligent::LayoutElement{3, 0, 3, Diligent::VT_FLOAT32, Diligent::False},

        Diligent::LayoutElement{4, 0, 3, Diligent::VT_FLOAT32, Diligent::False}
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
        {Diligent::SHADER_TYPE_VERTEX, ShaderNames::MUTABLE_MODELS.c_str(), Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE}
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
    contextData.m_pDevice->CreateGraphicsPipelineState(PSOCreateInfo, &m_pso);

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
    RTColorDesc.ClearValue.Color[0] = 0;
    RTColorDesc.ClearValue.Color[1] = 0;
    RTColorDesc.ClearValue.Color[2] = 0;
    RTColorDesc.ClearValue.Color[3] = 1;
    Diligent::RefCntAutoPtr<Diligent::ITexture> pRTColor;
    contextData.m_pDevice->CreateTexture(RTColorDesc, nullptr, &pRTColor);

    m_pRTColor = pRTColor->GetDefaultView(Diligent::TEXTURE_VIEW_RENDER_TARGET);

    Diligent::TextureDesc StagingTexDesc = RTColorDesc;
    StagingTexDesc.Usage = Diligent::USAGE_STAGING;
    StagingTexDesc.BindFlags = Diligent::BIND_NONE;
    StagingTexDesc.CPUAccessFlags = Diligent::CPU_ACCESS_READ;

    contextData.m_pDevice->CreateTexture(StagingTexDesc, nullptr, &m_pStagingTexture);

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

    m_pRTDepth = pRTDepth->GetDefaultView(Diligent::TEXTURE_VIEW_DEPTH_STENCIL);

    // Store the depth-stencil view
    m_pso->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, ShaderNames::CONSTANT_VIEW_PROJECTION.c_str())->Set(Prisma::MeshHandler::getInstance().viewProjection());

    m_pso->CreateShaderResourceBinding(&m_srb, true);

    Prisma::MeshIndirect::getInstance().addResizeHandler([&](auto buffer, auto& material)
        {
            m_srb.Release();
            m_pso->CreateShaderResourceBinding(&m_srb, true);
            m_srb->GetVariableByName(Diligent::SHADER_TYPE_VERTEX, ShaderNames::MUTABLE_MODELS.c_str())->Set(Prisma::MeshIndirect::getInstance().modelBuffer()->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE));
        });
    Prisma::GlobalData::getInstance().addGlobalTexture({ pRTColor ,"PixelCapture" });
}
