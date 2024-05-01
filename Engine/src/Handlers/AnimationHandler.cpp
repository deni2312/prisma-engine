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
	auto boneMatrices = currentGlobalScene->animateMeshes[0]->animator()->GetFinalBoneMatrices();

	m_ssboAnimation->modifyData(0, boneMatrices.size() * sizeof(glm::mat4), boneMatrices.data());
}

Prisma::AnimationHandler::AnimationHandler()
{

	m_ssboAnimation = std::make_shared<SSBO>(8);
	m_ssboAnimation->resize(sizeof(glm::mat4) * MAX_BONES);
}
