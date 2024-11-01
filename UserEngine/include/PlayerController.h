#include "../../Engine/include/engine.h"
#include "../../Engine/include/SceneObjects/Node.h"
#include "../../Engine/include/SceneObjects/Camera.h"
#include "../../Engine/include/Helpers/NodeHelper.h"
#include "../../Engine/include/Helpers/ScenePrinter.h"
#include "../../Engine/include/GlobalData/Keys.h"
#include "../../Engine/include/GlobalData/GlobalData.h"
#include "../../Engine/include/Components/PhysicsMeshComponent.h"
#include "ParticleController.h"

class PlayerController
{
public:
	PlayerController(std::shared_ptr<Prisma::Scene> scene);

	std::shared_ptr<Prisma::CallbackHandler> callback();

	void update();

private:
	std::shared_ptr<Prisma::Scene> m_scene;
	std::vector<std::shared_ptr<Prisma::AnimatedMesh>> m_animatedMeshes;
};
