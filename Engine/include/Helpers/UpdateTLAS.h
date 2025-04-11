#pragma once
#include "GlobalData/InstanceData.h"
#include "GlobalData/Platform.h"
#include "Common/interface/RefCntAutoPtr.hpp"
#include <vector>
#include <string>
#include <glm/glm.hpp>


namespace Diligent
{
	struct IShaderBindingTable;
	struct IBuffer;
	struct ITopLevelAS;
	struct TLASBuildInstanceData;
}

namespace Prisma{

	class UpdateTLAS : public InstanceData<UpdateTLAS>{
	public:
		UpdateTLAS();
		void update();

		void updateSizeTLAS();

		Diligent::RefCntAutoPtr<Diligent::ITopLevelAS> TLAS();

		Diligent::RefCntAutoPtr<Diligent::IShaderBindingTable> SBT();

	private:
		Diligent::RefCntAutoPtr<Diligent::ITopLevelAS> m_pTLAS;
		Diligent::RefCntAutoPtr<Diligent::IBuffer> m_InstanceBuffer;
		Diligent::RefCntAutoPtr<Diligent::IBuffer> m_ScratchBuffer;
		Diligent::RefCntAutoPtr<Diligent::IBuffer> m_vertexData;
		Diligent::RefCntAutoPtr<Diligent::IBuffer> m_vertexLocation;
		Diligent::RefCntAutoPtr<Diligent::IShaderBindingTable> m_pSBT;

		void updateTLAS(bool update);

		struct VertexBlas
		{
			glm::vec4 pos;
			glm::vec4 norm;
			glm::vec4 uv;
		};
	};
}
