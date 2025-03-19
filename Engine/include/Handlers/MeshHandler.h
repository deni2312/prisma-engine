#pragma once
#include <memory>
#include "glm/glm.hpp"
#include "../Helpers/Settings.h"

#include "../GlobalData/InstanceData.h"
#include "Common/interface/RefCntAutoPtr.hpp"

namespace Diligent
{
	struct IBuffer;
}

namespace Prisma
{
	class MeshHandler : public InstanceData<MeshHandler>
	{
	public:
		struct ViewProjectionData
		{
			glm::mat4 view;
			glm::mat4 projection;
		};

		struct alignas(16) ClusterData
		{
			glm::uvec4 gridSize;
			glm::uvec4 screenDimensions;
			float zNear;
			float zFar;
			float padding[2];
		};

		struct alignas(16) FragmentData
		{
			glm::vec4 viewPos;
			uint64_t irradiancePos;
			uint64_t prefilterPos;
			uint64_t lutPos;
			glm::vec2 padding;
			uint64_t textureLut;
			uint64_t textureM;
		};

		std::shared_ptr<ViewProjectionData> data() const;
		Diligent::RefCntAutoPtr<Diligent::IBuffer> viewProjection() const;
		void updateCamera();
		void updateCluster();
		void updateFragment();
		MeshHandler();

	private:
		std::shared_ptr<ViewProjectionData> m_uboData;
		ClusterData m_uboClusterData;
		FragmentData m_fragment;
		Settings m_settings;
		Diligent::RefCntAutoPtr<Diligent::IBuffer> m_viewProjection;

	};
}
