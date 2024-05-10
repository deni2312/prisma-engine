#pragma once
#include "Node.h"


namespace Prisma {
	class Camera : public Prisma::Node {
    public:
        void position(const glm::vec3& position);
        glm::vec3 position() const;
        void center(const glm::vec3& center);
        glm::vec3 center() const;
        void up(const glm::vec3& up);
        glm::vec3 up() const;

        void matrix(const glm::mat4& matrix, bool update = true) override;
        glm::mat4 matrix() const override;
    private:
        glm::vec3 m_position;
        glm::vec3 m_center;
        glm::vec3 m_up;
    };
}
