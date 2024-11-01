#include "../../include/Handlers/AnimationHandler.h"
#include "../../include/GlobalData/GlobalData.h"
#include "../../include/GlobalData/Defines.h"
#include "../../include/SceneData/MeshIndirect.h"

void Prisma::AnimationHandler::updateAnimations()
{
	m_ssboAnimation->modifyData(0, currentGlobalScene->animateMeshes.size() * sizeof(SSBOAnimation),
	                            m_animations.data());
}

void Prisma::AnimationHandler::clear()
{
	for (int i = 0; i < m_animations.size(); i++)
	{
		for (int j = 0; j < MAX_BONES; j++)
		{
			m_animations[i].animations[j] = glm::mat4(1.0f);
		}
	}
}

void Prisma::AnimationHandler::fill()
{
	for (auto mesh : currentGlobalScene->animateMeshes)
	{
		if (mesh->animator())
		{
			mesh->animator()->updateAnimation(0);
		}
	}
}

std::vector<Prisma::AnimationHandler::SSBOAnimation>& Prisma::AnimationHandler::animations()
{
	return m_animations;
}

Prisma::AnimationHandler::AnimationHandler()
{
	m_ssboAnimation = std::make_shared<SSBO>(8);
	m_ssboAnimation->resize(sizeof(SSBOAnimation) * MAX_ANIMATION_MESHES);
	m_animations.resize(MAX_ANIMATION_MESHES);
	for (int i = 0; i < m_animations.size(); i++)
	{
		for (int j = 0; j < MAX_BONES; j++)
		{
			m_animations[i].animations[j] = glm::mat4(1.0f);
		}
	}
}
