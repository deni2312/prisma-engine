#include "../../include/SceneObjects/AnimatedMesh.h"
#include "../../include/GlobalData/GlobalData.h"

void Prisma::AnimatedMesh::loadAnimateModel(std::shared_ptr<AnimateVerticesData> vertices) {
	m_animateVertices = vertices;
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

void Prisma::AnimatedMesh::animator(std::shared_ptr<Animator> animator) {
	m_animator = animator;
	m_animator->mesh(this);
}

std::shared_ptr<Prisma::Animator> Prisma::AnimatedMesh::animator() {
	return m_animator;
}
