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
	class PipelineSkybox : public InstanceData<PipelineSkybox>
	{
	public:
		void render();


		void texture(Texture texture, bool equirectangular = false);
		PipelineSkybox();

		const Prisma::Texture& texture() const;

	private:
		void calculateSkybox();


		Texture m_texture;

		std::shared_ptr<Shader> m_shader;
		std::shared_ptr<Shader> m_shaderEquirectangular;

		unsigned int m_bindlessPos;
		unsigned int m_bindlessPosEquirectangular;

		bool m_equirectangular = false;

		uint64_t m_id = 0;
		unsigned int m_envCubemap;

		unsigned int m_height;
		unsigned int m_width;
	};
}
