#pragma once
#include <memory>
#include "Shader.h"
#include "../Containers/SSBO.h"
#include "../GlobalData/Defines.h"

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
			unsigned int lightIndices[MAX_CLUSTER_SIZE];
		};

		ClusterCalculation(unsigned int numClusters);
		void updateCamera();
		void updateLights();

		static glm::vec3 grids()
		{
			return {m_gridSizeX, m_gridSizeY, m_gridSizeZ};
		}

	private:
		std::shared_ptr<Shader> m_shader;
		std::shared_ptr<Shader> m_shaderLights;

		static constexpr unsigned int m_gridSizeX = 12;
		static constexpr unsigned int m_gridSizeY = 12;
		static constexpr unsigned int m_gridSizeZ = 24;

		unsigned int m_nearPos;
		unsigned int m_farPos;
		unsigned int m_inverseProjectionPos;
		unsigned int m_gridSizePos;
		unsigned int m_screenDimensionsPos;

		std::shared_ptr<SSBO> m_ssbo;
	};
}
