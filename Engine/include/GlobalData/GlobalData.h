#pragma once
#include "PrismaFunc.h"

#include "../SceneData/SceneData.h"
#include <memory>
#include <unordered_map>

namespace Prisma {
	namespace LightType {
		struct LightDir;
		struct LightOmni;
	}
}

namespace Prisma {

	extern std::shared_ptr<Prisma::Scene> currentGlobalScene;
	extern std::shared_ptr<Prisma::FBO> fboTarget;
	extern std::unordered_map<uint64_t, Component*> sceneComponents;
	extern glm::mat4 currentProjection;
	extern Prisma::Texture defaultBlack;
	extern Prisma::Texture defaultWhite;
	extern Prisma::Texture defaultNormal;

}