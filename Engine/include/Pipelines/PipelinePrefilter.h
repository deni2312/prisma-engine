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

namespace Prisma
{
	class PipelinePrefilter : public InstanceData<PipelinePrefilter>
	{
	public:
		void texture(Texture texture);

		uint64_t id() const;
		PipelinePrefilter();

	private:
		uint64_t m_id=0;

		unsigned int m_prefilterMap;

		std::shared_ptr<Shader> m_shader;
	};
}
