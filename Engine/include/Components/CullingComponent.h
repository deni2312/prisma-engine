#pragma once

#include "Component.h"
#include "../Physics/Physics.h"
#include "glm/detail/type_quat.hpp"
#include "../SceneObjects/Mesh.h"
#include "glm/gtx/matrix_decompose.hpp"
#include "../GlobalData/GlobalData.h"

namespace Prisma
{
	class CullingComponent : public Prisma::Component
	{
	public:
		CullingComponent();

		void ui() override;

		void update() override;

		void start() override;

	private:
		std::shared_ptr<Prisma::Shader> m_shader;
		std::shared_ptr<Prisma::SSBO> m_ssbo;
	};
}
