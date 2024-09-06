#pragma once

#include "../Helpers/Shader.h"
#include "../SceneObjects/Camera.h"
#include "../SceneData/SceneLoader.h"
#include "../Handlers/MeshHandler.h"
#include "../SceneData/MeshIndirect.h"
#include "../Containers/FBO.h"
#include "PipelineFullScreen.h"
#include <memory>
#include "../GlobalData/InstanceData.h"

namespace Prisma {
	class PipelineLUT : public InstanceData<PipelineLUT>{
	public:

		void texture();

		uint64_t id() const;
        PipelineLUT();

    private:

		uint64_t m_id;

		unsigned int m_quadVAO = 0;
		unsigned int m_quadVBO;

		std::shared_ptr<Shader> m_shader;
	};
}