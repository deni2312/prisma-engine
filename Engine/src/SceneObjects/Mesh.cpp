#include "../../include/SceneObjects/Mesh.h"
#include "../../include/GlobalData/GlobalData.h"
#include "glm/glm.hpp"
#include "LinearMath/btTransform.h"
#include "BulletCollision/CollisionShapes/btConvexHullShape.h"
#include "btBulletDynamicsCommon.h"
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

const Prisma::Mesh::VerticesData& Prisma::Mesh::verticesData() const
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
		currentGlobalScene->root->addChild(parent);
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

void Prisma::Mesh::vectorId(unsigned int vectorId)
{
	m_vectorId = vectorId;
}

unsigned int Prisma::Mesh::vectorId()
{
	return m_vectorId;
}

void Prisma::Mesh::addGlobalList(bool globalList)
{
	m_addGlobal = globalList;
}

bool Prisma::Mesh::addGlobalList() const
{
	return m_addGlobal;
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
	glm::vec3 center(0.0f);

	for (const auto& vertex : vertices)
	{
		minPoint = min(minPoint, vertex.position);
		maxPoint = max(maxPoint, vertex.position);
		center += vertex.position;
	}

	m_aabbData.min = minPoint;
	m_aabbData.max = maxPoint;

	center /= static_cast<float>(vertices.size());

	m_aabbData.center = center;
}

Prisma::Mesh::AABBData Prisma::Mesh::aabbData()
{
	return m_aabbData;
}
