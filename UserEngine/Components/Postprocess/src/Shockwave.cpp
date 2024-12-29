#include "../include/Shockwave.h"

static std::shared_ptr<Prisma::Shader> shader;

Shockwave::Shockwave()
{
	shader = std::make_shared<Prisma::Shader>("../../../UserEngine/Shaders/ShockwavePipeline/vertex.glsl",
		"../../../UserEngine/Shaders/ShockwavePipeline/fragment.glsl");
	shader->use();
	m_bindlessPos = shader->getUniformPosition("screenTexture");
}

void Shockwave::render(std::shared_ptr<Prisma::FBO> texture, std::shared_ptr<Prisma::FBO> raw)
{
}
