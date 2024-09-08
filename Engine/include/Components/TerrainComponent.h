#pragma once

#include "../Helpers/Shader.h"
#include "../SceneObjects/Camera.h"
#include "../Containers/FBO.h"
#include <memory>
#include <chrono>

namespace Prisma {
	class TerrainComponent : public Component {
	public:
		TerrainComponent();

		void ui() override;

		void updateRender(std::shared_ptr<Prisma::FBO> fbo = 0) override;

		void start() override;

	};
}