#include "../include/ShockwaveComponent.h"
#include "../../../Engine/include/Postprocess/Postprocess.h"

ShockwaveComponent::ShockwaveComponent()
{
	name("Shockwave");
}

void ShockwaveComponent::start()
{
	Prisma::Component::start();
	m_shockwave = std::make_shared<Shockwave>();
	Prisma::Postprocess::getInstance().addPostProcess(m_shockwave);
	m_counter.start();
}

void ShockwaveComponent::update()
{
	if (m_counter.duration_seconds()>1)
	{
		Prisma::Postprocess::getInstance().removePostProcess(m_shockwave);
		parent()->removeComponent("Shockwave");
	}
}
