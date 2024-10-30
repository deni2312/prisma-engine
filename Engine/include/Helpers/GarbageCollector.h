#pragma once
#include <memory>
#include <vector>
#include "../GlobalData/InstanceData.h"

namespace Prisma
{
	class GarbageCollector : public InstanceData<GarbageCollector>
	{
	public:
		enum GarbageType
		{
			VAO,
			BUFFER,
			TEXTURE,
			FBO,
			RBO
		};

		void add(std::pair<GarbageType, unsigned int> garbage);

		void addTexture(std::pair<unsigned int, uint64_t> texture);

		void clear();

		GarbageCollector();

	private:
		std::vector<std::pair<GarbageType, unsigned int>> m_garbage;
		std::vector<std::pair<unsigned int, uint64_t>> m_garbageTexture;
	};
}
