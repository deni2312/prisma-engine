#pragma once

#include "../SceneObjects/Camera.h"
#include "../SceneData/SceneLoader.h"
#include "../Handlers/MeshHandler.h"
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
		struct IBLData
		{
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

		struct IBLViewProjection
		{
			glm::mat4 view;
			glm::mat4 projection;
		};

		Diligent::RefCntAutoPtr<Diligent::IBuffer> m_iblData;
		Diligent::RefCntAutoPtr<Diligent::ITextureView> m_pRTColor[6];

		void calculateSkybox();


		Texture m_texture;

		//std::shared_ptr<Shader> m_shader;
		//std::shared_ptr<Shader> m_shaderEquirectangular;

		unsigned int m_bindlessPos;
		unsigned int m_bindlessPosEquirectangular;

		bool m_equirectangular = false;

		uint64_t m_id = 0;
		unsigned int m_envCubemap;

		unsigned int m_height;
		unsigned int m_width;

		Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pso;
		Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srb;

		Diligent::RefCntAutoPtr<Diligent::ITexture> m_pMSColorRTV;

		const glm::vec2 m_dimensions = glm::vec2(512, 512);
	};
}
