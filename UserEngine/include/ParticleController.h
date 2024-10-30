#include "../../Engine/include/engine.h"
#include "../../Engine/include/SceneObjects/Node.h"
#include "../../Engine/include/SceneObjects/Camera.h"
#include "../../Engine/include/Helpers/NodeHelper.h"
#include "../../Engine/include/Helpers/ScenePrinter.h"
#include "../../Engine/include/GlobalData/Keys.h"
#include "../../Engine/include/GlobalData/GlobalData.h"
#include "../../Engine/include/Components/PhysicsMeshComponent.h"

class ParticleController
{
public:
	void init(std::shared_ptr<Prisma::Node> root);

	void update();

private:
	std::shared_ptr<Prisma::Shader> m_compute;
	unsigned int m_deltaPos;
	unsigned int m_timePos;
	bool m_start = false;
	float currentTime = 0;
	std::chrono::steady_clock::time_point m_startPoint;
};
