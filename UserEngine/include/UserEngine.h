#include "../../Engine/include/engine.h"
#include "../../Engine/include/SceneObjects/Node.h"
#include "../../Engine/include/SceneObjects/Camera.h"
#include "../../Engine/include/Helpers/NodeHelper.h"
#include "../../Engine/include/Helpers/ScenePrinter.h"
#include "../../Engine/include/GlobalData/Keys.h"

#include <memory>

class UserEngine : public Prisma::Engine {
public:
    UserEngine(Prisma::SceneHandler handler = {});
private:
	void mouseCallback();
    bool showMouse = false;
    bool pressed = false;
	std::shared_ptr<Prisma::CallbackHandler> m_callback;
	std::shared_ptr<Prisma::Scene> m_sceneNode;
	std::shared_ptr<Prisma::Camera> m_camera;

    glm::vec3 m_position = glm::vec3(0.0f);
    glm::vec3 m_front = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 m_up = glm::vec3(0.0f, 1.0f, 0.0f);

    float m_yaw = -90.0f;
    float m_pitch = 0.0f;
    float m_velocity = 1.0;
    bool firstMouse = true;
    float m_lastX;
    float m_lastY;
    bool m_lock = false;

    const unsigned int maxLights = 100;

    std::vector<std::shared_ptr<Prisma::Light<Prisma::LightType::LightOmni>>> m_lights;

protected:
	bool update() override;
};