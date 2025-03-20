#include "../../include/SceneObjects/Mesh.h"
#include "../../include/GlobalData/GlobalData.h"
#include "../../include/SceneData/MeshIndirect.h"
#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"
#include "glm/gtx/matrix_decompose.hpp"
#include <string>
#include "../../include/GlobalData/CacheScene.h"

void Prisma::Mesh::loadModel(std::shared_ptr<VerticesData> vertices, bool compute)
{
	m_vertices = vertices;
	if (compute)
	{
		computeAABB();
	}
}

Prisma::Mesh::VerticesData& Prisma::Mesh::verticesData() const
{
	return *m_vertices;
}

void Prisma::Mesh::matrix(const glm::mat4& matrix, bool update)
{
	Node::matrix(matrix);
	CacheScene::getInstance().updateData(true);
}

glm::mat4 Prisma::Mesh::matrix() const
{
	return Node::matrix();
}

void Prisma::Mesh::finalMatrix(const glm::mat4& matrix, bool update)
{
	Node::finalMatrix(matrix);
	if (m_vectorId >= 0)
	{
		Prisma::MeshIndirect::getInstance().updateModels(m_vectorId);
	}
	CacheScene::getInstance().updateData(true);
}

glm::mat4 Prisma::Mesh::finalMatrix() const
{
	return Node::finalMatrix();
}

std::shared_ptr<Prisma::Mesh> Prisma::Mesh::instantiate(std::shared_ptr<Mesh> mesh)
{
	std::shared_ptr<Mesh> newInstance = nullptr;
	if (mesh)
	{
		newInstance = std::make_shared<Mesh>();
		newInstance->loadModel(std::make_shared<VerticesData>(*mesh->m_vertices));
		newInstance->material(std::make_shared<MaterialComponent>(*mesh->material()));
		newInstance->matrix(mesh->matrix());
		newInstance->name(mesh->name() + std::to_string(newInstance->uuid()));
		auto parent = std::make_shared<Node>();
		parent->name(mesh->parent()->name() + std::to_string(parent->uuid()));
		parent->matrix(mesh->parent()->matrix());
		Prisma::GlobalData::getInstance().currentGlobalScene()->root->addChild(parent);
		parent->addChild(newInstance);
	}
	return newInstance;
}

void Prisma::Mesh::material(std::shared_ptr<MaterialComponent> material)
{
	m_material = material;
	addComponent(m_material);
	CacheScene::getInstance().updateTextures(true);
}

std::shared_ptr<Prisma::MaterialComponent> Prisma::Mesh::material()
{
	return m_material;
}

void Prisma::Mesh::vectorId(int vectorId)
{
	m_vectorId = vectorId;
}

int Prisma::Mesh::vectorId()
{
	return m_vectorId;
}

Diligent::RefCntAutoPtr<Diligent::IBuffer> Prisma::Mesh::vBuffer()
{
	return m_vBuffer;
}

Diligent::RefCntAutoPtr<Diligent::IBuffer> Prisma::Mesh::iBuffer()
{
	return m_iBuffer;
}

void Prisma::Mesh::computeAABB()
{
	auto vertices = verticesData().vertices;
	auto indices = verticesData().indices;
	if (vertices.empty())
	{
		m_aabbData = AABBData{glm::vec3(0.0f), glm::vec3(0.0f)};
	}

	glm::vec3 minPoint = vertices[0].position;
	glm::vec3 maxPoint = vertices[0].position;

	for (const auto& vertex : vertices)
	{
		minPoint.x = std::min(minPoint.x, vertex.position.x);
		minPoint.y = std::min(minPoint.y, vertex.position.y);
		minPoint.z = std::min(minPoint.z, vertex.position.z);

		maxPoint.x = std::max(maxPoint.x, vertex.position.x);
		maxPoint.y = std::max(maxPoint.y, vertex.position.y);
		maxPoint.z = std::max(maxPoint.z, vertex.position.z);
	}
	m_aabbData.min = minPoint;
	m_aabbData.max = maxPoint;

	m_aabbData.center = (m_aabbData.max + m_aabbData.min) * glm::vec3(0.5);
	m_aabbData.extents = m_aabbData.max - m_aabbData.center;
}

Prisma::Mesh::AABBData Prisma::Mesh::aabbData()
{
	return m_aabbData;
}

void Prisma::Mesh::uploadGPU()
{
	if (m_vertices) {
		// Create a vertex buffer that stores cube vertices
		Diligent::BufferDesc VertBuffDesc;
		VertBuffDesc.Name = "Vertices Data";
		VertBuffDesc.Usage = Diligent::USAGE_IMMUTABLE;
		VertBuffDesc.BindFlags = Diligent::BIND_VERTEX_BUFFER;
		VertBuffDesc.Size = sizeof(Prisma::Mesh::Vertex) * m_vertices->vertices.size();
		Diligent::BufferData VBData;
		VBData.pData = m_vertices->vertices.data();
		VBData.DataSize = sizeof(Prisma::Mesh::Vertex) * m_vertices->vertices.size();
		Prisma::PrismaFunc::getInstance().contextData().m_pDevice->CreateBuffer(VertBuffDesc, &VBData, &m_vBuffer);

		Diligent::BufferDesc IndBuffDesc;
		IndBuffDesc.Name = "Index Data";
		IndBuffDesc.Usage = Diligent::USAGE_IMMUTABLE;
		IndBuffDesc.BindFlags = Diligent::BIND_INDEX_BUFFER;
		IndBuffDesc.Size = sizeof(unsigned int) * m_vertices->indices.size();
		Diligent::BufferData IBData;
		IBData.pData = m_vertices->indices.data();
		IBData.DataSize = sizeof(unsigned int) * m_vertices->indices.size();
		Prisma::PrismaFunc::getInstance().contextData().m_pDevice->CreateBuffer(IndBuffDesc, &IBData, &m_iBuffer);
	}
}
