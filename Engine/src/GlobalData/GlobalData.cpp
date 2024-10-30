#include "../../include/GlobalData/GlobalData.h"


namespace Prisma
{
	std::shared_ptr<Scene> currentGlobalScene = nullptr;
	glm::mat4 currentProjection = glm::mat4(1.0f);
	Texture defaultBlack;
	Texture defaultWhite;
	Texture defaultNormal;
	std::shared_ptr<FBO> fboTarget = nullptr;
	std::unordered_map<uint64_t, Component*> sceneComponents;
}
