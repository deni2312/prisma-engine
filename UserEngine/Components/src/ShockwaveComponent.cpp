#include "../include/ShockwaveComponent.h"
#include "../Postprocess/Postprocess.h"

ShockwaveComponent::ShockwaveComponent()
{
	name("Shockwave");
	m_shockwave = std::make_shared<Shockwave>();
}

void ShockwaveComponent::start()
{
	Prisma::Component::start();
	Prisma::Postprocess::getInstance().addPostProcess(m_shockwave);
	m_counter.start();
}

void ShockwaveComponent::update()
{
	if (m_counter.duration_seconds()>1)
	{
		Prisma::Postprocess::getInstance().removePostProcess(m_shockwave);
	}
}

void ShockwaveComponent::position(const glm::vec3& position)
{
	m_position = position;
	m_shockwave->position(m_position);
}
