#include "../include/PlayerController.h"

PlayerController::PlayerController() {
    createCamera();
}

void PlayerController::updateCamera()
{
    m_velocity = 1 * 1.0f / (float)Prisma::Engine::getInstance().fps();
    updateKeyboard();
    m_scene->camera->position(m_position);
    m_scene->camera->center(m_position + m_front);
    m_scene->camera->up(m_up);
}

void PlayerController::updateKeyboard()
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

void PlayerController::scene(std::shared_ptr<Prisma::Scene> scene) {
    m_scene = scene;
}

void PlayerController::update() {
    updateCamera();
}

std::shared_ptr<Prisma::CallbackHandler> PlayerController::callback() {
    return m_handler;
}

void PlayerController::createCamera() {
    m_window = Prisma::PrismaFunc::getInstance().window();
    m_handler = std::make_shared<Prisma::CallbackHandler>();

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
