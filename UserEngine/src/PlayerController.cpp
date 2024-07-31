#include "../include/PlayerController.h"

PlayerController::PlayerController(std::shared_ptr<Prisma::Scene> scene) : m_scene{scene} {
    Prisma::NodeHelper nodeHelper;

    m_animatedMesh = std::dynamic_pointer_cast<Prisma::AnimatedMesh>(nodeHelper.find(m_scene->root, "MutantMesh")->children()[0]);

    if (m_animatedMesh) {
        auto animation = std::make_shared<Prisma::Animation>("../../../Resources/DefaultScene/animations/animation.gltf", m_animatedMesh);

        auto animator = std::make_shared<Prisma::Animator>(animation);
        m_animatedMesh->animator(animator);
    }
    createCamera();
}

void PlayerController::updateCamera() {
    m_velocity = m_baseVelocity * 1.0f / (float)Prisma::Engine::getInstance().fps();
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

    if (glfwGetKey(m_window, Prisma::KEY_W) == GLFW_PRESS) {
        playerData[3] -= glm::vec4(m_front * m_velocity, 0);
    }

    if (glfwGetKey(m_window, Prisma::KEY_A) == GLFW_PRESS) {
        playerData[3] += glm::vec4(glm::normalize(glm::cross(m_front, m_up)) * m_velocity, 0);
    }

    if (glfwGetKey(m_window, Prisma::KEY_S) == GLFW_PRESS) {
        playerData[3] += glm::vec4(m_front * m_velocity, 0);
    }

    if (glfwGetKey(m_window, Prisma::KEY_D) == GLFW_PRESS) {
        playerData[3] -= glm::vec4(glm::normalize(glm::cross(m_front, m_up)) * m_velocity, 0);
    }

    m_animatedMesh->parent()->parent()->matrix(playerData);

    if (glfwGetKey(m_window, Prisma::KEY_G) == GLFW_PRESS && !m_press) {
        m_hide = !m_hide;
        if (m_hide) {
            Prisma::PrismaFunc::getInstance().hiddenMouse(m_hide);
        }
        else {
            Prisma::PrismaFunc::getInstance().hiddenMouse(m_hide);
        }

        m_press = true;
    }

    if (glfwGetKey(m_window, Prisma::KEY_G) == GLFW_RELEASE) {
        m_press = false;
    }
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
    m_distance = 10;
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
