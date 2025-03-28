#include "../../include/Pipelines/PipelinePrefilter.h"
#include "../../include/GlobalData/GlobalData.h"
#include "../../include/Pipelines/PipelineDIffuseIrradiance.h"
#include "../../include/Helpers/PrismaRender.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "../../include/Helpers/SettingsLoader.h"


Prisma::PipelinePrefilter::PipelinePrefilter()
{
	//m_shader = std::make_shared<Shader>("../../../Engine/Shaders/PrefilterPipeline/vertex.glsl",
	//                                    "../../../Engine/Shaders/PrefilterPipeline/fragment.glsl");
}

void Prisma::PipelinePrefilter::texture(Texture texture)
{
	//if (m_id)
	//{
	//	glMakeTextureHandleNonResidentARB(m_id);
	//	glDeleteTextures(1, &m_prefilterMap);
	//}

	//// pbr: create a pre-filter cubemap, and re-scale capture FBO to pre-filter scale.
	//// --------------------------------------------------------------------------------
	//glGenTextures(1, &m_prefilterMap);
	//glBindTexture(GL_TEXTURE_CUBE_MAP, m_prefilterMap);
	//for (unsigned int i = 0; i < 6; ++i)
	//{
	//	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
	//}
	//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	//// be sure to set minification filter to mip_linear 
	//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//// generate mipmaps for the cubemap so Prisma automatically allocates the required memory.
	//glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	//// pbr: run a quasi monte-carlo simulation on the environment lighting to create a prefilter (cube)map.
	//// ----------------------------------------------------------------------------------------------------
	////m_shader->use();
	////m_shader->setInt64(m_shader->getUniformPosition("environmentMap"), texture.id());
	////m_shader->setInt(m_shader->getUniformPosition("resolution"), texture.data().width);
	////m_shader->setMat4(m_shader->getUniformPosition("projection"), PrismaRender::getInstance().data().captureProjection);

	//glBindFramebuffer(GL_FRAMEBUFFER, PrismaRender::getInstance().data().fbo);
	//glViewport(0, 0, 32, 32); // don't forget to configure the viewport to the capture dimensions.

	//unsigned int maxMipLevels = 8;
	//for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
	//{
	//	// reisze framebuffer according to mip-level size.
	//	unsigned int mipWidth = static_cast<unsigned int>(128 * std::pow(0.5, mip));
	//	unsigned int mipHeight = static_cast<unsigned int>(128 * std::pow(0.5, mip));
	//	glBindRenderbuffer(GL_RENDERBUFFER, PrismaRender::getInstance().data().rbo);
	//	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
	//	glViewport(0, 0, mipWidth, mipHeight);

	//	float roughness = static_cast<float>(mip) / static_cast<float>(maxMipLevels - 1);
	//	m_shader->setFloat(m_shader->getUniformPosition("roughness"), roughness);
	//	for (unsigned int i = 0; i < 6; ++i)
	//	{
	//		m_shader->setMat4(m_shader->getUniformPosition("view"), PrismaRender::getInstance().data().captureViews[i]);
	//		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
	//			m_prefilterMap, mip);

	//		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//		//PrismaRender::getInstance().renderCube();
	//	}
	//}
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//glViewport(0, 0, Prisma::SettingsLoader().getInstance().getSettings().width,
	//           Prisma::SettingsLoader().getInstance().getSettings().height);
	//// don't forget to configure the viewport to the capture dimensions.
	//m_id = glGetTextureHandleARB(m_prefilterMap);
	//glMakeTextureHandleResidentARB(m_id);
}

uint64_t Prisma::PipelinePrefilter::id() const
{
	return m_id;
}
