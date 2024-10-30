#include "../../include/SceneObjects/AnimatedMesh.h"
#include "../../include/GlobalData/GlobalData.h"

void Prisma::AnimatedMesh::computeAABB()
{
	auto vertices = animateVerticesData()->vertices;
	auto indices = animateVerticesData()->indices;
	if (vertices.empty())
	{
		m_aabbData = AABBData{glm::vec3(0.0f), glm::vec3(0.0f)};
	}

	glm::vec3 minPoint = vertices[0].position;
	glm::vec3 maxPoint = vertices[0].position;

	for (const auto& vertex : vertices)
	{
		minPoint = min(minPoint, vertex.position);
		maxPoint = max(maxPoint, vertex.position);
	}

	m_aabbData.min = minPoint;
	m_aabbData.max = maxPoint;

	glm::vec3 center(0.0f);

	for (const auto& index : indices)
	{
		center += vertices[index].position;
	}

	center /= static_cast<float>(vertices.size());

	m_aabbData.center = center;
}

void Prisma::AnimatedMesh::loadAnimateModel(std::shared_ptr<AnimateVerticesData> vertices)
{
	m_animateVertices = vertices;
	computeAABB();
}

std::shared_ptr<Prisma::AnimatedMesh::AnimateVerticesData> Prisma::AnimatedMesh::animateVerticesData()
{
	return m_animateVertices;
}

std::map<std::string, Prisma::BoneInfo>& Prisma::AnimatedMesh::boneInfoMap()
{
	return m_BoneInfoMap;
}

int& Prisma::AnimatedMesh::boneInfoCounter()
{
	return m_BoneCounter;
}

void Prisma::AnimatedMesh::animator(std::shared_ptr<Animator> animator)
{
	m_animator = animator;
	m_animator->mesh(this);
}

std::shared_ptr<Prisma::Animator> Prisma::AnimatedMesh::animator()
{
	return m_animator;
}
