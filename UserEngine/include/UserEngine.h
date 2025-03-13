#include "../../Engine/include/engine.h"
#include "../../Engine/include/SceneObjects/Node.h"
#include "../../Engine/include/SceneObjects/Camera.h"
#include "../../Engine/include/Helpers/NodeHelper.h"
#include "../../Engine/include/Helpers/ScenePrinter.h"
#include "../../Engine/include/GlobalData/Keys.h"

#include <memory>

class UserEngine : public Prisma::UserData
{
public:
	void start() override;

	void update() override;

	void finish() override;

	std::shared_ptr<Prisma::CallbackHandler> callbacks() override;

private:
	//std::shared_ptr<PlayerController> m_player;

};
