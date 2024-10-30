#pragma once
#include "PrismaFunc.h"

#include "../SceneData/SceneData.h"
#include <memory>
#include <unordered_map>

namespace Prisma::LightType
{
	struct LightDir;
	struct LightOmni;
}

namespace Prisma
{
	extern std::shared_ptr<Scene> currentGlobalScene;
	extern std::shared_ptr<FBO> fboTarget;
	extern std::unordered_map<uint64_t, Component*> sceneComponents;
	extern glm::mat4 currentProjection;
	extern Texture defaultBlack;
	extern Texture defaultWhite;
	extern Texture defaultNormal;
}
