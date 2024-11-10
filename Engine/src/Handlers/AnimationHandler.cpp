#include "../../include/Handlers/AnimationHandler.h"
#include "../../include/GlobalData/GlobalData.h"
#include "../../include/GlobalData/Defines.h"
#include "../../include/Helpers/TimeCounter.h"
#include "../../include/SceneData/MeshIndirect.h"


void Prisma::AnimationHandler::fill()
{
	for (auto mesh : Prisma::GlobalData::getInstance().currentGlobalScene()->animateMeshes)
	{
		if (mesh->animator())
		{
			mesh->animator()->updateAnimation(0);
		}
	}
}


std::shared_ptr<Prisma::SSBO> Prisma::AnimationHandler::ssbo()
{
	return m_ssboAnimation;
}

Prisma::AnimationHandler::AnimationHandler()
{
	std::vector<SSBOAnimation> animations;

	m_ssboAnimation = std::make_shared<SSBO>(8);
	m_ssboAnimation->resize(sizeof(SSBOAnimation) * MAX_ANIMATION_MESHES);
	animations.resize(MAX_ANIMATION_MESHES);
	for (int i = 0; i < animations.size(); i++)
	{
		for (int j = 0; j < MAX_BONES; j++)
		{
			animations[i].animations[j] = glm::mat4(1.0f);
		}
	}
}
