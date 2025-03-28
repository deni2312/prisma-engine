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

		struct BufferData
		{
			Diligent::RefCntAutoPtr<Diligent::IBuffer> vBuffer;
			Diligent::RefCntAutoPtr<Diligent::IBuffer> iBuffer;
			unsigned int iBufferSize;
		};

		BufferData quadBuffer();

		BufferData cubeBuffer();



		PrismaRender();

	private:

		Prisma::PrismaRender::BufferData m_quadBufferData;
		Prisma::PrismaRender::BufferData m_cubeBufferData;

		struct VData
		{
			glm::vec3 pos;
			glm::vec2 uv;
		};

		bool m_initQuad = false;
		bool m_initCube = false;

	};
}
