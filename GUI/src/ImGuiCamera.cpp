#include "../include/ImGuiCamera.h"
#include "../../Engine/include/GlobalData/Keys.h"
#include "../../Engine/include/SceneObjects/Mesh.h"
#include "GLFW/glfw3.h"
#include <iostream>
#include "../include/PixelCapture.h"

Prisma::ImGuiCamera::ImGuiCamera()
{
    m_callback = std::make_shared<Prisma::CallbackHandler>();
    m_imguiSelector=std::make_shared<ImGuiSelector>();
    m_currentSelect=nullptr;
}

void Prisma::ImGuiCamera::updateCamera(std::shared_ptr<Prisma::Camera> camera)
{
    if (!m_lock) {
        camera->position(m_position);
        camera->center(m_position + m_front);
        camera->up(m_up);
    }
}

void Prisma::ImGuiCamera::keyboardUpdate(void* windowData)
{
    auto window = (GLFWwindow*)windowData;
    if (glfwGetKey(window, Prisma::KEY_C) == GLFW_PRESS) {
        //Prisma::PrismaFunc()->closeWindow();
    }
    if (glfwGetKey(window, Prisma::KEY_W) == GLFW_PRESS) {
        m_position += m_front * m_velocity;
    }
    if (glfwGetKey(window, Prisma::KEY_A) == GLFW_PRESS) {
        m_position -= glm::normalize(glm::cross(m_front, m_up)) * m_velocity;
    }
    if (glfwGetKey(window, Prisma::KEY_S) == GLFW_PRESS) {
        m_position -= m_front * m_velocity;
    }
    if (glfwGetKey(window, Prisma::KEY_D) == GLFW_PRESS) {
        m_position += glm::normalize(glm::cross(m_front, m_up)) * m_velocity;
    }
    if (glfwGetKey(window, Prisma::KEY_G) == GLFW_PRESS && !pressed) {
        showMouse = !showMouse;
        m_lock = !m_lock;
        //PrismaData()->hiddenMouse(showMouse);
        pressed = true;
    }

    if (glfwGetKey(window, Prisma::KEY_G) == GLFW_RELEASE) {
        pressed = false;
    }
}

void Prisma::ImGuiCamera::mouseCallback()
{
    m_callback->mouse = [this](float x, float y) {
        if (!m_lock && x<m_constraints.maxX && y<m_constraints.maxY && x>m_constraints.minX && y>m_constraints.minY) {
            float xpos = static_cast<float>(x);
            float ypos = static_cast<float>(y);

            if (firstMouse)
            {
                m_lastX = xpos;
                m_lastY = ypos;
                firstMouse = false;
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
        }
    };
}

void Prisma::ImGuiCamera::velocity(float velocity)
{
    m_velocity = velocity;
}

void Prisma::ImGuiCamera::currentSelect(Node* currentSelect) {
    m_currentSelect = currentSelect;
}

std::shared_ptr<Prisma::CallbackHandler> Prisma::ImGuiCamera::callback()
{
    return m_callback;
}

void Prisma::ImGuiCamera::mouseButtonCallback() {
    m_callback->mouseClick = [this](int button,int action,float x, float y) {
        // Here you would handle mouse button clicks
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && x<m_constraints.maxX && y<m_constraints.maxY && x>m_constraints.minX && y>m_constraints.minY && !m_constraints.isOver) {

            Prisma::PixelCapture::getInstance().capture(glm::vec2(x, y));
            // Ray casting from mouse coordinates
            std::pair<glm::vec3, glm::vec3> rayData = m_imguiSelector->castRayFromMouse(x, y);
            glm::vec3 rayOrigin = m_position;
            glm::vec3 rayDir = rayData.second;
            auto result=m_imguiSelector->raycastWorld(rayOrigin,rayDir*100000.0f);
            if(result) {
                m_currentSelect = result->other;
                auto model=result->other->matrix();
                m_constraints.model(model);
            }else{
                auto model=glm::mat4(1.0f);
                m_constraints.model(model);
                m_currentSelect=nullptr;
            }
        }
    };
}

std::shared_ptr<Prisma::ImGuiSelector> Prisma::ImGuiCamera::imguiSelector() {
    return m_imguiSelector;
}

Prisma::Node *Prisma::ImGuiCamera::currentSelect() {
    return m_currentSelect;
}

void Prisma::ImGuiCamera::constraints(Prisma::ImGuiCamera::CameraConstarints constraints) {
    m_constraints=constraints;
}
