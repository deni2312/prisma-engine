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
	class PipelineSkybox {
	public:
		void render();

		static PipelineSkybox& getInstance();

		void texture(Prisma::Texture texture, bool equirectangular = false);

		PipelineSkybox(const PipelineSkybox&) = delete;
		PipelineSkybox& operator=(const PipelineSkybox&) = delete;

        PipelineSkybox();

    private:
        uint64_t calculateSkybox();

		static std::shared_ptr<PipelineSkybox> instance;

		Prisma::Texture m_texture;

		std::shared_ptr<Shader> m_shader;
		std::shared_ptr<Shader> m_shaderEquirectangular;

		unsigned int m_bindlessPos;
		unsigned int m_bindlessPosEquirectangular;

		bool m_equirectangular=false;

		uint64_t m_skyboxId;
	};
}