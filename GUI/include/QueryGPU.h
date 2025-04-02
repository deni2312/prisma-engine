#pragma once
#include "GlobalData/InstanceData.h"

namespace Prisma {
	class QueryGPU : InstanceData<QueryGPU>{
	public:
		struct QueryData {
			unsigned int numTriangles;
		};

		void start();

		const QueryData& end();
	private:
		QueryData m_data;
	};
}
