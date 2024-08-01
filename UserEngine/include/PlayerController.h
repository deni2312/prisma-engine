#include "../../Engine/include/engine.h"
#include "../../Engine/include/SceneObjects/Node.h"
#include "../../Engine/include/SceneObjects/Camera.h"
#include "../../Engine/include/Helpers/NodeHelper.h"
#include "../../Engine/include/Helpers/ScenePrinter.h"
#include "../../Engine/include/GlobalData/Keys.h"
#include "../../Engine/include/GlobalData/GlobalData.h"

class PlayerController {
public:
    PlayerController(std::shared_ptr<Prisma::Scene> scene);

    void updateCamera();

    void updateKeyboard();

    void scene(std::shared_ptr<Prisma::Scene> scene);

    void update();

    std::shared_ptr<Prisma::CallbackHandler> callback();

	void target(glm::vec3 target);

private:

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

	float m_distance;

	GLFWwindow* m_window;

	glm::vec3 m_target = glm::vec3(0.0f);

    std::shared_ptr<Prisma::Scene> m_scene;

	std::shared_ptr<Prisma::AnimatedMesh> m_animatedMesh;

	std::shared_ptr<Prisma::Mesh> m_bboxMesh;

	bool m_hide = true;

	bool m_press = false;

	float m_baseVelocity = 10.0f;
};