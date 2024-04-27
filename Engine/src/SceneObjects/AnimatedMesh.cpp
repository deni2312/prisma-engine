#include "../../include/SceneObjects/AnimatedMesh.h"

void Prisma::AnimatedMesh::loadAnimateModel(std::shared_ptr<AnimateVerticesData> vertices) {
	m_animateVertices = vertices;
}

std::map<std::string, Prisma::AnimatedMesh::BoneInfo>& Prisma::AnimatedMesh::boneInfoMap()
{
	return m_BoneInfoMap;
}

int& Prisma::AnimatedMesh::boneInfoCounter()
{
	return m_BoneCounter;
}
