#include "../../Engine/include/engine.h"
#include "../../Engine/include/SceneObjects/Node.h"
#include "../../Engine/include/SceneObjects/Camera.h"
#include "../../Engine/include/Helpers/NodeHelper.h"
#include "../../Engine/include/Helpers/ScenePrinter.h"
#include "../../Engine/include/GlobalData/Keys.h"

#include <memory>

class UserEngine : public Prisma::UserData {
public:
	virtual void start();

	virtual void update();

	virtual void finish();

	virtual std::shared_ptr<Prisma::CallbackHandler> callbacks();
private:

	void updateCamera();
	
	void updateKeyboard();

	void createCamera();

	std::shared_ptr<Prisma::CallbackHandler> m_handler;

	glm::vec3 m_position = glm::vec3(0.0f);

	glm::vec3 m_front = glm::vec3(0.0f, 0.0f, -1.0f);

	glm::vec3 m_up = glm::vec3(0.0f, 1.0f, 0.0f);

	float m_yaw = -90.0f;

	float m_pitch = 0.0f;

	float m_velocity = 1.0;

	bool m_firstMouse = true;

	float m_lastX;

	float m_lastY;

	std::shared_ptr<Prisma::Scene> m_root;

};