#pragma once
#include "../../../../Engine/include/Helpers/TimeCounter.h"
#include "../../../../Engine/include/Postprocess/PostprocessEffect.h"

class Shockwave : public Prisma::PostprocessEffect
{
public:

	Shockwave();

	void render(std::shared_ptr<Prisma::FBO> texture, std::shared_ptr<Prisma::FBO> raw) override;

private:
	unsigned int m_bindlessPos;
	unsigned int m_centerPos;
	unsigned int m_timePos;
	Prisma::TimeCounter m_counter;
	glm::vec3 m_position = glm::vec3(0, 0, 0);
};

