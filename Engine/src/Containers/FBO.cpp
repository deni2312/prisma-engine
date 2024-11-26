#include "../../include/Containers/FBO.h"
#include "../../include/Helpers/GarbageCollector.h"
#include <iostream>
#include "../../../GUI/include/TextureInfo.h"

Prisma::FBO::FBO(FBOData fboData)
	: m_vao{0}, m_fboData{fboData}
{
	glGenFramebuffers(1, &m_framebufferID);
	glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferID);

	// Create a color attachment texture
	unsigned int textureID;
	glGenTextures(1, &textureID);

	if (m_fboData.enableMultisample)
	{
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureID);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, m_fboData.internalFormat, m_fboData.width,
		                        m_fboData.height, GL_TRUE);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, textureID, 0);
		// create a (also multisampled) renderbuffer object for depth and stencil attachments
		unsigned int rbo;
		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT, m_fboData.width, m_fboData.height);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
	}
	else
	{
		if (m_fboData.enableColor)
		{
			glBindTexture(GL_TEXTURE_2D, textureID);

			glTexImage2D(GL_TEXTURE_2D, 0, m_fboData.internalFormat, fboData.width, fboData.height, 0, GL_RGBA,
			             m_fboData.internalType, nullptr);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_fboData.filtering);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_fboData.filtering);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_fboData.border);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_fboData.border);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID, 0);
		}

		if (m_fboData.enableDepth)
		{
			if (m_fboData.rbo)
			{
				unsigned int rbo;
				glGenRenderbuffers(1, &rbo);
				glBindRenderbuffer(GL_RENDERBUFFER, rbo);
				glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_fboData.width, m_fboData.height);
				glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
				GarbageCollector::getInstance().add({GarbageCollector::GarbageType::RBO, rbo});
			}
			else
			{
				unsigned int depthTexture;

				// Generate and configure the depth texture
				glGenTextures(1, &depthTexture);
				glBindTexture(GL_TEXTURE_2D, depthTexture);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_fboData.width, m_fboData.height, 0,
				             GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

				m_depthId = glGetTextureHandleARB(depthTexture);
				glMakeTextureHandleResidentARB(m_depthId);
			}
		}

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			Prisma::Logger::getInstance().log(Prisma::LogLevel::ERROR,
			                                  "Frame buffer not created correctly.");
	}

	m_id = glGetTextureHandleARB(textureID);
	glMakeTextureHandleResidentARB(m_id);
	GarbageCollector::getInstance().add({GarbageCollector::GarbageType::FBO, m_framebufferID});
	GarbageCollector::getInstance().addTexture({textureID, m_id});
	TextureInfo::getInstance().add({textureID, m_fboData.name});

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Prisma::FBO::FBO(std::vector<FBOData> fboData)
{
	glGenFramebuffers(1, &m_framebufferID);
	glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferID);


	std::vector<unsigned int> textures;
	for (size_t i = 0; i < fboData.size(); ++i)
	{
		auto currentFboData = fboData[i];

		// Generate and configure the texture
		unsigned int textureID;
		glGenTextures(1, &textureID);

		if (currentFboData.enableMultisample)
		{
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureID);
			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, currentFboData.internalFormat, currentFboData.width,
			                        currentFboData.height, GL_TRUE);
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D_MULTISAMPLE, textureID, 0);
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, textureID);
			glTexImage2D(GL_TEXTURE_2D, 0, currentFboData.internalFormat, currentFboData.width, currentFboData.height,
			             0, GL_RGBA,
			             currentFboData.internalType, nullptr);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, currentFboData.filtering);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, currentFboData.filtering);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, currentFboData.border);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, currentFboData.border);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, textureID, 0);
		}

		// Store texture handle for each attachment
		GLuint64 handle = glGetTextureHandleARB(textureID);
		glMakeTextureHandleResidentARB(handle);

		m_textureId.push_back(handle);

		// Register texture with GarbageCollector and TextureInfo
		GarbageCollector::getInstance().addTexture({textureID, handle});
		TextureInfo::getInstance().add({textureID, currentFboData.name});
		textures.push_back(GL_COLOR_ATTACHMENT0 + i);
	}

	glDrawBuffers(textures.size(), textures.data());

	// Optionally configure depth or stencil renderbuffers
	if (!fboData.empty() && fboData[0].enableDepth)
	{
		unsigned int depthTexture;
		glGenTextures(1, &depthTexture);
		glBindTexture(GL_TEXTURE_2D, depthTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, fboData[0].width, fboData[0].height, 0,
		             GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

		m_depthId = glGetTextureHandleARB(depthTexture);
		glMakeTextureHandleResidentARB(m_depthId);

		GarbageCollector::getInstance().add({GarbageCollector::GarbageType::TEXTURE, depthTexture});
	}

	// Check framebuffer completeness
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		Prisma::Logger::getInstance().log(Prisma::LogLevel::ERROR,
		                                  "Frame buffer not created correctly.");

	// Unbind framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Register framebuffer with GarbageCollector
	GarbageCollector::getInstance().add({GarbageCollector::GarbageType::FBO, m_framebufferID});
}

Prisma::FBO::~FBO()
{
}

void Prisma::FBO::bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferID);
}

void Prisma::FBO::unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

uint64_t Prisma::FBO::texture() const
{
	return m_id;
}

std::vector<uint64_t> Prisma::FBO::textures() const
{
	return m_textureId;
}

uint64_t Prisma::FBO::depth() const
{
	return m_depthId;
}

unsigned int Prisma::FBO::frameBufferID()
{
	return m_framebufferID;
}
