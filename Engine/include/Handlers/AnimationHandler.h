#pragma once
#include <memory>
#include "glm/glm.hpp"
#include "../GlobalData/Defines.h"
#include "../GlobalData/InstanceData.h"
#include <vector>

namespace Prisma
{
	class AnimationHandler : public InstanceData<AnimationHandler>
	{
	public:
		struct SSBOAnimation
		{
			glm::mat4 animations[Define::MAX_BONES];
		};

		void fill();

		//std::shared_ptr<Prisma::SSBO> ssbo();

		AnimationHandler();

	private:
		//std::shared_ptr<SSBO> m_ssboAnimation;
	};
}
