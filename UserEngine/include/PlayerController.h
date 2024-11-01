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
	enum ANIMATIONS
	{
		IDLE,
		WALK,
		JUMP,
		DEFAULT
	};

	PlayerController(std::shared_ptr<Prisma::Scene> scene);


	std::shared_ptr<Prisma::CallbackHandler> callback();

private:
	std::shared_ptr<Prisma::Scene> m_scene;

	std::shared_ptr<Prisma::AnimatedMesh> m_animatedMesh;

	std::shared_ptr<Prisma::Animation> m_walkAnimation;

	std::shared_ptr<Prisma::Animation> m_jumpAnimation;

	std::shared_ptr<Prisma::Animation> m_idleAnimation;
};
