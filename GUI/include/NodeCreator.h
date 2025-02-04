#pragma once

#include <memory>
#include "../../Engine/include/SceneObjects/Mesh.h"
#include "../../Engine/include/GlobalData/GlobalData.h"


namespace Prisma
{
	class NodeCreator
	{
	public:
		static std::shared_ptr<Prisma::Node> createCube();

		static std::shared_ptr<Prisma::Node> createSphere(int subDivisions);

		static std::shared_ptr<Prisma::Node> createOmnidirectional();

		static std::shared_ptr<Prisma::Node> createDirectional();

		static std::shared_ptr<Prisma::Node> createArea();

	private:
		static std::shared_ptr<Node> createMesh(std::shared_ptr<Prisma::Mesh::VerticesData> verticesData,
		                                        const std::string& name);

		static std::shared_ptr<MaterialComponent> getEmptyMaterial();
	};
}
