#include "../../include/Components/CullingComponent.h"

Prisma::CullingComponent::CullingComponent()
{
}

void Prisma::CullingComponent::ui()
{
}

void Prisma::CullingComponent::update()
{
}

void Prisma::CullingComponent::start()
{
	m_shader = std::make_shared<Shader>("../../../Engine/Shaders/CullingPipeline/compute.glsl");
}
