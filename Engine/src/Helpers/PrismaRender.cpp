#include "../../include/Helpers/PrismaRender.h"
#include "../../include/GlobalData/GlobalData.h"
#include "../../include/Pipelines/PipelineDIffuseIrradiance.h"
#include "../../include/Pipelines/PipelinePrefilter.h"
#include "../../include/Pipelines/PipelineLUT.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "../../include/Helpers/GarbageCollector.h"

Prisma::PrismaRender::PrismaRender()
{
}

void Prisma::PrismaRender::renderCube()
{
	// initialize (if necessary)
	if (m_vaoCube == nullptr)
	{
		float vertices[] = {
			// back face
			-1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, // bottom-right
			1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, // top-right
			1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, // top-right
			-1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, // top-left
			-1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			// front face
			-1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
			1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // top-right
			1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, // bottom-right
			1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // top-right
			-1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
			-1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, // top-left
			// left face
			-1.0f, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-right
			-1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // bottom-left
			-1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-left
			-1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-left
			-1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // top-right
			-1.0f, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-right
			// right face
			1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // bottom-left
			1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // bottom-right
			1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // top-right
			1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // top-right
			1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // top-left
			1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // bottom-left
			// bottom face
			-1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
			1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // bottom-left
			1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f, // top-left
			1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // bottom-left
			-1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
			-1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, // bottom-right
			// top face
			-1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top-right
			1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, // top-left
			1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-left
			1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-left
			-1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f // top-left
		};

		m_vaoCube = std::make_shared<VAO>();
		m_vaoCube->bind();
		m_vboCube = std::make_shared<VBO>();
		m_vboCube->writeData(sizeof(vertices), vertices);
		// link vertex attributes
		m_vaoCube->addAttribPointer(0, 3, 8 * sizeof(float), nullptr);
		m_vaoCube->addAttribPointer(1, 3, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		m_vaoCube->addAttribPointer(2, 2, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		VAO::resetVao();
	}
	// render Cube
	m_vaoCube->bind();
	glDrawArrays(GL_TRIANGLES, 0, 36);
	VAO::resetVao();
}

void Prisma::PrismaRender::renderQuad()
{
	if (m_vaoQuad == nullptr)
	{
		float quadVertices[] = {
			// vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
			// positions   // texCoords
			-1.0f, 1.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f,
			1.0f, -1.0f, 1.0f, 0.0f,

			-1.0f, 1.0f, 0.0f, 1.0f,
			1.0f, -1.0f, 1.0f, 0.0f,
			1.0f, 1.0f, 1.0f, 1.0f
		};
		// setup plane VAO
		m_vaoQuad = std::make_shared<VAO>();
		m_vaoQuad->bind();
		m_vboQuad = std::make_shared<VBO>();
		m_vboQuad->writeData(sizeof(quadVertices), quadVertices);
		m_vaoQuad->addAttribPointer(0, 2, 4 * sizeof(float), nullptr);
		m_vaoQuad->addAttribPointer(1, 2, 4 * sizeof(float), (void*)(2 * sizeof(float)));
		glEnableVertexAttribArray(0);
	}
	m_vaoQuad->bind();
	glDrawArrays(GL_TRIANGLES, 0, 6);
	VAO::resetVao();
}

void Prisma::PrismaRender::renderQuad(unsigned int instances)
{
	if (m_vaoQuad == nullptr)
	{
		float quadVertices[] = {
			// vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
			// positions   // texCoords
			-1.0f, 1.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f,
			1.0f, -1.0f, 1.0f, 0.0f,

			-1.0f, 1.0f, 0.0f, 1.0f,
			1.0f, -1.0f, 1.0f, 0.0f,
			1.0f, 1.0f, 1.0f, 1.0f
		};
		// setup plane VAO
		m_vaoQuad = std::make_shared<VAO>();
		m_vaoQuad->bind();
		m_vboQuad = std::make_shared<VBO>();
		m_vboQuad->writeData(sizeof(quadVertices), quadVertices);
		m_vaoQuad->addAttribPointer(0, 2, 4 * sizeof(float), nullptr);
		m_vaoQuad->addAttribPointer(1, 2, 4 * sizeof(float), (void*)(2 * sizeof(float)));
		glEnableVertexAttribArray(0);
	}
	m_vaoQuad->bind();
	glDrawArraysInstanced(GL_TRIANGLES, 0, 6, instances);
	VAO::resetVao();
}

std::shared_ptr<Prisma::Texture> Prisma::PrismaRender::renderPerlin(unsigned int width, unsigned int height)
{
	if (!m_noiseShader)
	{
		m_noiseShader = std::make_shared<Shader>("../../../Engine/Shaders/PerlinPipeline/vertex.glsl",
		                                         "../../../Engine/Shaders/PerlinPipeline/fragment.glsl");
	}

	FBO::FBOData fboData;
	fboData.width = width;
	fboData.height = height;
	fboData.internalFormat = GL_RGBA;
	fboData.internalType = GL_FLOAT;
	fboData.name = "PERLIN";
	m_noiseFbo = std::make_shared<FBO>(fboData);
	m_noiseFbo->bind();
	m_noiseShader->use();
	m_noiseShader->setVec2(m_noiseShader->getUniformPosition("resolution"), glm::vec2(width, height));
	renderQuad();

	m_noiseFbo->unbind();
	auto textureData = new char[width * height * 4];

	m_noiseFbo->bind();
	glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, &textureData[0]);
	m_noiseFbo->unbind();
	auto texture = std::make_shared<Texture>();
	texture->id(m_noiseFbo->texture());
	Texture::TextureData data;
	data.width = width;
	data.height = height;
	data.dataContent = (unsigned char*)textureData;
	data.deleteStbi = false;
	data.nrComponents = 4;
	texture->data(data);
	return texture;
}

void Prisma::PrismaRender::createFbo(unsigned int width, unsigned int height)
{
	glGenFramebuffers(1, &m_data.fbo);
	glGenRenderbuffers(1, &m_data.rbo);

	glBindFramebuffer(GL_FRAMEBUFFER, m_data.fbo);
	glBindRenderbuffer(GL_RENDERBUFFER, m_data.rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_data.rbo);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	GarbageCollector::getInstance().add({GarbageCollector::GarbageType::FBO, m_data.fbo});
	GarbageCollector::getInstance().add({GarbageCollector::GarbageType::RBO, m_data.rbo});
	m_data.width = width;
	m_data.height = height;
}

Prisma::PrismaRender::IBLData Prisma::PrismaRender::data()
{
	return m_data;
}
