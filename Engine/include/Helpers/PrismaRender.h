#pragma once

#include "../GlobalData/InstanceData.h"
#include <memory>
#include <glm/glm.hpp>

#include "Common/interface/RefCntAutoPtr.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"

namespace Diligent
{
	struct IBuffer;
}

namespace Prisma
{
	class PrismaRender : public InstanceData<PrismaRender>
	{
	public:
		struct IBLData
		{
			unsigned int fbo=0;
			unsigned int rbo=0;
			unsigned int width=0;
			unsigned int height=0;
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

		struct BufferData
		{
			Diligent::RefCntAutoPtr<Diligent::IBuffer> vBuffer;
			Diligent::RefCntAutoPtr<Diligent::IBuffer> iBuffer;
			unsigned int iBufferSize;
		};

		void createFbo(unsigned int width, unsigned int height);
		IBLData data();

		BufferData quadBuffer();

		//void renderQuad(unsigned int instances);
		//std::shared_ptr<Texture> renderPerlin(unsigned int width, unsigned int height);


		PrismaRender();

	private:
		IBLData m_data;

		Prisma::PrismaRender::BufferData m_quadBufferData;

		struct VData
		{
			glm::vec3 pos;
			glm::vec2 uv;
		};

		bool m_initQuad = false;

	};
}
