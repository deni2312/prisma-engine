#include "../include/UserEngine.h"
#include "../../Engine/include/Pipelines/PipelineSkybox.h"
#include "../../Engine/include/Components/PhysicsMeshComponent.h"


#include <memory>
#include <string>
#include <iostream>
#include <glm/gtx/string_cast.hpp>

#include <random>
#include <glm/gtc/random.hpp>
#include "../../Engine/include/SceneData/Animation.h"

void UserEngine::start()
{
    m_root = Prisma::Engine::getInstance().getScene("../../../Resources/DefaultScene/default.gltf", { true });
    Prisma::Texture texture;
    texture.loadEquirectangular("../../../Resources/Skybox/equirectangular.hdr");
    texture.data({ 4096,4096,3 });
    Prisma::PipelineSkybox::getInstance().texture(texture, true);

    Prisma::NodeHelper nodeHelper;

    auto animatedMesh = std::dynamic_pointer_cast<Prisma::AnimatedMesh>(nodeHelper.find(m_root->root, "MutantMesh")->children()[0]);

    if (animatedMesh) {
        auto animation = std::make_shared<Prisma::Animation>("../../../Resources/DefaultScene/animations/animation.gltf", animatedMesh);

        auto animator = std::make_shared<Prisma::Animator>(animation);
        animatedMesh->animator(animator);
    }

    nodeHelper.nodeIterator(m_root->root, [](auto mesh, auto parent) {
        auto currentMesh = std::dynamic_pointer_cast<Prisma::Mesh>(mesh);
        if (currentMesh) {
            auto physicsComponent = std::make_shared<Prisma::PhysicsMeshComponent>();
            physicsComponent->collisionData({ Prisma::Physics::Collider::BOX_COLLIDER,0.0,btVector3(0.0,0.0,0.0),true });
            currentMesh->addComponent(physicsComponent);
        }
    });

    Prisma::Physics::getInstance().physicsWorld()->dynamicsWorld->setGravity(btVector3(0.0, -10.0, 0.0));
    m_handler = std::make_shared<Prisma::CallbackHandler>();
    createCamera();
    m_window = Prisma::PrismaFunc::getInstance().window();
}

void UserEngine::update()
{
    updateCamera();
}

void UserEngine::finish()
{
}

std::shared_ptr<Prisma::CallbackHandler> UserEngine::callbacks()
{
    return m_handler;
}

void UserEngine::updateCamera()
{
    m_velocity = 1 * 1.0f/(float)Prisma::Engine::getInstance().fps();
    updateKeyboard();
    m_root->camera->position(m_position);
    m_root->camera->center(m_position + m_front);
    m_root->camera->up(m_up);
}

void UserEngine::updateKeyboard()
{

    if (glfwGetKey(m_window, Prisma::KEY_W) == GLFW_PRESS) {
        m_position += m_front * m_velocity;
    }
    if (glfwGetKey(m_window, Prisma::KEY_A) == GLFW_PRESS) {
        m_position -= glm::normalize(glm::cross(m_front, m_up)) * m_velocity;
    }
    if (glfwGetKey(m_window, Prisma::KEY_S) == GLFW_PRESS) {
        m_position -= m_front * m_velocity;
    }

    if (glfwGetKey(m_window, Prisma::KEY_D) == GLFW_PRESS) {
        m_position += glm::normalize(glm::cross(m_front, m_up)) * m_velocity;
    }
}

void UserEngine::createCamera()
{
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
