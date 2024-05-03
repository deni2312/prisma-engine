#pragma once
#include <memory>
#include "glm/glm.hpp"
#include "../Containers/SSBO.h"
#include "../GlobalData/Defines.h"
#include <vector>

namespace Prisma {
	class AnimationHandler {
	public:
		AnimationHandler(const AnimationHandler&) = delete;
		AnimationHandler& operator=(const AnimationHandler&) = delete;

		static AnimationHandler& getInstance();

		void updateAnimations();

		AnimationHandler();

	private:
		std::shared_ptr<SSBO> m_ssboAnimation;
		struct SSBOAnimation {
			glm::mat4 animations[MAX_BONES];
		};
		void copyMatrices(SSBOAnimation& animation, std::vector<glm::mat4>& animationsData);
		static std::shared_ptr<AnimationHandler> instance;
	};
}