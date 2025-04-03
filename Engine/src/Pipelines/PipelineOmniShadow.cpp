#include "Pipelines/PipelineOmniShadow.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "SceneData/MeshIndirect.h"
#include "GlobalData/GlobalData.h"
#include "GlobalData/GlobalShaderNames.h"
#include "Helpers/SettingsLoader.h"
#include <glm/gtc/type_ptr.hpp>


//static std::shared_ptr<Prisma::Shader> m_shader = nullptr;
//static std::shared_ptr<Prisma::Shader> m_shaderAnimation = nullptr;


Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pso;

static bool initShadow = false;

Prisma::PipelineOmniShadow::PipelineOmniShadow(unsigned int width, unsigned int height, bool post): m_width{width},
	m_height{height}
{
	if (!post)
	{
		//init();
	}
}

void Prisma::PipelineOmniShadow::update(glm::vec3 lightPos)
{
	/*m_shadowProj = glm::perspective(glm::radians(90.0f), static_cast<float>(m_width) / static_cast<float>(m_height),
	                                m_nearPlane, m_farPlane);
	m_shadowTransforms.clear();
	m_shadowTransforms.push_back(m_shadowProj * lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f),
	                                                   glm::vec3(0.0f, -1.0f, 0.0f)));
	m_shadowTransforms.push_back(m_shadowProj * lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f),
	                                                   glm::vec3(0.0f, -1.0f, 0.0f)));
	m_shadowTransforms.push_back(m_shadowProj * lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f),
	                                                   glm::vec3(0.0f, 0.0f, 1.0f)));
	m_shadowTransforms.push_back(m_shadowProj * lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f),
	                                                   glm::vec3(0.0f, 0.0f, -1.0f)));
	m_shadowTransforms.push_back(m_shadowProj * lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f),
	                                                   glm::vec3(0.0f, -1.0f, 0.0f)));
	m_shadowTransforms.push_back(m_shadowProj * lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f),
	                                                   glm::vec3(0.0f, -1.0f, 0.0f)));
	glViewport(0, 0, m_width, m_height);
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	glClear(GL_DEPTH_BUFFER_BIT);
	m_shader->use();
	for (unsigned int i = 0; i < 6; ++i)
	{
		m_shader->setMat4(m_shadowPosition[i], m_shadowTransforms[i]);
	}
	m_shader->setFloat(m_farPlanePos, m_farPlane);
	m_shader->setVec3(m_lightPos, lightPos);

	MeshIndirect::getInstance().renderMeshesCopy();

	m_shaderAnimation->use();

	for (unsigned int i = 0; i < 6; ++i)
	{
		m_shaderAnimation->setMat4(m_shadowPositionAnimation[i], m_shadowTransforms[i]);
	}
	m_shaderAnimation->setFloat(m_farPlanePosAnimation, m_farPlane);
	m_shaderAnimation->setVec3(m_lightPosAnimation, lightPos);

	MeshIndirect::getInstance().renderAnimateMeshes();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, Prisma::SettingsLoader().getInstance().getSettings().width,
	           Prisma::SettingsLoader().getInstance().getSettings().height);*/
	// don't forget to configure the viewport to the capture dimensions.

    m_shadowProj = glm::perspective(glm::radians(90.0f), static_cast<float>(m_width) / static_cast<float>(m_height),
        m_nearPlane, m_farPlane);
    m_shadowTransforms.clear();
    m_shadows.shadows[0]=m_shadowProj * lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, -1.0f, 0.0f));
    m_shadows.shadows[1]=m_shadowProj * lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, -1.0f, 0.0f));
    m_shadows.shadows[2]=m_shadowProj * lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.0f));
    m_shadows.shadows[3]=m_shadowProj * lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, -1.0f));
    m_shadows.shadows[4]=m_shadowProj * lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(0.0f, -1.0f, 0.0f));
    m_shadows.shadows[5]=m_shadowProj * lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f),
        glm::vec3(0.0f, -1.0f, 0.0f));
    auto& contextData = Prisma::PrismaFunc::getInstance().contextData();

    contextData.m_pImmediateContext->UpdateBuffer(m_shadowBuffer, 0, sizeof(OmniShadow), &m_shadows, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    m_lightPlane.far_plane = m_farPlane;
    m_lightPlane.lightPos = lightPos;

    contextData.m_pImmediateContext->UpdateBuffer(m_lightBuffer, 0, sizeof(LightPlane), &m_lightPlane, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    auto color = m_pMSColorRTV->GetDefaultView(Diligent::TEXTURE_VIEW_RENDER_TARGET);
    auto depth = m_depth->GetDefaultView(Diligent::TEXTURE_VIEW_DEPTH_STENCIL);
    // Clear the back buffer
    contextData.m_pImmediateContext->SetRenderTargets(1, &color, depth, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    contextData.m_pImmediateContext->ClearRenderTarget(color, glm::value_ptr(Define::CLEAR_COLOR), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    contextData.m_pImmediateContext->ClearDepthStencil(depth, Diligent::CLEAR_DEPTH_FLAG, 1.f, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

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

    Prisma::PrismaFunc::getInstance().bindMainRenderTarget();
}

uint64_t Prisma::PipelineOmniShadow::id()
{
	return m_id;
}

float Prisma::PipelineOmniShadow::farPlane()
{
	return m_farPlane;
}

void Prisma::PipelineOmniShadow::farPlane(float farPlane)
{
	m_farPlane = farPlane;
}

void Prisma::PipelineOmniShadow::init()
{
	/*if (!m_shader)
	{
		m_shader = std::make_shared<Shader>("../../../Engine/Shaders/OmniShadowPipeline/vertex.glsl",
			"../../../Engine/Shaders/OmniShadowPipeline/fragment.glsl",
			"../../../Engine/Shaders/OmniShadowPipeline/geometry.glsl");
		m_shaderAnimation = std::make_shared<Shader>("../../../Engine/Shaders/AnimationPipeline/vertex_omni.glsl",
			"../../../Engine/Shaders/OmniShadowPipeline/fragment.glsl",
			"../../../Engine/Shaders/OmniShadowPipeline/geometry.glsl");
	}

	glGenFramebuffers(1, &m_fbo);
	// create depth cubemap texture
	unsigned int depthCubemap;
	glGenTextures(1, &depthCubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
	for (unsigned int i = 0; i < 6; ++i)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, m_width, m_height, 0,
			GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	// attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	m_shader->use();
	m_farPlanePos = m_shader->getUniformPosition("far_plane");
	m_lightPos = m_shader->getUniformPosition("lightPos");
	for (unsigned int i = 0; i < 6; ++i)
		m_shadowPosition.push_back(m_shader->getUniformPosition("shadowMatrices[" + std::to_string(i) + "]"));

	m_shaderAnimation->use();
	m_farPlanePosAnimation = m_shaderAnimation->getUniformPosition("far_plane");
	m_lightPosAnimation = m_shaderAnimation->getUniformPosition("lightPos");
	for (unsigned int i = 0; i < 6; ++i)
		m_shadowPositionAnimation.push_back(
			m_shaderAnimation->getUniformPosition("shadowMatrices[" + std::to_string(i) + "]"));
	m_id = glGetTextureHandleARB(depthCubemap);
	glMakeTextureHandleResidentARB(m_id);*/
    auto& contextData = Prisma::PrismaFunc::getInstance().contextData();

    if (!initShadow) {

        // Pipeline state object encompasses configuration of all GPU stages

        Diligent::GraphicsPipelineStateCreateInfo PSOCreateInfo;

        // Pipeline state name is used by the engine to report issues.
        // It is always a good idea to give objects descriptive names.
        PSOCreateInfo.PSODesc.Name = "Omni Shadow Render";

        // This is a graphics pipeline
        PSOCreateInfo.PSODesc.PipelineType = Diligent::PIPELINE_TYPE_GRAPHICS;

        // clang-format off
        // This tutorial will render to a single render target
        PSOCreateInfo.GraphicsPipeline.NumRenderTargets = 1;
        // Set render target format which is the format of the swap chain's color buffer
        PSOCreateInfo.GraphicsPipeline.RTVFormats[0] = Diligent::TEX_FORMAT_RGBA16_FLOAT;
        PSOCreateInfo.GraphicsPipeline.DSVFormat = Prisma::PrismaFunc::getInstance().renderFormat().DepthBufferFormat;
        // Primitive topology defines what kind of primitives will be rendered by this pipeline state
        PSOCreateInfo.GraphicsPipeline.PrimitiveTopology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        PSOCreateInfo.GraphicsPipeline.RasterizerDesc.FrontCounterClockwise = true;
        // Cull back faces
        PSOCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode = Diligent::CULL_MODE_NONE;
        // Enable depth testing
        PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable = true;

        Diligent::ShaderCreateInfo ShaderCI;
        // Tell the system that the shader source code is in HLSL.
        // For OpenGL, the engine will convert this into GLSL under the hood.
        ShaderCI.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_GLSL;


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
            ShaderCI.Desc.Name = "Omni Shadow VS";
            ShaderCI.FilePath = "../../../Engine/Shaders/OmniShadowPipeline/vertex.glsl";
            contextData.m_pDevice->CreateShader(ShaderCI, &pVS);
        }

        Diligent::RefCntAutoPtr<Diligent::IShader> pGS;
        {
            ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_GEOMETRY;
            ShaderCI.EntryPoint = "main";
            ShaderCI.Desc.Name = "Omni Shadow GS";
            ShaderCI.FilePath = "../../../Engine/Shaders/OmniShadowPipeline/geometry.glsl";
            contextData.m_pDevice->CreateShader(ShaderCI, &pGS);
        }

        // Create a pixel shader
        Diligent::RefCntAutoPtr<Diligent::IShader> pPS;
        {
            ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_PIXEL;
            ShaderCI.EntryPoint = "main";
            ShaderCI.Desc.Name = "Omni Shadow PS";
            ShaderCI.FilePath = "../../../Engine/Shaders/OmniShadowPipeline/fragment.glsl";
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
        PSOCreateInfo.pGS = pGS;
        PSOCreateInfo.pPS = pPS;

        // Define variable type that will be used by default
        PSOCreateInfo.PSODesc.ResourceLayout.DefaultVariableType = Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC;


        std::string lightPlane = "LightPlane";
        std::string shadowMatrices = "ShadowMatrices";

        Diligent::ShaderResourceVariableDesc Vars[] =
        {
            {Diligent::SHADER_TYPE_VERTEX, Prisma::ShaderNames::MUTABLE_MODELS.c_str(), Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {Diligent::SHADER_TYPE_GEOMETRY, shadowMatrices.c_str(), Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
            {Diligent::SHADER_TYPE_PIXEL, lightPlane.c_str(), Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        };
        // clang-format on
        PSOCreateInfo.PSODesc.ResourceLayout.Variables = Vars;
        PSOCreateInfo.PSODesc.ResourceLayout.NumVariables = _countof(Vars);


        Diligent::BufferDesc LightBuffer;
        LightBuffer.Name = "LightBuffer";
        LightBuffer.Usage = Diligent::USAGE_DEFAULT;
        LightBuffer.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
        LightBuffer.Mode = Diligent::BUFFER_MODE_STRUCTURED;
        LightBuffer.ElementByteStride = sizeof(LightPlane);
        LightBuffer.Size = sizeof(LightPlane);
        contextData.m_pDevice->CreateBuffer(LightBuffer, nullptr, &m_lightBuffer);
        
        Diligent::BufferDesc ShadowBuffer;
        ShadowBuffer.Name = "ShadowBuffer";
        ShadowBuffer.Usage = Diligent::USAGE_DEFAULT;
        ShadowBuffer.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
        ShadowBuffer.Mode = Diligent::BUFFER_MODE_STRUCTURED;
        ShadowBuffer.Size = sizeof(OmniShadow);
        ShadowBuffer.ElementByteStride = sizeof(OmniShadow);
        contextData.m_pDevice->CreateBuffer(ShadowBuffer, nullptr, &m_shadowBuffer);

        contextData.m_pDevice->CreateGraphicsPipelineState(PSOCreateInfo, &m_pso);
        m_pso->GetStaticVariableByName(Diligent::SHADER_TYPE_GEOMETRY, shadowMatrices.c_str())->Set(m_shadowBuffer);
        m_pso->GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, lightPlane.c_str())->Set(m_lightBuffer);

        initShadow = true;
    }

    m_pso->CreateShaderResourceBinding(&m_srb, true);

    Prisma::MeshIndirect::getInstance().addResizeHandler([&](Diligent::RefCntAutoPtr<Diligent::IBuffer> buffers, Prisma::MeshIndirect::MaterialView& materials)
        {
            m_srb.Release();
            m_pso->CreateShaderResourceBinding(&m_srb, true);
            m_srb->GetVariableByName(Diligent::SHADER_TYPE_VERTEX, Prisma::ShaderNames::MUTABLE_MODELS.c_str())->Set(buffers->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE));
        });

    Diligent::TextureDesc RTColorDesc;
    RTColorDesc.Name = "Offscreen render target";
    RTColorDesc.Type = Diligent::RESOURCE_DIM_TEX_CUBE;  // Set cubemap type
    RTColorDesc.Width = m_width;
    RTColorDesc.Height = m_height;
    RTColorDesc.MipLevels = 1;
    RTColorDesc.Format = Diligent::TEX_FORMAT_RGBA16_FLOAT;
    // Specify 6 faces for cubemap
    RTColorDesc.ArraySize = 6;
    // Allow it to be used as both a shader resource and a render target
    RTColorDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE | Diligent::BIND_RENDER_TARGET;
    // Define optimal clear value
    RTColorDesc.ClearValue.Format = RTColorDesc.Format;
    // Create the cubemap texture
    contextData.m_pDevice->CreateTexture(RTColorDesc, nullptr, &m_pMSColorRTV);

    // Create window-size depth buffer
    Diligent::TextureDesc RTDepthDesc = RTColorDesc;
    RTDepthDesc.Name = "Offscreen depth buffer";
    RTDepthDesc.Format = Prisma::PrismaFunc::getInstance().renderFormat().DepthBufferFormat;
    RTDepthDesc.BindFlags |= Diligent::BIND_DEPTH_STENCIL;
    // Define optimal clear value
    RTDepthDesc.ClearValue.Format = RTDepthDesc.Format;
    RTDepthDesc.ClearValue.DepthStencil.Depth = 1;
    RTDepthDesc.ClearValue.DepthStencil.Stencil = 0;
    contextData.m_pDevice->CreateTexture(RTDepthDesc, nullptr, &m_depth);

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

        Diligent::TextureViewDesc DepthDesc;
        DepthDesc.ViewType = Diligent::TEXTURE_VIEW_DEPTH_STENCIL;
        DepthDesc.TextureDim = Diligent::RESOURCE_DIM_TEX_2D_ARRAY;
        DepthDesc.MostDetailedMip = 0;
        DepthDesc.NumMipLevels = 1;
        DepthDesc.FirstArraySlice = i;  // Select the specific face
        DepthDesc.NumArraySlices = 1;
        m_depth->CreateView(DepthDesc, &m_depthView[i]);

        m_pMSColorRTV->CreateView(RTVDesc, &m_pRTColor[i]);
    }
}

float Prisma::PipelineOmniShadow::nearPlane()
{
	return m_nearPlane;
}

void Prisma::PipelineOmniShadow::nearPlane(float nearPlane)
{
	m_nearPlane = nearPlane;
}
