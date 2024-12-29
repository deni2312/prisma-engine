#include "../include/ShockwaveComponent.h"
#include "../../../Engine/include/Postprocess/Postprocess.h"

void ShockwaveComponent::start()
{
	Prisma::Component::start();
	m_shockwave = std::make_shared<Shockwave>();
	Prisma::Postprocess::getInstance().addPostProcess(m_shockwave);
}

void ShockwaveComponent::update()
{
}
