#include "../../include/Pipelines/PipelineSkybox.h"
#include "../../include/GlobalData/GlobalData.h"
#include "../../include/Pipelines/PipelineDIffuseIrradiance.h"
#include "../../include/Pipelines/PipelinePrefilter.h"
#include "../../include/Pipelines/PipelineLUT.h"
#include "../../include/Helpers/PrismaRender.h"
#include "../../include/Helpers/SettingsLoader.h"


#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

Prisma::PipelineSkybox::PipelineSkybox() : m_height{512},m_width{512}
{
	m_shader = std::make_shared<Shader>("../../../Engine/Shaders/SkyboxPipeline/vertex.glsl",
	                                    "../../../Engine/Shaders/SkyboxPipeline/fragment.glsl");
	m_shaderEquirectangular = std::make_shared<Shader>("../../../Engine/Shaders/EquirectangularPipeline/vertex.glsl",
	                                                   "../../../Engine/Shaders/EquirectangularPipeline/fragment.glsl");
	m_bindlessPos = m_shader->getUniformPosition("skybox");
	m_bindlessPosEquirectangular = m_shaderEquirectangular->getUniformPosition("equirectangularMap");
}

const Prisma::Texture& Prisma::PipelineSkybox::texture() const
{
	return m_texture;
}

void Prisma::PipelineSkybox::calculateSkybox()
{
	if (m_id)
	{
		glMakeTextureHandleNonResidentARB(m_id);
		glDeleteTextures(1, &m_envCubemap);
	}
	glGenTextures(1, &m_envCubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_envCubemap);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, m_width, m_height, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// pbr: set up projection and view matrices for capturing data onto the 6 cubemap face directions
	// ----------------------------------------------------------------------------------------------

	m_shaderEquirectangular->use();
	m_shaderEquirectangular->setInt64(m_bindlessPosEquirectangular, m_texture.id());
	m_shaderEquirectangular->setMat4(m_shaderEquirectangular->getUniformPosition("projection"),
	                                 PrismaRender::getInstance().data().captureProjection);


	auto posView = m_shaderEquirectangular->getUniformPosition("view");
	glViewport(0, 0, m_width, m_height); // don't forget to configure the viewport to the capture dimensions.
	glBindFramebuffer(GL_FRAMEBUFFER, PrismaRender::getInstance().data().fbo);
	glBindRenderbuffer(GL_RENDERBUFFER, PrismaRender::getInstance().data().rbo);
	for (unsigned int i = 0; i < 6; ++i)
	{
		m_shaderEquirectangular->setMat4(posView, PrismaRender::getInstance().data().captureViews[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_envCubemap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//PrismaRender::getInstance().renderCube();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, Prisma::SettingsLoader().getInstance().getSettings().width,
	           Prisma::SettingsLoader().getInstance().getSettings().height);
	// don't forget to configure the viewport to the capture dimensions.

	glBindTexture(GL_TEXTURE_CUBE_MAP, m_envCubemap);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	m_id = glGetTextureHandleARB(m_envCubemap);
	glMakeTextureHandleResidentARB(m_id);
}

void Prisma::PipelineSkybox::render()
{
	glDepthFunc(GL_LEQUAL);
	m_shader->use();
	m_shader->setInt64(m_bindlessPos, m_id);
	//PrismaRender::getInstance().renderCube();
	glBindVertexArray(0);
	glDepthFunc(GL_LESS);
}

void Prisma::PipelineSkybox::texture(Texture texture, bool equirectangular)
{
	m_texture = texture;
	m_equirectangular = equirectangular;
	if (m_equirectangular)
	{
		//PrismaRender::getInstance().createFbo(texture.data().width, texture.data().height);
		m_height = texture.data().height;
		m_width = texture.data().width;
		calculateSkybox();
		Texture textureIrradiance;
		textureIrradiance.data(texture.data());
		textureIrradiance.id(m_id);
		PipelineDiffuseIrradiance::getInstance().texture(textureIrradiance);
		PipelinePrefilter::getInstance().texture(textureIrradiance);
		PipelineLUT::getInstance().texture();
	}
}
