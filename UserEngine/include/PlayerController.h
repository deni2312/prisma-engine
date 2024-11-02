#include <deque>

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

	void finish();

private:
	bool checkFinish();
	void clearFinish();
	std::vector<std::thread> m_threads;
	std::mutex m_mutex;
	std::atomic_bool m_start = false;
	unsigned int m_animationThread;
	std::deque<std::atomic_bool> m_finish;
	std::shared_ptr<Prisma::Scene> m_scene;
	std::vector<std::shared_ptr<Prisma::AnimatedMesh>> m_animatedMeshes;
	std::atomic_bool m_finishProgram;
};
