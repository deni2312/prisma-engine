#pragma once

#include "Shader.h"
#include "../SceneObjects/Camera.h"
#include "../SceneData/SceneLoader.h"
#include "../Handlers/MeshHandler.h"
#include "../SceneData/MeshIndirect.h"
#include "../Containers/FBO.h"
#include "../Pipelines/PipelineFullScreen.h"
#include <memory>

namespace Prisma {
	class IBLBuilder {
	public:
		struct IBLData {
			unsigned int fbo;
			unsigned int rbo;
			unsigned int width;
			unsigned int height;
			glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
			glm::mat4 captureViews[18] =
			{
				glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
				glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
				glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
				glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
				glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
				glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
			};
		};
		static IBLBuilder& getInstance();
		void createFbo(unsigned int width, unsigned int height);
		IBLData data();
		void renderCube();
		void renderQuad();
		IBLBuilder(const IBLBuilder&) = delete;
		IBLBuilder& operator=(const IBLBuilder&) = delete;

        IBLBuilder();

    private:
        static std::shared_ptr<IBLBuilder> instance;
		std::shared_ptr<Prisma::VAO> m_vaoCube = nullptr;
        std::shared_ptr<Prisma::VBO> m_vboCube = nullptr;
        std::shared_ptr<Prisma::VAO> m_vaoQuad = nullptr;
        std::shared_ptr<Prisma::VBO> m_vboQuad = nullptr;
		IBLData m_data;
	};
}