#pragma once

#include "imgui.h"
#include <vector>
#include <memory>
#include "glm/glm.hpp"
#include <string>
#include "../../Engine/include/Containers/FBO.h"
#include "../../Engine/include/Containers/SSBO.h"
#include "../../Engine/include/SceneObjects/Mesh.h"
#include "../../Engine/include/GlobalData/InstanceData.h"

namespace Prisma
{
	class PixelCapture : public InstanceData<PixelCapture>
	{
	public:
		PixelCapture();
		std::shared_ptr<Mesh> capture(glm::vec2 position, const glm::mat4& model);

	private:
		std::shared_ptr<FBO> m_fbo;
		unsigned int m_modelLocation;
	};
}
