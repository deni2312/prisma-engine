#include "../include/PlayerController.h"
#include "../../Engine/include/Components/PhysicsMeshComponent.h"

PlayerController::PlayerController(std::shared_ptr<Prisma::Scene> scene) : m_scene{scene} {
    Prisma::NodeHelper nodeHelper;

    m_animatedMesh = std::dynamic_pointer_cast<Prisma::AnimatedMesh>(nodeHelper.find(m_scene->root, "MutantMesh")->children()[0]);

    if (m_animatedMesh) {
        m_walkAnimation = std::make_shared<Prisma::Animation>("../../../Resources/DefaultScene/animations/animation.gltf", m_animatedMesh);
        m_jumpAnimation = std::make_shared<Prisma::Animation>("../../../Resources/DefaultScene/animations/jump.gltf", m_animatedMesh);
        auto animator = std::make_shared<Prisma::Animator>(m_walkAnimation);
        m_animatedMesh->animator(animator);
    }

    m_bboxMesh = std::dynamic_pointer_cast<Prisma::Mesh>(nodeHelper.find(m_scene->root, "BBoxMesh"));
    m_bboxMesh->visible(false);
    m_physics = std::dynamic_pointer_cast<Prisma::PhysicsMeshComponent>(m_bboxMesh->components()["Physics"]);
    m_physics->collisionData({ Prisma::Physics::Collider::BOX_COLLIDER,1.0,btVector3(0.0,0.0,0.0),true });
    m_baseData = m_animatedMesh->parent()->parent()->matrix();
    createCamera();
    createKeyboard();
}

void PlayerController::updateCamera() {
    m_velocity = m_baseVelocity;
    //m_velocity = m_baseVelocity * 1.0f / (float)Prisma::Engine::getInstance().fps();

    // Calculate the new position based on yaw, pitch, and distance from target
    glm::vec3 offset;
    offset.x = m_distance * cos(glm::radians(m_pitch)) * cos(glm::radians(m_yaw));
    offset.y = m_distance * sin(glm::radians(m_pitch));
    offset.z = m_distance * cos(glm::radians(m_pitch)) * sin(glm::radians(m_yaw));

    m_position = m_target + offset;

    // Update the camera view matrix
    m_scene->camera->position(m_position);
    m_scene->camera->center(m_target);
    m_scene->camera->up(m_up);
}


void PlayerController::updateKeyboard()
{
    auto playerData = m_animatedMesh->parent()->parent()->matrix();
    auto rb = m_physics->rigidBody();
    auto shape = m_physics->shape();
    rb->setAngularFactor(btVector3(0, 0, 0));
    glm::vec3 frontClamp = m_front;
    frontClamp.y = 0;


    auto isJumping = m_animatedMesh->animator()->animation()->id() == m_jumpAnimation->id() && m_animatedMesh->animator()->currentTime() + m_jumpAnimation->ticksPerSecond()*1.0f / (float)Prisma::Engine::getInstance().fps() >= m_jumpAnimation->duration();

    if (isColliding() && !m_isColliding) {
        if (isJumping) {
            m_animatedMesh->animator()->playAnimation(m_walkAnimation, m_blending);
            m_isColliding = true;
        }
    }

    glm::mat4 offsetRotation;
    btVector3 velocity = rb->getLinearVelocity();

    if (glfwGetKey(m_window, Prisma::KEY_W) == GLFW_PRESS) {
        auto currentDirection = Prisma::getVec3BT(-glm::normalize(glm::vec3(frontClamp * m_velocity)));
        currentDirection.setY(velocity.getY());
        rb->setLinearVelocity(currentDirection);
        offsetRotation = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0, 0, 1));
        playerData = m_baseData * glm::rotate(glm::mat4(offsetRotation), glm::radians(m_yaw), glm::vec3(0, 0, 1));
        m_animatedMesh->parent()->parent()->matrix(playerData);
        m_previousClick = Prisma::KEY_W;
        m_clearPhysics = false;
    }

    if (glfwGetKey(m_window, Prisma::KEY_A) == GLFW_PRESS) {
        auto currentDirection = Prisma::getVec3BT(glm::normalize(glm::cross(frontClamp, m_up)) * m_velocity);
        currentDirection.setY(velocity.getY());
        rb->setLinearVelocity(currentDirection);
        playerData = m_baseData * glm::rotate(glm::mat4(1.0f), glm::radians(m_yaw), glm::vec3(0, 0, 1));
        m_animatedMesh->parent()->parent()->matrix(playerData);
        m_previousClick = Prisma::KEY_A;
        m_clearPhysics = false;
    }

    if (glfwGetKey(m_window, Prisma::KEY_S) == GLFW_PRESS) {
        auto currentDirection = Prisma::getVec3BT(glm::normalize(glm::vec3(frontClamp * m_velocity)));
        currentDirection.setY(velocity.getY());
        rb->setLinearVelocity(currentDirection);
        offsetRotation = glm::rotate(glm::mat4(1.0f), glm::radians(270.0f), glm::vec3(0, 0, 1));
        playerData = m_baseData * glm::rotate(glm::mat4(offsetRotation), glm::radians(m_yaw), glm::vec3(0, 0, 1));
        m_animatedMesh->parent()->parent()->matrix(playerData);
        m_previousClick = Prisma::KEY_S;
        m_clearPhysics = false;
    }

    if (glfwGetKey(m_window, Prisma::KEY_D) == GLFW_PRESS) {
        auto currentDirection = Prisma::getVec3BT(-glm::normalize(glm::cross(frontClamp, m_up)) * m_velocity);
        currentDirection.setY(velocity.getY());
        rb->setLinearVelocity(currentDirection);
        offsetRotation = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0, 0, 1));
        playerData = m_baseData * glm::rotate(glm::mat4(offsetRotation), glm::radians(m_yaw), glm::vec3(0, 0, 1));
        m_animatedMesh->parent()->parent()->matrix(playerData);
        m_previousClick = Prisma::KEY_D;
        m_clearPhysics = false;
    }

    if (isJumping) {

    }
    else {
        m_animatedMesh->animator()->updateAnimation(1.0f / (float)Prisma::Engine::getInstance().fps());
    }

    clearVelocity();

    rb->activate(true);
}

void PlayerController::scene(std::shared_ptr<Prisma::Scene> scene) {
    m_scene = scene;
}

void PlayerController::update() {
    target(m_animatedMesh->parent()->finalMatrix()[3]);
    updateCamera();
    updateKeyboard();
}

std::shared_ptr<Prisma::CallbackHandler> PlayerController::callback() {
    return m_handler;
}

void PlayerController::target(glm::vec3 target) {
    m_target = target;
}

void PlayerController::createCamera() {
    m_window = Prisma::PrismaFunc::getInstance().window();
    m_handler = std::make_shared<Prisma::CallbackHandler>();
    m_distance = 5;
    m_handler->mouse = [this](float x, float y) {
        float xpos = static_cast<float>(x);
        float ypos = static_cast<float>(y);

        if (m_firstMouse)
        {
            m_lastX = xpos;
            m_lastY = ypos;
            m_firstMouse = false;
        }

        float xoffset = xpos - m_lastX;
        float yoffset = m_lastY - ypos; // reversed since y-coordinates go from bottom to top
        m_lastX = xpos;
        m_lastY = ypos;

        float sensitivity = 0.1f; // change this value to your liking
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        m_yaw += xoffset;
        m_pitch += yoffset;

        // make sure that when pitch is out of bounds, screen doesn't get flipped
        if (m_pitch > 89.0f)
            m_pitch = 89.0f;
        if (m_pitch < -89.0f)
            m_pitch = -89.0f;

        glm::vec3 front;
        front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
        front.y = sin(glm::radians(m_pitch));
        front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
        m_front = glm::normalize(front);
    };
    m_handler->mouseClick = [](int button, int action, double x, double y) {};
}

void PlayerController::createKeyboard()
{
    m_handler->keyboard = [this](int key, int scancode, int action, int mods) {

        if (key == Prisma::KEY_G && action == GLFW_PRESS) {
            m_hide = !m_hide;
            if (m_hide) {
                Prisma::PrismaFunc::getInstance().hiddenMouse(m_hide);
            }
            else {
                Prisma::PrismaFunc::getInstance().hiddenMouse(m_hide);
            }
            m_previousClick = Prisma::KEY_G;
        }

        if (key == Prisma::KEY_SPACE && action == GLFW_PRESS) {

            auto rb = m_physics->rigidBody();
            if (m_animatedMesh->animator()->animation()->id() != m_jumpAnimation->id()) {
                btVector3 velocity = rb->getLinearVelocity();
                rb->applyCentralImpulse(btVector3(0, 2.0f, 0));
                m_animatedMesh->animator()->playAnimation(m_jumpAnimation,m_blending);
            }
            m_isColliding = false;
            m_previousClick = Prisma::KEY_SPACE;
        }
    };
}

void PlayerController::clearVelocity()
{
    auto rb = m_physics->rigidBody();
    btVector3 velocity = rb->getLinearVelocity();
    if (glfwGetKey(m_window, m_previousClick) == GLFW_RELEASE && !m_clearPhysics) {
        rb->setLinearVelocity(btVector3(0.0f, velocity.getY(), 0.0f));

        m_clearPhysics = true;
    }
}

bool PlayerController::isColliding()
{
    auto world=Prisma::Physics::getInstance().physicsWorld()->dispatcher;
    int numManifolds = world->getNumManifolds();
    for (int i = 0; i < numManifolds; i++)
    {
        btPersistentManifold* contactManifold = world->getManifoldByIndexInternal(i);
        auto obA = (Prisma::Mesh*)contactManifold->getBody0()->getUserPointer();
        auto obB = (Prisma::Mesh*)contactManifold->getBody1()->getUserPointer();
        if (obA->uuid() == m_bboxMesh->uuid() || obB->uuid() == m_bboxMesh->uuid()) {
            return true;
        }
    }
    return false;
}
