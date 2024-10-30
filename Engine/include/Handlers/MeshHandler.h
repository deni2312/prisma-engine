#pragma once
#include <memory>
#include "glm/glm.hpp"
#include "../Containers/Ubo.h"
#include "../Helpers/Settings.h"

#include "../GlobalData/InstanceData.h"

namespace Prisma
{
	class MeshHandler : public InstanceData<MeshHandler>
	{
	public:
		struct UBOData
		{
			glm::mat4 view;
			glm::mat4 projection;
		};

		struct alignas(16) UBOCluster
		{
			glm::uvec4 gridSize;
			glm::uvec4 screenDimensions;
			float zNear;
			float zFar;
			float padding[2];
		};

		struct alignas(16) UBOFragment
		{
			glm::vec4 viewPos;
			uint64_t irradiancePos;
			uint64_t prefilterPos;
			uint64_t lutPos;
			glm::vec2 padding;
		};

		std::shared_ptr<UBOData> data() const;
		std::shared_ptr<Ubo> ubo() const;
		void updateCamera();
		void updateCluster();
		void updateFragment();
		MeshHandler();

	private:
		std::shared_ptr<UBOData> m_uboData;
		std::shared_ptr<Ubo> m_ubo;
		std::shared_ptr<Ubo> m_uboCluster;
		std::shared_ptr<Ubo> m_uboFragment;
		UBOCluster m_uboClusterData;
		UBOFragment m_fragment;
		Settings m_settings;
	};
}
