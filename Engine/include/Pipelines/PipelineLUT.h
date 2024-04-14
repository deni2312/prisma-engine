#pragma once

#include "../Helpers/Shader.h"
#include "../SceneObjects/Camera.h"
#include "../SceneData/SceneLoader.h"
#include "../Handlers/MeshHandler.h"
#include "../SceneData/MeshIndirect.h"
#include "../Containers/FBO.h"
#include "PipelineFullScreen.h"
#include <memory>

namespace Prisma {
	class PipelineLUT {
	public:

		static PipelineLUT& getInstance();

		void texture();

		uint64_t id() const;

		PipelineLUT(const PipelineLUT&) = delete;
		PipelineLUT& operator=(const PipelineLUT&) = delete;

        PipelineLUT();

    private:

        static std::shared_ptr<PipelineLUT> instance;

		uint64_t m_id;

		unsigned int m_quadVAO = 0;
		unsigned int m_quadVBO;

		std::shared_ptr<Shader> m_shader;
	};
}