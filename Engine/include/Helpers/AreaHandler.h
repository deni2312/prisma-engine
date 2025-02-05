#pragma once

#include "../GlobalData/InstanceData.h"
#include <iostream>

namespace Prisma {
	class AreaHandler : public InstanceData<AreaHandler> {
	public:
		AreaHandler();

		uint64_t idLut();
		uint64_t idM();

	private:
		uint64_t m_idLut;
		uint64_t m_idM;
	};
}