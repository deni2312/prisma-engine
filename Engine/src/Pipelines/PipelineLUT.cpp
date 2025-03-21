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
	m_shader = std::make_shared<Shader>("../../../Engine/Shaders/LUTPipeline/vertex.glsl",
	                                    "../../../Engine/Shaders/LUTPipeline/fragment.glsl");
}

void Prisma::PipelineLUT::texture()
{
	if (!m_id) {
		// pbr: setup framebuffer
		// ----------------------
		unsigned int width = PrismaRender::getInstance().data().width;
		unsigned int height = PrismaRender::getInstance().data().height;

		// pbr: generate a 2D LUT from the BRDF equations used.
		// ----------------------------------------------------
		unsigned int brdfLUTTexture;
		glGenTextures(1, &brdfLUTTexture);

		// pre-allocate enough memory for the LUT texture.
		glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, width, height, 0, GL_RG, GL_FLOAT, nullptr);
		// be sure to set wrapping mode to GL_CLAMP_TO_EDGE
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// then re-configure capture framebuffer object and render screen-space quad with BRDF shader.
		glBindFramebuffer(GL_FRAMEBUFFER, PrismaRender::getInstance().data().fbo);
		glBindRenderbuffer(GL_RENDERBUFFER, PrismaRender::getInstance().data().rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTexture, 0);

		glViewport(0, 0, PrismaRender::getInstance().data().width, PrismaRender::getInstance().data().height);
		m_shader->use();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		PrismaRender::getInstance().renderQuad();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		m_id = glGetTextureHandleARB(brdfLUTTexture);
		glMakeTextureHandleResidentARB(m_id);
		glViewport(0, 0, Prisma::SettingsLoader().getInstance().getSettings().width,
			Prisma::SettingsLoader().getInstance().getSettings().height);
		Prisma::GlobalData::getInstance().addGlobalTexture({ brdfLUTTexture });
		// don't forget to configure the viewport to the capture dimensions.
	}
}

uint64_t Prisma::PipelineLUT::id() const
{
	return m_id;
}
