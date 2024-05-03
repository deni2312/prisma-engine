#pragma once
#include <memory>
#include "glm/glm.hpp"
#include "../Containers/SSBO.h"
#include "../GlobalData/Defines.h"
#include <vector>

namespace Prisma {
	class AnimationHandler {
	public:
		struct SSBOAnimation {
			glm::mat4 animations[MAX_BONES];
		};
		AnimationHandler(const AnimationHandler&) = delete;
		AnimationHandler& operator=(const AnimationHandler&) = delete;

		static AnimationHandler& getInstance();

		void updateAnimations();

		std::vector<SSBOAnimation>& animations();

		AnimationHandler();

	private:
		std::shared_ptr<SSBO> m_ssboAnimation;
		void copyMatrices(SSBOAnimation& animation, std::vector<glm::mat4>& animationsData);
		std::vector<SSBOAnimation> m_animations;

		static std::shared_ptr<AnimationHandler> instance;
	};
}