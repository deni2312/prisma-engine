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
	class PipelineDiffuseIrradiance : public InstanceData<PipelineDiffuseIrradiance>
	{
	public:
		void texture(Texture texture);

		uint64_t id();
		PipelineDiffuseIrradiance();

	private:
		uint64_t m_id=0;
		unsigned int m_diffuseIrradiance;

		std::shared_ptr<Shader> m_shader;
		glm::mat4 m_projection;
		unsigned int m_skyboxVAO;
		unsigned int m_bindlessPos;
	};
}
