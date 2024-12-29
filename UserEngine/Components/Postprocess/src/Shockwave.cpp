#include "../include/Shockwave.h"
#include "../../../../Engine/include/Helpers/PrismaRender.h"

static std::shared_ptr<Prisma::Shader> shader=nullptr;

Shockwave::Shockwave()
{
	shader = std::make_shared<Prisma::Shader>("../../../UserEngine/Shaders/ShockwavePipeline/vertex.glsl",
		"../../../UserEngine/Shaders/ShockwavePipeline/fragment.glsl");
	if (!shader) {
		shader->use();
	}
	m_bindlessPos = shader->getUniformPosition("screenTexture");
	m_centerPos = shader->getUniformPosition("shockCenter");
	m_timePos = shader->getUniformPosition("time");
	m_counter.start();
}

void Shockwave::render(std::shared_ptr<Prisma::FBO> texture, std::shared_ptr<Prisma::FBO> raw)
{
	glClear(GL_DEPTH_BUFFER_BIT);
	shader->use();
	shader->setInt64(m_bindlessPos, texture->texture());
	shader->setVec3(m_centerPos, m_position);
	shader->setFloat(m_timePos, m_counter.duration_seconds());
	Prisma::PrismaRender::getInstance().renderQuad();
}

void Shockwave::position(glm::vec3 position)
{
	m_position = position;
}
