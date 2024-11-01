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

		void updateAnimations();

		void clear();

		void fill();

		std::vector<SSBOAnimation>& animations();

		AnimationHandler();

	private:
		std::shared_ptr<SSBO> m_ssboAnimation;
		std::vector<SSBOAnimation> m_animations;
	};
}
