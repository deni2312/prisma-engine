#pragma once

#include <memory>
#include "../../Engine/include/SceneObjects/Mesh.h"
#include "../../Engine/include/GlobalData/GlobalData.h"


namespace Prisma
{
	class MeshCreator
	{
	public:
		static std::shared_ptr<Prisma::Node> createCube();

		static std::shared_ptr<Prisma::Node> createSphere(int subDivisions);

	private:
		static std::shared_ptr<Node> createMesh(std::shared_ptr<Prisma::Mesh::VerticesData> verticesData,
		                                        const std::string& name);

		static std::shared_ptr<MaterialComponent> getEmptyMaterial();
	};
}
