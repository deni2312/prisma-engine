#include "../../Engine/include/engine.h"
#include "../../Engine/include/SceneObjects/Node.h"
#include "../../Engine/include/SceneObjects/Camera.h"
#include "../../Engine/include/Helpers/NodeHelper.h"
#include "../../Engine/include/Helpers/ScenePrinter.h"
#include "../../Engine/include/GlobalData/Keys.h"
#include "PlayerController.h"

#include <memory>

class UserEngine : public Prisma::UserData {
public:
	virtual void start();

	virtual void update();

	virtual void finish();

	virtual std::shared_ptr<Prisma::CallbackHandler> callbacks();
private:

	std::shared_ptr<PlayerController> m_player;

	std::shared_ptr<Prisma::Scene> m_root;

};