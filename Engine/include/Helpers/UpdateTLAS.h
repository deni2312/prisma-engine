#pragma once
#include "GlobalData/InstanceData.h"
#include "GlobalData/Platform.h"
#include "Common/interface/RefCntAutoPtr.hpp"


namespace Diligent
{
	struct IShaderBindingTable;
	struct IBuffer;
	struct ITopLevelAS;
}

namespace Prisma{

	class UpdateTLAS : public InstanceData<UpdateTLAS>{
	public:
		UpdateTLAS();
		void update();

		Diligent::RefCntAutoPtr<Diligent::ITopLevelAS> TLAS();

		Diligent::RefCntAutoPtr<Diligent::IShaderBindingTable> SBT();

	private:
		bool m_init=false;

		Diligent::RefCntAutoPtr<Diligent::ITopLevelAS> m_pTLAS;
		Diligent::RefCntAutoPtr<Diligent::IBuffer> m_InstanceBuffer;
		Diligent::RefCntAutoPtr<Diligent::IBuffer> m_ScratchBuffer;
		Diligent::RefCntAutoPtr<Diligent::IShaderBindingTable> m_pSBT;

	};
}
