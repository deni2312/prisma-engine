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
	auto animateMeshes = currentGlobalScene->animateMeshes;
	std::vector<SSBOAnimation> animations;
	animations.resize(MAX_ANIMATION_MESHES);
	for (int i = 0; i < animateMeshes.size(); i++) {
		auto animator = animateMeshes[i]->animator();
		if (animator) {
			copyMatrices(animations[i], animator->GetFinalBoneMatrices());
		}
	}
	m_ssboAnimation->modifyData(0, animations.size() * sizeof(SSBOAnimation), animations.data());
}

Prisma::AnimationHandler::AnimationHandler()
{

	m_ssboAnimation = std::make_shared<SSBO>(8);
	m_ssboAnimation->resize(sizeof(SSBOAnimation)*MAX_ANIMATION_MESHES);
}

inline void Prisma::AnimationHandler::copyMatrices(SSBOAnimation& animation, std::vector<glm::mat4>& animationsData) {
	for (int i = 0; i < animationsData.size(); i++) {
		animation.animations[i] = animationsData[i];
	}
}
