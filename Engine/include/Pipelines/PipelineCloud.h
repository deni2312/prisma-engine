#pragma once

#include "../Helpers/Shader.h"
#include "../SceneObjects/Camera.h"
#include "../SceneData/SceneLoader.h"
#include "../Containers/FBO.h"
#include "PipelineFullScreen.h"
#include "../Helpers/Settings.h"
#include <memory>

namespace Prisma {
	class PipelineCloud {
	public:
		PipelineCloud();
		void render();
	
	};
}