#pragma once
#include <memory>
#include "Shader.h"
#include "../GlobalData/Defines.h"
#include "Common/interface/RefCntAutoPtr.hpp"

namespace Diligent
{
	struct IShaderResourceBinding;
	struct IPipelineState;
	struct IBuffer;
}

namespace Prisma
{
	class ClusterCalculation
	{
	public:
		struct alignas(16) Cluster
		{
			glm::vec4 minPoint;
			glm::vec4 maxPoint;
			unsigned int count;
			unsigned int lightIndices[Define::MAX_CLUSTER_SIZE];
		};

		ClusterCalculation();
		void updateCamera();
		void updateLights();


	private:

		static constexpr unsigned int m_gridSizeX = 12;
		static constexpr unsigned int m_gridSizeY = 12;
		static constexpr unsigned int m_gridSizeZ = 24;

		unsigned int m_nearPos;
		unsigned int m_farPos;
		unsigned int m_inverseProjectionPos;
		unsigned int m_gridSizePos;
		unsigned int m_screenDimensionsPos;

		struct  ClusterData
		{
			glm::vec4 nearFar;
			glm::mat4 inverseProjection;
			glm::ivec4 gridSize;
			glm::ivec4 screenDimensions;
		};

		Diligent::RefCntAutoPtr<Diligent::IBuffer> m_clusterData;
		Diligent::RefCntAutoPtr<Diligent::IBuffer> m_cluster;
		Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pso;
		Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srb;
		ClusterData m_data;

	};
}
