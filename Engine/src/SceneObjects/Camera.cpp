#include "../../include/SceneObjects/Camera.h"
#include "glm/ext/matrix_transform.hpp"

void Prisma::Camera::position(const glm::vec3& position)
{
	m_position = position;
	Node::matrix(Camera::matrix(), true);
}

glm::vec3 Prisma::Camera::position() const
{
	return m_position;
}

void Prisma::Camera::matrix(const glm::mat4& matrix, bool update)
{
	Node::matrix(matrix, update);
}

glm::mat4 Prisma::Camera::matrix() const
{
	return lookAt(m_position, m_center, m_up);
}

void Prisma::Camera::center(const glm::vec3& center)
{
	m_center = center;
	Node::matrix(Camera::matrix(), true);
}

glm::vec3 Prisma::Camera::center() const
{
	return m_center;
}

void Prisma::Camera::up(const glm::vec3& up)
{
	m_up = up;
	Node::matrix(Camera::matrix(), true);
}

glm::vec3 Prisma::Camera::up() const
{
	return m_up;
}

void Prisma::Camera::right(const glm::vec3& right)
{
	m_right = right;
}

glm::vec3 Prisma::Camera::right() const
{
	return m_right;
}

void Prisma::Camera::front(const glm::vec3& front)
{
	m_front = front;
}

glm::vec3 Prisma::Camera::front() const
{
	return m_front;
}

void Prisma::Camera::nearPlane(float nearPlane)
{
	m_nearPlane = nearPlane;
}

float Prisma::Camera::nearPlane() const
{
	return m_nearPlane;
}

void Prisma::Camera::farPlane(float farPlane)
{
	m_farPlane = farPlane;
}

float Prisma::Camera::farPlane() const
{
	return m_farPlane;
}

void Prisma::Camera::angle(float angle)
{
	m_angle = angle;
}

float Prisma::Camera::angle() const
{
	return m_angle;
}
