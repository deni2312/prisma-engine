#pragma once
#include "Node.h"


namespace Prisma
{
	class Camera : public Node
	{
	public:
		void position(const glm::vec3& position);
		glm::vec3 position() const;
		void center(const glm::vec3& center);
		glm::vec3 center() const;
		void up(const glm::vec3& up);
		glm::vec3 up() const;

		void right(const glm::vec3& right);

		glm::vec3 right() const;

		void front(const glm::vec3& front);

		glm::vec3 front() const;

		void nearPlane(float nearPlane);
		float nearPlane() const;
		void farPlane(float farPlane);
		float farPlane() const;
		void angle(float angle);
		float angle() const;
		void matrix(const glm::mat4& matrix, bool update = true) override;
		glm::mat4 matrix() const override;

	private:
		float m_nearPlane = 0.1f;
		float m_farPlane = 1000.0f;
		float m_angle = 45.0f;
		glm::vec3 m_position = glm::vec3(1, 0, 0);
		glm::vec3 m_center = glm::vec3(0, 0, 0);
		glm::vec3 m_up = glm::vec3(0, 1, 0);
		glm::vec3 m_right;
		glm::vec3 m_front;
	};
}
