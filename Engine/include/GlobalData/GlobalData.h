#pragma once
#include "PrismaFunc.h"

#include "../SceneData/SceneData.h"
#include <memory>

namespace Prisma {
	namespace LightType {
		struct LightDir;
		struct LightOmni;
	}
}

namespace Prisma {

	extern std::shared_ptr<Prisma::Scene> currentGlobalScene;
	extern std::shared_ptr<Prisma::FBO> fboTarget;
	extern glm::mat4 currentProjection;
	extern bool updateLights;
	extern bool updateSizes;
	extern bool updateData;
	extern bool skipUpdate;
	extern bool updateTextures;
	extern Prisma::Texture defaultBlack;
	extern Prisma::Texture defaultNormal;
	extern void resetCaches();

}