#pragma once
#include "../../../../Engine/include/Postprocess/PostprocessEffect.h"

class Shockwave : public Prisma::PostprocessEffect
{
public:

	Shockwave();


	void render(std::shared_ptr<Prisma::FBO> texture, std::shared_ptr<Prisma::FBO> raw) override;

private:
	unsigned int m_bindlessPos;
};

