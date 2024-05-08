#include "../../include/Handlers/AnimationHandler.h"
#include "../../include/GlobalData/GlobalData.h"
#include "../../include/GlobalData/Defines.h"
#include "../../include/SceneData/MeshIndirect.h"

std::shared_ptr<Prisma::AnimationHandler> Prisma::AnimationHandler::instance = nullptr;

Prisma::AnimationHandler& Prisma::AnimationHandler::getInstance()
{
	if (!instance) {
		instance = std::make_shared<AnimationHandler>();
	}
	return *instance;
}

void Prisma::AnimationHandler::updateAnimations()
{
	m_ssboAnimation->modifyData(0, currentGlobalScene->animateMeshes.size() * sizeof(SSBOAnimation), m_animations.data());
}

std::vector<Prisma::AnimationHandler::SSBOAnimation>& Prisma::AnimationHandler::animations()
{
	return m_animations;
}

Prisma::AnimationHandler::AnimationHandler()
{
	m_ssboAnimation = std::make_shared<SSBO>(8);
	m_ssboAnimation->resize(sizeof(SSBOAnimation)*MAX_ANIMATION_MESHES);
	m_animations.resize(MAX_ANIMATION_MESHES);
	for (int i = 0; i < m_animations.size(); i++) {
		for (int j = 0; j < MAX_BONES; j++) {
			m_animations[i].animations[j] = glm::mat4(1.0f);
		}
	}
}
