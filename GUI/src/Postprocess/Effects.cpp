#include "../../include/Postprocess/Effects.h"
#include "../../../Engine/include/Helpers/SettingsLoader.h"
#include "../../include/TextureInfo.h"

Prisma::Effects::Effects()
{
	m_shaderSeppia = std::make_shared<Shader>("../../../GUI/Shaders/Seppia/vertex.glsl",
	                                          "../../../GUI/Shaders/Seppia/fragment.glsl");
	m_shaderSeppia->use();
	m_bindlessPosSeppia = m_shaderSeppia->getUniformPosition("screenTexture");

	m_shaderCartoon = std::make_shared<Shader>("../../../GUI/Shaders/Cartoon/vertex.glsl",
	                                           "../../../GUI/Shaders/Cartoon/fragment.glsl");
	m_shaderCartoon->use();
	m_bindlessPosCartoon = m_shaderCartoon->getUniformPosition("screenTexture");

	m_shaderVignette = std::make_shared<Shader>("../../../GUI/Shaders/Vignette/vertex.glsl",
	                                            "../../../GUI/Shaders/Vignette/fragment.glsl");
	m_shaderVignette->use();
	m_bindlessPosVignette = m_shaderVignette->getUniformPosition("screenTexture");

	m_shaderBloom = std::make_shared<Shader>("../../../GUI/Shaders/Bloom/vertex.glsl",
	                                         "../../../GUI/Shaders/Bloom/fragment.glsl");
	m_shaderBloom->use();
	m_bindlessPosBloom = m_shaderBloom->getUniformPosition("screenTexture");
	m_horizontalPosBloom = m_shaderBloom->getUniformPosition("horizontal");

	m_shaderBrightness = std::make_shared<Shader>("../../../GUI/Shaders/Brightness/vertex.glsl",
	                                              "../../../GUI/Shaders/Brightness/fragment.glsl");
	m_shaderBrightness->use();
	m_bindlessPosBrightness = m_shaderBrightness->getUniformPosition("screenTexture");

	m_shaderVolumetric = std::make_shared<Shader>("../../../GUI/Shaders/Volumetric/vertex.glsl",
	                                              "../../../GUI/Shaders/Volumetric/fragment.glsl");
	m_shaderVolumetric->use();

	m_bindlessPosVolumetric = m_shaderVolumetric->getUniformPosition("screenTexture");

	m_shaderHdr = std::make_shared<Shader>("../../../GUI/Shaders/BloomHdr/vertex.glsl",
	                                       "../../../GUI/Shaders/BloomHdr/fragment.glsl");
	m_shaderHdr->use();
	m_bindlessPosHdr = m_shaderHdr->getUniformPosition("hdrTexture");
	m_bindlessPosHBloom = m_shaderHdr->getUniformPosition("screenTexture");

	unsigned int pingpongColorbuffers[2];
	glGenFramebuffers(2, m_pingpongFBO);
	glGenTextures(2, pingpongColorbuffers);

	auto settings = SettingsLoader::getInstance().getSettings();

	for (unsigned int i = 0; i < 2; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_pingpongFBO[i]);
		glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, settings.width, settings.height, 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		// we clamp to the edge as the blur filter would otherwise sample repeated texture values!
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);
		// also check if framebuffers are complete (no need for depth buffer)
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "Framebuffer not complete!" << std::endl;
		m_bloomTexture[i] = glGetTextureHandleARB(pingpongColorbuffers[i]);
		glMakeTextureHandleResidentARB(m_bloomTexture[i]);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	FBO::FBOData fboData;

	fboData.width = settings.width;
	fboData.height = settings.height;
	fboData.enableDepth = true;
	fboData.enableSrgb = true;

	m_brightnessFbo = std::make_shared<FBO>(fboData);
	TextureInfo::getInstance().add({pingpongColorbuffers[0], "Bloom1"});
	TextureInfo::getInstance().add({pingpongColorbuffers[1], "Bloom2"});
}

void Prisma::Effects::effect(EFFECTS effect)
{
	m_effects = effect;
}

void Prisma::Effects::render(std::shared_ptr<FBO> texture, std::shared_ptr<FBO> raw)
{
	switch (m_effects)
	{
	case EFFECTS::NORMAL:
		break;
	case EFFECTS::SEPPIA:
		glClear(GL_DEPTH_BUFFER_BIT);
		m_shaderSeppia->use();
		m_shaderSeppia->setInt64(m_bindlessPosSeppia, texture->texture());
		PrismaRender::getInstance().renderQuad();
		break;
	case EFFECTS::CARTOON:
		glClear(GL_DEPTH_BUFFER_BIT);
		m_shaderCartoon->use();
		m_shaderCartoon->setInt64(m_bindlessPosCartoon, texture->texture());
		PrismaRender::getInstance().renderQuad();
		break;
	case EFFECTS::VIGNETTE:
		glClear(GL_DEPTH_BUFFER_BIT);
		m_shaderVignette->use();
		m_shaderVignette->setInt64(m_bindlessPosVignette, texture->texture());
		PrismaRender::getInstance().renderQuad();
		break;
	case EFFECTS::VOLUMETRIC:
		glClear(GL_DEPTH_BUFFER_BIT);
		m_shaderVolumetric->use();
		m_shaderVolumetric->setInt64(m_bindlessPosVolumetric, texture->texture());
		PrismaRender::getInstance().renderQuad();
		break;
	case EFFECTS::BLOOM:

		m_brightnessFbo->bind();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		m_shaderBrightness->use();

		m_shaderBrightness->setInt64(m_bindlessPosBrightness, raw->texture());

		PrismaRender::getInstance().renderQuad();

		m_brightnessFbo->unbind();
		m_shaderBloom->use();
		bool horizontal = true, first_iteration = true;
		unsigned int amount = 10;
		for (unsigned int i = 0; i < amount; i++)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, m_pingpongFBO[horizontal]);
			m_shaderBloom->setInt(m_horizontalPosBloom, horizontal);
			m_shaderBloom->setInt64(m_bindlessPosBloom,
			                        first_iteration ? m_brightnessFbo->texture() : m_bloomTexture[!horizontal]);
			PrismaRender::getInstance().renderQuad();
			horizontal = !horizontal;
			if (first_iteration)
				first_iteration = false;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		texture->bind();
		m_shaderHdr->use();
		m_shaderHdr->setInt64(m_bindlessPosHdr, raw->texture());
		m_shaderHdr->setInt64(m_bindlessPosHBloom, m_bloomTexture[!horizontal]);
		PrismaRender::getInstance().renderQuad();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		texture->bind();

		break;
	}
}
