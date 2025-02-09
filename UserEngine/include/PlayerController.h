#include "../../Engine/include/engine.h"
#include "../../Engine/include/SceneObjects/Node.h"
#include "../../Engine/include/SceneObjects/Camera.h"
#include "../../Engine/include/Helpers/NodeHelper.h"
#include "../../Engine/include/Helpers/ScenePrinter.h"
#include "../../Engine/include/GlobalData/Keys.h"
#include "../../Engine/include/GlobalData/GlobalData.h"
#include "../../Engine/include/Components/PhysicsMeshComponent.h"
#include "ParticleController.h"
#include "../../Engine/include/Helpers/Interpolator.h"

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

	void updateCamera();

	void updateKeyboard();

	void scene(std::shared_ptr<Prisma::Scene> scene);

	void update();

	std::shared_ptr<Prisma::CallbackHandler> callback();

	void target(glm::vec3 target);

private:
	void createCamera();

	void createKeyboard();

	void clearVelocity();

	void updateAnimations();

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

	std::shared_ptr<Prisma::Mesh> m_sphereMesh;

	std::shared_ptr<Prisma::Node> m_gunPosition;

	std::shared_ptr<Prisma::PhysicsMeshComponent> m_physics;

	bool m_hide = true;

	float m_baseVelocity = 1.0f;

	unsigned int m_previousClick = 0;

	glm::mat4 m_baseData;

	glm::mat4 m_basePosition;

	std::shared_ptr<Prisma::Animation> m_walkAnimation;

	std::shared_ptr<Prisma::Animation> m_jumpAnimation;

	std::shared_ptr<Prisma::Animation> m_idleAnimation;

	bool m_isColliding = false;

	float m_blending = 0.1f;

	ANIMATIONS m_animations;

	ANIMATIONS m_previousAnimations;

	Vec3 m_currentDirection = Vec3(0, 0, 1);

	ParticleController m_particleController;

	std::vector<std::shared_ptr<Prisma::Mesh>> m_balls;

	Prisma::Interpolator m_interpolator;

	std::shared_ptr<Prisma::Node> m_areaLight;
};
