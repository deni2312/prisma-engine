#pragma once

#include "SceneObjects/Node.h"

namespace Diligent
{
	struct IPipelineState;
}

class ParticleController
{
public:
	void init(std::shared_ptr<Prisma::Node> root);

	void update();

private:
	struct TimeData
	{
		float delta;
		float time;
		glm::vec2 padding;
	};
	bool m_start = false;
	float currentTime = 0;
	std::chrono::steady_clock::time_point m_startPoint;

	Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pso;
	Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srb;
	Diligent::RefCntAutoPtr<Diligent::IBuffer> m_time;
};
