#pragma once

#include "Shader.h"
#include "../SceneObjects/Camera.h"
#include "../SceneData/SceneLoader.h"
#include "../Handlers/MeshHandler.h"
#include "../SceneData/MeshIndirect.h"
#include "../Containers/FBO.h"
#include "../Pipelines/PipelineFullScreen.h"
#include "../GlobalData/InstanceData.h"
#include <memory>

namespace Prisma
{
	class PrismaRender : public InstanceData<PrismaRender>
	{
	public:
		struct IBLData
		{
			unsigned int fbo;
			unsigned int rbo;
			unsigned int width;
			unsigned int height;
			glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
			glm::mat4 captureViews[18] =
			{
				lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
				lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
				lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
				lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
				lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
				lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))
			};
		};

		void createFbo(unsigned int width, unsigned int height);
		IBLData data();
		void renderCube();
		void renderQuad();
		void renderQuad(unsigned int instances);
		std::shared_ptr<Texture> renderPerlin(unsigned int width, unsigned int height);
		PrismaRender();

	private:
		std::shared_ptr<VAO> m_vaoCube = nullptr;
		std::shared_ptr<VBO> m_vboCube = nullptr;
		std::shared_ptr<VAO> m_vaoQuad = nullptr;
		std::shared_ptr<VBO> m_vboQuad = nullptr;
		std::shared_ptr<Shader> m_noiseShader = nullptr;
		std::shared_ptr<FBO> m_noiseFbo;
		IBLData m_data;
	};
}
