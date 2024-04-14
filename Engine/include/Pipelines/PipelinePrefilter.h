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
	class PipelinePrefilter {
	public:

		static PipelinePrefilter& getInstance();

		void texture(Prisma::Texture texture);

		uint64_t id() const;

		PipelinePrefilter(const PipelinePrefilter&) = delete;
		PipelinePrefilter& operator=(const PipelinePrefilter&) = delete;

        PipelinePrefilter();

    private:

        static std::shared_ptr<PipelinePrefilter> instance;

		uint64_t m_id;

		std::shared_ptr<Shader> m_shader;
	};
}