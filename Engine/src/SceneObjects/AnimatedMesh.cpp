#include "../../include/SceneObjects/AnimatedMesh.h"

std::map<std::string, Prisma::AnimatedMesh::BoneInfo>& Prisma::AnimatedMesh::boneInfoMap()
{
	return m_BoneInfoMap;
}

int& Prisma::AnimatedMesh::boneInfoCounter()
{
	return m_BoneCounter;
}
