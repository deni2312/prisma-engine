#include "Pipelines/PipelineOmniShadow.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "SceneData/MeshIndirect.h"
#include "GlobalData/GlobalData.h"
#include "GlobalData/GlobalShaderNames.h"
#include "Helpers/SettingsLoader.h"
#include <glm/gtc/type_ptr.hpp>

#include "Handlers/OmniShadowHandler.h"


Prisma::PipelineOmniShadow::PipelineOmniShadow(unsigned int width, unsigned int height, bool post): m_width{width},
                                                                                                    m_height{height}
{
	if (!post)
	{
		init();
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
    Prisma::OmniShadowHandler::getInstance().render({ m_color,m_depth,m_nearPlane,m_farPlane,m_width,m_height,lightPos });
}

Diligent::RefCntAutoPtr<Diligent::ITexture> Prisma::PipelineOmniShadow::shadowTexture()
{
    return m_depth;
}

float Prisma::PipelineOmniShadow::farPlane()
{
	return m_farPlane;
}

void Prisma::PipelineOmniShadow::farPlane(float farPlane)
{
	m_farPlane = farPlane;
    CacheScene::getInstance().updateStatus(true);
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
    contextData.m_pDevice->CreateTexture(RTColorDesc, nullptr, &m_color);

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
        Diligent::RefCntAutoPtr<Diligent::ITextureView> depth;
        m_depth->CreateView(DepthDesc, &depth);
        Diligent::RefCntAutoPtr<Diligent::ITextureView> color;
        m_color->CreateView(RTVDesc, &color);
    }
}

float Prisma::PipelineOmniShadow::nearPlane()
{
	return m_nearPlane;
}

void Prisma::PipelineOmniShadow::nearPlane(float nearPlane)
{
	m_nearPlane = nearPlane;
    CacheScene::getInstance().updateStatus(true);
}
