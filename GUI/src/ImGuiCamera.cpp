#include "../include/ImGuiCamera.h"
#include "../../Engine/include/GlobalData/Keys.h"
#include "../../Engine/include/SceneObjects/Mesh.h"
#include <iostream>
#include "../../Engine/include/Helpers/SettingsLoader.h"
#include "../../Engine/include/SceneData/SceneExporter.h"
#include "../../Engine/include/engine.h"
#include <imgui.h>

#include "../imguizmo/imguizmo.h"
#include "../include/PixelCapture.h"
#include "../include/TextureInfo.h"

Prisma::ImGuiCamera::ImGuiCamera()
{
	m_callback = std::make_shared<CallbackHandler>();
	m_currentSelect = nullptr;
}

void Prisma::ImGuiCamera::updateCamera(std::shared_ptr<Camera> camera)
{
	camera->position(m_position);
	camera->center(m_position + m_front);
	camera->up(m_up);
	camera->front(m_front);
	camera->right(glm::normalize(glm::cross(m_front, m_up)));
	m_totalVelocity = m_velocity * 1.0f / Engine::getInstance().fps();
}

void Prisma::ImGuiCamera::keyboardUpdate(void* windowData)
{
	auto window = static_cast<GLFWwindow*>(windowData);
	//auto textureInfo = !Prisma::TextureInfo::getInstance().textureTab();
	if (!ImGui::GetIO().WantTextInput) {

		if (glfwGetKey(window, KEY_DELETE) == GLFW_PRESS)
		{
			if (m_currentSelect)
			{
				m_currentSelect->parent()->removeChild(m_currentSelect->uuid());
				m_currentSelect = nullptr;
			}
		}
		if (glfwGetKey(window, KEY_W) == GLFW_PRESS)
		{
			m_position += m_front * m_totalVelocity;
		}

		if (glfwGetKey(window, KEY_A) == GLFW_PRESS)
		{
			m_position -= normalize(cross(m_front, m_up)) * m_totalVelocity;
		}

		if (glfwGetKey(window, KEY_S) == GLFW_PRESS && (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
			glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS) && !m_save)
		{
			Exporter::getInstance().exportScene();
			m_save = true;
		}
		else if (glfwGetKey(window, KEY_S) == GLFW_PRESS && !(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
			glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS))
		{
			m_position -= m_front * m_totalVelocity;
		}

		if (glfwGetKey(window, KEY_S) == GLFW_RELEASE || glfwGetKey(window, GLFW_KEY_LEFT_CONTROL == GLFW_RELEASE) ||
			glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL == GLFW_RELEASE))
		{
			m_save = false;
		}

		if (glfwGetKey(window, KEY_D) == GLFW_PRESS)
		{
			m_position += normalize(cross(m_front, m_up)) * m_totalVelocity;
		}

		if (glfwGetKey(window, KEY_G) == GLFW_RELEASE)
		{
			m_pressed = false;
		}
	}
}

void Prisma::ImGuiCamera::mouseCallback()
{
	m_callback->mouse = [this](float x, float y)
	{
		//auto textureInfo = !Prisma::TextureInfo::getInstance().textureTab();
		if (m_right && x < m_constraints.maxX && y < m_constraints.maxY && x > m_constraints.minX && y > m_constraints.
			minY)
		{
			float xpos = x;
			float ypos = y;

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
			m_front = normalize(front);
		}
	};
}

void Prisma::ImGuiCamera::velocity(float velocity)
{
	m_velocity = velocity;
}

void Prisma::ImGuiCamera::currentSelect(Node* currentSelect)
{
	m_currentSelect = currentSelect;
}

std::shared_ptr<Prisma::CallbackHandler> Prisma::ImGuiCamera::callback()
{
	return m_callback;
}

bool Prisma::ImGuiCamera::openPopup() const
{
	return m_openPopup;
}

void Prisma::ImGuiCamera::openPopup(bool openPopup)
{
	m_openPopup = openPopup;
}

int id = 0;

void Prisma::ImGuiCamera::mouseButtonCallback()
{
	m_callback->mouseClick = [this](int button, int action, float x, float y)
	{
		//ISOVER BUGGED IMGUIZMO RETURN TRUE RANDOMLY
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && x < m_constraints.maxX && y < m_constraints.maxY
			&& x > m_constraints.minX && y > m_constraints.minY && !ImGuizmo::IsOver() && !Prisma::TextureInfo::getInstance().textureTab())
		{
			auto settings = SettingsLoader::getInstance().getSettings();
			y = settings.height - y;
			auto result = Prisma::PixelCapture::getInstance().capture(glm::vec2(x, y), m_constraints.model);

			if (result)
			{
				m_currentSelect = result.get();
				auto model = result->matrix();
			}
			else
			{
				auto model = glm::mat4(1.0f);
				m_currentSelect = nullptr;
			}
		}


		if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
		{
			m_right = true;
			m_lastX = x;
			m_lastY = y;
		}

		if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
		{
			m_right = false;
			if (x < m_constraints.minX && y < m_constraints.maxY)
			{
				m_openPopup = true;
			}
		}
	};

	m_callback->rollMouse = [this](double xOffset, double yOffset)
	{
		if (yOffset > 0)
		{
			m_velocity += 1.0f; // Increase velocity when scrolling up
		}
		else if (yOffset < 0)
		{
			m_velocity -= 1.0f; // Decrease velocity when scrolling down
			if (m_velocity < 0.1f)
			{
				m_velocity = 1.0f; // Prevent velocity from becoming too small or negative
			}
		}
	};
}

Prisma::Node* Prisma::ImGuiCamera::currentSelect()
{
	return m_currentSelect;
}

void Prisma::ImGuiCamera::constraints(CameraConstarints constraints)
{
	m_constraints = constraints;
}
