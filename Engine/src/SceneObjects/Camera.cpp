
#include "../../include/SceneObjects/Camera.h"
#include "glm/ext/matrix_transform.hpp"

void Prisma::Camera::position(glm::vec3 position) {
    m_position=position;
}

glm::vec3 Prisma::Camera::position() {
    return m_position;
}

void Prisma::Camera::matrix(glm::mat4 matrix, bool update) {
    Node::matrix(matrix, update);
}

glm::mat4 Prisma::Camera::matrix() const {
    return glm::lookAt(m_position,m_center,m_up);
}

void Prisma::Camera::center(glm::vec3 center) {
    m_center=center;
}

glm::vec3 Prisma::Camera::center() {
    return m_center;
}

void Prisma::Camera::up(glm::vec3 up) {
    m_up=up;
}

glm::vec3 Prisma::Camera::up() {
    return m_up;
}
