#pragma once

#include "../SceneObjects/Camera.h"
#include "../SceneData/SceneLoader.h"
#include "../Handlers/MeshHandler.h"
#include <memory>

#include "PipelineSkyboxRenderer.h"
#include "../GlobalData/InstanceData.h"

namespace Prisma
{
	class PipelineSkybox : public InstanceData<PipelineSkybox>
	{
	public:
		struct IBLData
		{
			glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
			glm::mat4 captureViews[6] =
			{
				lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
				lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
				lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
				lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
				lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
				lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))
			};
		};

		struct IBLViewProjection
		{
			glm::mat4 view;
			glm::mat4 projection;
		};

		void render();

		void texture(Texture texture);
		PipelineSkybox();

		const Prisma::Texture& texture() const;

		bool isInit();

	private:

		Diligent::RefCntAutoPtr<Diligent::IBuffer> m_iblData;
		Diligent::RefCntAutoPtr<Diligent::ITextureView> m_pRTColor[6];

		void calculateSkybox();

		Texture m_texture;

		Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pso;

		Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srb;

		Diligent::RefCntAutoPtr<Diligent::ITexture> m_pMSColorRTV;

		const glm::vec2 m_dimensions = glm::vec2(4096, 4096);

		const IBLData m_iblTransform;

		std::shared_ptr<Prisma::PipelineSkyboxRenderer> m_skyboxRenderer;

		bool m_init = false;
	};
}
