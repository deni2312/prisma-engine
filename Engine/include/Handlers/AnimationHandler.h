#pragma once
#include <memory>
#include "glm/glm.hpp"
#include "../Containers/SSBO.h"
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
			glm::mat4 animations[MAX_BONES];
		};

		void fill();

		std::shared_ptr<Prisma::SSBO> ssbo();

		AnimationHandler();

	private:
		std::shared_ptr<SSBO> m_ssboAnimation;
	};
}
