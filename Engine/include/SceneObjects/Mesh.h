#pragma once
#include <vector>
#include <memory>
#include "Node.h"
#include "../Containers/Texture.h"
#include "../Components/MaterialComponent.h"
#include "../Physics/Physics.h"
#include "../Components/Component.h"
#include <map>
#include "../GlobalData/Platform.h"

#include "Common/interface/RefCntAutoPtr.hpp"


namespace Diligent
{
	struct IBottomLevelAS;
}

namespace Prisma
{
	class Mesh : public Node
	{
	public:
		struct Vertex
		{
			glm::vec3 position = glm::vec3(0, 0, 0);
			glm::vec3 normal = glm::vec3(0, 0, 0);
			glm::vec2 texCoords = glm::vec2(0, 0);
			glm::vec3 tangent = glm::vec3(0, 0, 0);
			glm::vec3 bitangent = glm::vec3(0, 0, 0);
		};

		struct VerticesData
		{
			std::vector<Vertex> vertices;
			std::vector<unsigned int> indices;
		};

		struct AABBData
		{
			glm::vec3 min;
			glm::vec3 max;
			glm::vec3 center;
			glm::vec3 extents;
		};

		struct AABBssbo
		{
			glm::vec4 center;
			glm::vec4 extents;
		};

		struct BoneInfo
		{
			int id;
			glm::mat4 offset;
		};

		struct MeshData
		{
			glm::mat4 model;
			glm::mat4 normal;
		};

		void loadModel(std::shared_ptr<VerticesData> vertices, bool compute = true);
		VerticesData& verticesData() const;
		void matrix(const glm::mat4& matrix, bool update = true) override;
		glm::mat4 matrix() const override;
		void finalMatrix(const glm::mat4& matrix, bool update = true) override;
		glm::mat4 finalMatrix() const override;

		static std::shared_ptr<Mesh> instantiate(std::shared_ptr<Mesh> mesh);
		void material(std::shared_ptr<MaterialComponent> material);
		std::shared_ptr<MaterialComponent> material();
		void vectorId(int vectorId);
		virtual void computeAABB();
		AABBData aabbData();

		int vectorId();

		Diligent::RefCntAutoPtr<Diligent::IBuffer> vBuffer();

		Diligent::RefCntAutoPtr<Diligent::IBuffer> iBuffer();

		void uploadBLAS();

		Diligent::RefCntAutoPtr<Diligent::IBottomLevelAS> blas();

		Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> raytracingSrb();

	protected:
		std::shared_ptr<VerticesData> m_vertices;
		std::shared_ptr<MaterialComponent> m_material;

		int m_vectorId = -1;
		AABBData m_aabbData;

		bool m_blasGPU = false;
		Diligent::RefCntAutoPtr<Diligent::IBuffer> m_CubeAttribsCB;
		Diligent::RefCntAutoPtr<Diligent::IBottomLevelAS> m_pCubeBLAS;

		Diligent::RefCntAutoPtr<Diligent::IBuffer> m_vBuffer;
		Diligent::RefCntAutoPtr<Diligent::IBuffer> m_iBuffer;

		Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srb;
	};
}
