#pragma once
#include <memory>
#include "glm/glm.hpp"
#include "../Containers/SSBO.h"

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

		static std::shared_ptr<AnimationHandler> instance;
	};
}