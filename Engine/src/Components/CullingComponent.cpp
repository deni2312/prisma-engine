#include "../../include/Components/CullingComponent.h"

Prisma::CullingComponent::CullingComponent()
{
	name("Culling");
}

void Prisma::CullingComponent::ui()
{
	Prisma::Component::ui();
}

void Prisma::CullingComponent::update()
{
}

void Prisma::CullingComponent::start()
{
	Component::start();
	//m_shader = std::make_shared<Shader>("../../../Engine/Shaders/CullingPipeline/compute.glsl");
	//m_ssbo = std::make_shared<SSBO>(17);
	//m_ssbo->resize(sizeof(glm::mat4) * Prisma::GlobalData::getInstance().currentGlobalScene()->meshes.size());
}
