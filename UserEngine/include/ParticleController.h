#include "engine.h"
#include "SceneObjects/Node.h"
#include "SceneObjects/Camera.h"
#include "Helpers/NodeHelper.h"
#include "Helpers/ScenePrinter.h"
#include "GlobalData/Keys.h"
#include "GlobalData/GlobalData.h"
#include "Components/PhysicsMeshComponent.h"

class ParticleController
{
public:
	void init(std::shared_ptr<Prisma::Node> root);

	void update();

private:
	//std::shared_ptr<Prisma::Shader> m_compute;
	unsigned int m_deltaPos;
	unsigned int m_timePos;
	bool m_start = false;
	float currentTime = 0;
	std::chrono::steady_clock::time_point m_startPoint;
};
