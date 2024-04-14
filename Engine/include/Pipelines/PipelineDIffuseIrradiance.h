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
	class PipelineDiffuseIrradiance {
	public:

		static PipelineDiffuseIrradiance& getInstance();

		void texture(Prisma::Texture texture);

		uint64_t id();

		PipelineDiffuseIrradiance(const PipelineDiffuseIrradiance&) = delete;
		PipelineDiffuseIrradiance& operator=(const PipelineDiffuseIrradiance&) = delete;

        PipelineDiffuseIrradiance();

    private:

        static std::shared_ptr<PipelineDiffuseIrradiance> instance;

		Prisma::Texture m_texture;
		uint64_t m_id;

		std::shared_ptr<Shader> m_shader;
		glm::mat4 m_projection;
		unsigned int m_skyboxVAO;
		unsigned int m_bindlessPos;
	};
}