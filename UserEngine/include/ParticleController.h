#pragma once

#include "SceneObjects/Node.h"

class ParticleController
{
public:
	void init(std::shared_ptr<Prisma::Node> root);

	void update();

private:
	//std::shared_ptr<Prisma::Shader> m_compute;
	unsigned int m_deltaPos;
	unsigned int m_timePos;
	bool m_start = false;
	float currentTime = 0;
	std::chrono::steady_clock::time_point m_startPoint;
};
